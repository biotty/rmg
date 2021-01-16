#include <sys/stat.h>
#include <sys/types.h>

#include <ctime>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

using count_t = uint32_t;  // rather big counters, to be hit by the logistic function
using line = vector<count_t>;
using offset_t = uint32_t;  // the feigenbaum loop spits offsets, not spraying mem
using buffer = vector<offset_t>;

double rnd(double f = 1.0) { return rand() * f / RAND_MAX; }
void randomize(vector<double> & r) { for (auto & v : r) v = rnd(); }

extern "C" void feigen(double m, offset_t * q, size_t n_line, double * r, size_t n_iters);
// ^ parameter order layout as desired; rdi for stosd and rcx for loop, see feigen.S

void produce_line(line & v, double m, double d, size_t k_iters, size_t k, size_t n_threads = 1)
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
        for (auto q : buf) buckets[q >> bucket_bits].push_back(q);
        for (auto p : buckets) for (auto q : p) ++v[q];
    }
}

struct output_params { size_t w, i, n, x, s; string dir; };

string create_dir(size_t h, size_t i, size_t n)
{
    ostringstream buf;
    buf << "y" << i << "-" << n << "." << h;
    string r = buf.str();
    mkdir(r.c_str(), 0755);
    return r;
}

FILE * open_file(string dir, size_t x, const char * mode)
{
    ostringstream buf;
    buf << dir << "/" << setfill('0') << setw(6) << x << ".pnm";
    return fopen(buf.str().c_str(), mode);
}

void create_files(output_params par)
{
    for (size_t x = par.x; x < par.w; x += par.s) {
        FILE * grl_f = open_file(par.dir, x, "w");
        fprintf(grl_f, "P5\n%zu %zu\n65535\n", par.s, par.n - par.i);
        fclose(grl_f);
    }
}

// note: not srgb gamma, but currenly ouputs linear even tho pnm

void append_grl(count_t * counts, size_t s, FILE * f)
{
    double u = (1 << 16) / (1.0 + *max_element(counts, counts + s));
    vector<uint8_t> buf;
    buf.reserve(s * 2);
    for (size_t i = 0; i < s; i++) {
        unsigned g = counts[i] * u;
        buf.push_back(g >> 8);
        buf.push_back(g);
    }
    if (buf.size() != fwrite(buf.data(), 1, buf.size(), f)) {
        fprintf(stderr, "short fwrite");
        exit(1);
    }
}

void append_files(output_params par, count_t * counts)
{
    for (size_t x = par.x; x < par.w; x += par.s) {
        FILE * grl_f = open_file(par.dir, x, "a");
        append_grl(counts + x, par.s, grl_f);
        fclose(grl_f);
    }
}

int main(int argc, char ** argv)
{
    if (argc != 8) {
        fputs("args: w h i n x s p\n", stderr);
        return 1;
    }
    size_t w = atoi(*++argv);
    size_t h = atoi(*++argv);
    size_t i = atoi(*++argv);
    size_t n = atoi(*++argv);
    size_t x = atoi(*++argv);
    size_t s = atoi(*++argv);
    size_t p = atoi(*++argv);
    size_t z = p >= 2 ? 40000u : p >= 1 ? 4000u : 400u;
    double a = 3.5;
    double b = 4.0;
    srand(time(NULL));
    output_params par = {
        w, i, n, x, s, create_dir(h, i, n) };
    create_files(par);
    double d = (b - a) / h;
    for (size_t j = i; j < n; j++) {
        line v(w);
        produce_line(v, a + d * j, d, z, z);
        append_files(par, v.data());
        fprintf(stderr, "\r%zu", j);
    }
    fprintf(stderr, "\n");
}
