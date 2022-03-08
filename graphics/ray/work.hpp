//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef WORK_HPP
#define WORK_HPP

#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>

template <typename T>
class work {
    struct seq_result {
        int i;
        T r;
        bool operator<(const seq_result & other) const {
            return i > other.i;
        }
        seq_result() : i(-1) {}
        seq_result(int i, T r) : i(i), r(r) {}
    };

    int full_at;
    std::condition_variable not_full;
    std::priority_queue<seq_result> results;
    std::mutex results_mutex;
    std::atomic<int> seq_give;
    std::mutex wanted_mutex;
    int seq_wanted;
    std::condition_variable may_deliver;
    std::vector<std::thread> workers;
    int seq_done;

    void put_result(seq_result s) {
        {
            auto lock = std::unique_lock<std::mutex>{results_mutex};
            results.push(s);
        }
        may_deliver.notify_one();
    }

public:
    work(size_t n_thr, int maxbuf, int total_count,
            std::function<T(int)> input)
        : full_at(maxbuf + n_thr), seq_give(0), seq_wanted(0), seq_done(total_count)
    {
        for (size_t j=0; j<n_thr; j++) {
            workers.push_back(std::thread([this, input, j](){
                while (true) {
                    int i = seq_give++;
                    if (i >= seq_done) {
                        break;
                    }
                    {
                        auto lock = std::unique_lock<std::mutex>{wanted_mutex};
                        while (i - seq_wanted > this->full_at) {
                            not_full.wait(lock);
                        }
                    }
                    put_result({i, input(i)});
                }
            }));
        }
    }

    ~work() {
        for (auto & thr : workers) {
            thr.join();
        }
    }

    T get_result() {
        T r;
        {
            auto lock = std::unique_lock<std::mutex>{results_mutex};
            while (results.empty() || results.top().i != seq_wanted) {
                may_deliver.wait(lock);
            }
            r = results.top().r;
            results.pop();
        }
        {
            auto lock = std::unique_lock<std::mutex>{wanted_mutex};
            ++seq_wanted;
        }
        not_full.notify_one();
        return r;
    }

    bool more() {
        return seq_wanted < seq_done;
    }
};

#endif
