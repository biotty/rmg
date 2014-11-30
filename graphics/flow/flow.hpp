//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef FLOW_HPP
#define FLOW_HPP

#include "cellular.hpp"
#include "planar.hpp"


template<class T>
struct Flow;


template<class T>
struct FlowFunction
{
    virtual ~FlowFunction() {}
    virtual bool operator()(Grid<T> * flow_swap, Grid<T> * flow, double step_t) = 0;
};


template<class T>
struct Flow
{
    Grid<T> field_a;
    Grid<T> field_b;
    Grid<T> * field;
    Grid<T> * field_swap;

    Flow(size_t h, size_t w)
            : field_a(h, w) , field_b(h, w)
            , field(&field_a) , field_swap(&field_b)
    {}

    void lapse(double delta_t, size_t & n_steps,
            std::vector<FlowFunction<T>*> & functions)
    {
        for (size_t step = 0; step < n_steps; ++step)
            for (size_t i = 0; i < functions.size(); ++i) {
                re:try {
                    if ((*functions[i])(field_swap, field, delta_t/n_steps))
                        std::swap(field, field_swap);
                } catch (double f) {
                    assert(i == 0);  //a "guard"-function must be the first one
                    n_steps *= 2;
                    step *= 2;
                    std::cerr << f << " -- augments #steps to " << n_steps << "\n";
                    goto re;
                }
            }
    }
};


template<class T>
struct CompositeFunction : FlowFunction<T>
{
    std::vector<FlowFunction<T> *> functions;

    CompositeFunction(std::vector<FlowFunction<T> *> v) : functions(v) {}
    bool operator()(Grid<T> * field_swap, Grid<T> * field, double step_t)
    {
        Grid<T> * a = field_swap;
        Grid<T> * b = field;
        for (size_t k = 0; k < functions.size(); ++k)
            if ((*functions[k])(a, b, step_t))
                std::swap(a, b);
        
        return (a == field);
    }
};


template<class T>
struct EdgeFunction : FlowFunction<T>
{
    EdgeFunction() {}
    bool operator()(Grid<T> * /*field_swap*/, Grid<T> * field, double /*step_t*/)
    {
        size_t h = field->h;
        size_t w = field->w;
        for (size_t i = 0; i < h; ++i) {
            field->cell(i, 0) = xy::zero;
            field->cell(i, w - 1) = xy::zero;
        }
        for (size_t j = 0; j < w; ++j) {
            field->cell(0, j) = xy::zero;
            field->cell(h - 1, j) = xy::zero;
        }
        return false;
    }
};


#endif

