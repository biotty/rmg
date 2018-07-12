//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "observer.hpp"
#include "image.h"

#include <iostream>
#include <algorithm>
#include <utility>
#include <thread>
#include <mutex>


using serial_t = int;
#define SERIAL_END -1
template<typename T>
struct Sequenced {
    serial_t serial;
    T value;

    Sequenced() : serial(SERIAL_END) {}
    operator bool() { return serial != SERIAL_END; }

    template<typename U>
    Sequenced(Sequenced<U> & seq, T v) : serial(seq.serial), value(v) {}
    Sequenced(serial_t serial, T v) : serial(serial), value(v) {}
};


struct RasterJob {
    std::mutex input_mutex;
    serial_t input_serial;
    int x, y, w, h;

    using type = std::pair<decltype(x), decltype(y)>;

    std::mutex output_mutex;
    serial_t output_serial;
    using result = Sequenced<color>;
    std::vector<result> buffer;
    std::function<color(type)> f;
    image out;

    RasterJob(int width, int height,
            std::function<color(type)> f, image out)
        : input_serial(), x(), y(), w(width), h(height)
        , output_serial(), f(f), out(out) {}

    void output(result r) {
        std::lock_guard<std::mutex> guard(output_mutex);
        buffer.push_back(r);

        while (true) {
            auto it = std::find_if(buffer.begin(), buffer.end(),
                    [this](result & r)
                    { return output_serial == r.serial; });
            if (it == buffer.end())
                break;

            image_write(out, it->value);

            buffer.erase(it);
            ++output_serial;
        }
    }

    Sequenced<type> input() {
        if (y == h) {
            return {};
        }
        std::lock_guard<std::mutex> guard(input_mutex);

        type r{x, y};

        if (++x == w) {
            ++y;
            x = 0;
        }
        return {input_serial++, r};
    }

    void run() {
        while (auto s = input()) output({s, f(s.value)});
    }
};


    ray
observer_ray(const observer & o, real aspect_ratio,
        real unit_column, real unit_row)
{
    direction x = o.column_direction;
    direction y = o.row_direction;
    scale(&x, aspect_ratio * (unit_column - .5));
    scale(&y, unit_row - .5);
    point v = o.view;
    move(&v, x);
    move(&v, y);
    ray ray_{v, distance_vector(o.eye, v)};
    move(&ray_.endpoint, distance_vector(o.view, o.eye));
    normalize(&ray_.head);
    return ray_;
}


    void
produce_trace(const char * path, int width, int height,
        const world & w, const observer & obs, unsigned n_threads)
{
    if ( ! n_threads) n_threads = std::thread::hardware_concurrency();
    image out = image_create(path, width, height);

    RasterJob job{width, height,
        [obs, width, height, &w](typename RasterJob::type p){
            return trace(observer_ray(obs, width /(real) height,
                        (p.first + (real).5) / width,
                        (p.second + (real).5) / height), w);
        }, out};
    std::vector<std::thread> workers;
    for (auto i = 0u; i < n_threads; i++) {
        workers.emplace_back(&RasterJob::run, &job);
    }

    for (auto & t : workers) t.join();
    image_close(out);
}
