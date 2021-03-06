//      © Christian Sommerfeldt Øien
//      All rights reserved

#include <sys/stat.h>
#include <sys/types.h>

#include <ctime>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cassert>

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <numeric>

using namespace std;

using count_t = uint32_t;  // rather big counters, to be hit by the logistic function
using line = vector<count_t>;
using offset_t = uint32_t;  // the feigenbaum loop spits offsets, not spraying mem
using buffer = vector<offset_t>;

double rnd(double f = 1.0) { return rand() * f / (RAND_MAX + 1.); }
void randomize(vector<double> & r) { for (auto & v : r) v = rnd(); }

extern "C" void feigen(double m, offset_t * q, size_t n_line, double * r, size_t n_iters);
// ^ parameter order layout as desired; rdi for stosd and rcx for loop, see feigen.S

void produce_line(line & v, offset_t lower, offset_t upper,
        double m, double d, size_t k_iters, size_t k, size_t n_threads = 1)
{
    size_t n_line = v.size();
    vector<double> r(n_threads * 4);
    constexpr size_t bucket_bits = 8;  // 4*2^b is mem covered per bucket, each to spray some mem
    const size_t n_buckets = 1 + ((n_line - 1) >> bucket_bits);
    for (size_t i = 0; i < k_iters; i++) {
        randomize(r);
        double w = m + rnd() * d;
        buffer buf(k * n_threads);
     // vector<thread> tds;
        size_t t = 0;
     // for (; t < n_threads; t++) tds.emplace_back([&](){
            feigen(w, &buf[t * k], n_line, &r[t * 4], k / 4);
     //     });
     // while (t) tds[--t].join();
        vector<buffer> buckets(n_buckets);
        for (auto q : buf) {
            if (q >= lower && q < upper) {
                buckets[q >> bucket_bits].push_back(q);
            }
        }
        for (auto p : buckets) for (auto q : p) ++v[q];
    }
}

struct output_params { char * dir; size_t w, i, n, x, s, k; };

FILE * open_file(char * dir, size_t x, const char * mode)
{
    ostringstream buf;
    buf << dir << "/" << setfill('0') << setw(6) << x << ".pnm";
    return fopen(buf.str().c_str(), mode);
}

void create_files(output_params par)
{
    size_t h = par.w - par.s;
    for (size_t x = par.x; x <= h; x += par.s) {
        if (FILE * grl_f = open_file(par.dir, x, "w")) {
            fprintf(grl_f, "P5\n%zu %zu\n65535\n", par.s, par.n - par.i);
            fclose(grl_f);
        }
        if (par.k) break;
    }
}

void srgb_encode(double s, vector<uint8_t> & buf)
{
    if (s <= 0.0031308) s *= 12.92;
    else s = pow(s, 1.0 / 2.4) * 1.055 - 0.055;
    uint16_t g = s * 65535;
    buf.push_back(g >> 8);
    buf.push_back(g & 255);
}

void append_grl(count_t * counts, size_t s, FILE * f, count_t max_)
{
    vector<uint8_t> buf;
    buf.reserve(s * 2);
    double u = 1. / (1. + max_);
    for (size_t i = 0; i < s; i++) {
        srgb_encode(counts[i] * u, buf);
    }
    if (buf.size() != fwrite(buf.data(), 1, buf.size(), f)) {
        fprintf(stderr, "short fwrite");
        exit(1);
    }
}

void append_files(output_params par, count_t * counts,
        vector<bool> & interest, size_t sum_min)
{
    size_t h = par.w - par.s;
    size_t i = 0;
    for (size_t x = par.x; x <= h; x += par.s, i++) {
        if (FILE * grl_f = open_file(par.dir, x, "a")) {
            count_t *p = counts + x;
            count_t max_ = 0, sum_ = accumulate(p, p + par.s, 0,
                    [&max_](count_t a, count_t c){
                    if (max_ < c) max_ = c;
                    return a + c;
                    });
            if (sum_ >= sum_min) {
                interest[i] = true;
            }
            append_grl(p, par.s, grl_f, max_);
            fclose(grl_f);
        }
        if (par.k) break;
    }
}

void discard_files(output_params par, vector<bool> & interest)
{
    size_t h = par.w - par.s;
    size_t i = 0;
    for (size_t x = par.x; x <= h; x += par.s, i++) {
        if ( ! interest[i]) {
            if (FILE * grl_f = open_file(par.dir, x, "w")) {
                fclose(grl_f);
            }
        }
        if (par.k) break;
    }
}

int main(int argc, char ** argv)
{
    if (argc != 10) {
        fputs("args: g w h i n x s p k\n", stderr);
        return 1;
    }
    char * g = *++argv;
    size_t w = atoi(*++argv);
    size_t h = atoi(*++argv);
    size_t i = atoi(*++argv);
    size_t n = atoi(*++argv);
    size_t x = atoi(*++argv);
    size_t s = atoi(*++argv);
    size_t p = atoi(*++argv);
    size_t k = atoi(*++argv);
    size_t z = p >= 2 ? 40000u : p >= 1 ? 4000u : 400u;
    double a = 3.5;
    double b = 4.0;
    srand(time(NULL));
    output_params par = { g, w, i, n, x, s, k };
    create_files(par);
    size_t q = (w - x) / s;
    vector<bool> interest(q);
    double d = (b - a) / h;
    offset_t lower = x;
    offset_t upper = w;
    if (k) {
        upper = lower + s;
    }
    for (size_t j = i; j < n; j++) {
        line v(w);
        produce_line(v, lower, upper, a + d * j, d, z, z);
        append_files(par, v.data(), interest, 15 * 4 * z / q);
        fprintf(stderr, "\r%zu", j);
    }
    discard_files(par, interest);
    fprintf(stderr, "\r        \r");
}
