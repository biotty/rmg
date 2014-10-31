#ifndef RINGBUF_HPP
#define RINGBUF_HPP

#include <cstring>

template<typename T>
class ringbuf
{
    T * a;
    unsigned head;
    unsigned tail;
    bool full;

public:
    const unsigned n;

    ringbuf(unsigned n) : a(new T[n]), head(), tail(), full(), n(n) {}
    ~ringbuf() { delete[] a; }

    void fill()
    {
        std::memset(a, 0, n * sizeof (T));
        full = true;
    }

    unsigned count()
    {
        if (head == tail) return full ? n : 0;
        else if (head > tail) return head - tail;
        else return head + (n - tail);
    }

    void skip()
    {
        if (++tail == n) tail = 0;
    }

    T get()
    {
        if (tail == head && ! full) return 0;
        T y = a[tail];
        skip();
        full = false;
        return y;
    }

    void put(T y)
    {
        a[head] = y;
        if (full) skip();
        if (++head == n) head = 0;

        full = (head == tail);
    }

    T tap(unsigned i)
    {
        if (i >= count()) return 0;

        unsigned prev = (head) ? head - 1 : n - 1;
        unsigned j = (i <= prev) ? prev - i : prev + n - i;
        return a[j];
    }
};

typedef ringbuf<double> ring_buffer;

#endif

