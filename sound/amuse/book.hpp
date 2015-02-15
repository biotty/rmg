//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef BOOK_HPP
#define BOOK_HPP

#include "page.hpp"
#include "screen.hpp"
#include <vector>

struct book
{
    std::vector<std::shared_ptr<page>> pages;
    page * current;
    page * copy_from;
    screen & s;

    book(screen * s);
    void unmark();
    unsigned effect_end(unsigned k, unsigned p, unsigned q);
    mv_ptr glide(unsigned k, unsigned p, unsigned q);
    bu_ptr link(unsigned k, unsigned p, unsigned q);
    void print_h();
    void flip(unsigned i);
    void handle(int ch);
};

#endif
