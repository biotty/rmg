#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

using count_t = uint32_t;  // rather big counters, to be hit by the logistic function
using line = vector<count_t>;
using offset_t = uint32_t;  // the feigenbaum loop spits offsets, not spraying mem
using buffer = vector<offset_t>;

double rnd(double f = 1.0) { return rand() * f / RAND_MAX; }

extern "C" void feigen(double m, offset_t * b, size_t n_line, double * r, size_t n_iters);
// ^ parameter order layout as desired; rdi for stosd and rcx for loop, see feigen.S

void feigenbaum(double m, count_t * line, size_t n_line, size_t k, size_t k_rounds, size_t n_threads)
{
    constexpr size_t bucket_bits = 8;  // 4*2^b is mem covered per bucket.  opt for speed
    const size_t n_buckets = 1 + ((n_line - 1) >> bucket_bits);
    vector<double> r;
    for (size_t t = n_threads * 4; t; t--) r.push_back(rnd());
    for (size_t i = 0; i < k_rounds; i++) {
        buffer buf(k * n_threads);

     // vector<thread> tds;
        size_t t = 0;
     // for (; t < n_threads; t++) tds.emplace_back([&](){
            feigen(m, buf.data() + t * k, n_line, r.data() + t * 4, k);
     //     });
     // for (size_t t = n_threads; t;) tds[--t].join();

        vector<buffer> buckets(n_buckets);
        for (auto j : buf) buckets[j >> bucket_bits].push_back(j);
        for (auto p : buckets) for (auto j : p) ++line[j];
    }
}

void slice(size_t i, size_t t, double a, double w, size_t n, FILE * f)
{
    line v(n);
    for (; i < t; i++) {
        fprintf(stderr, "\r%zu", i);
        double m = i * w + a;
        feigenbaum(m, v.data(), v.size(), 20000, 20000, 1);
        fwrite(v.data(), sizeof v[0], v.size(), f);
        fill(v.begin(), v.end(), 0);
    }
    fputc('\n', stderr);
}

FILE * slice_file(size_t i, size_t n)
{
    size_t j = (i / 100) * 100;
    ostringstream buf;
    buf << "slices" << j << "-" << (j + 100);
    mkdir(buf.str().c_str(), S_IRWXU);
    buf << "/" << setfill('0') << setw(6) << i << "." << n;
    return fopen(buf.str().c_str(), "w");
}

int main(int argc, char ** argv)
{
    if (argc != 3) {
        fputs("args\n", stderr);
        return 1;
    }
    size_t n = atoi(*++argv);
    size_t t = atoi(*++argv);
    double a = 3.50;
    double b = 4.00;
    size_t s = 40;

    double w = (b - a) / t;
    size_t i = 0;
    for (size_t j = 0; j < t;) {
        FILE * f = slice_file(i++, n);
        size_t k = i * s;
        if (k > t) k = t;
        slice(j, k, a, w, n, f);
        fclose(f);
        j = k;
    }
}
