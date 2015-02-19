//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "Python.h"
#include "unit.hpp"
#include "generators.hpp"
#include "builders.hpp"
#include "musicm.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <cassert>
#include <cmath>

namespace {

struct UgenObject {
    PyObject_HEAD
    ug_ptr g;
};

PyObject *
Ugen_new(PyTypeObject * type, PyObject * args, PyObject * kw)
{
    PyObject * self = type->tp_alloc(type, 0);
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    void * p = new (&u.g) ug_ptr;
    assert(p == (void *)&u.g);
    (void) p;  // compile: otherwise "unused"
    return self;
}

void
Ugen_dealloc(PyObject * self)
{
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    u.g.ug_ptr::~ug_ptr();
    self->ob_type->tp_free(self);
}

PyObject *
Ugen_call(PyObject * self, PyObject * args, PyObject * kw)
{
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    int16_t e[SU];
    int z = sizeof e;
    if (u.g && u.g->more()) {
        unit a;
        u.g->generate(a);
        FOR_SU(i) e[i] = 32766 * a.y[i];
    } else {
        z = 0;
    }
    return PyString_FromStringAndSize((char *)e, z);
}

PyTypeObject UgenType = {
    PyObject_HEAD_INIT(NULL)
    0, "fuge.Ugen", sizeof (UgenObject), 0, Ugen_dealloc,
    0, 0, 0, 0, 0, 0, 0, 0, 0, Ugen_call,
    0, 0, 0, 0, Py_TPFLAGS_DEFAULT, "encoded-unit generator",
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Ugen_new,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

long
parse_int(PyObject * n)
{
    PyObject * i = PyNumber_Int(n);
    const long x = PyInt_AsLong(i);
    Py_DECREF(i);
    return x;
}

double
parse_float(PyObject * n)
{
    PyObject * f = PyNumber_Float(n);
    const double x = PyFloat_AsDouble(f);
    Py_DECREF(f);
    return x;
}

struct param
{
    std::vector<double> values;
    double get() const { return values.at(0); }
    std::size_t size() const { return values.size(); }
};

typedef std::vector<param> params;

params
parse_params(PyObject * seq)
{
    params r;
    if ( ! PyList_Check(seq)) throw std::runtime_error("params isn't List");
    const int n = PyList_Size(seq);
    for (int i=0; i<n; i++) {
        PyObject * o = PyList_GetItem(seq, i);
        param s;
        if ( ! PyList_Check(o)) {
            s.values.push_back(parse_float(o));
        } else {
            const int k = PyList_Size(o);
            for (int j=0; j<k; j++)
                s.values.push_back(parse_float(PyList_GetItem(o, j)));
        }
        r.push_back(s);
    }
    return r;
}

struct instrument
{
    bu_ptr operator()(double span, PyObject * list)
    {
        return play(span, parse_params(list));
    }
    virtual bu_ptr play(double span, params const &) = 0;
    virtual ~instrument() {}
};

en_ptr
mk_envelope(param const & p)
{
    std::vector<double> const & v = p.values;
    unsigned n = v.size();
    if ( ! n) throw std::runtime_error("list-param empty");
    if (n == 1) return P<constant>(v[0]);

    pe_ptr e = P<punctual>(v[0], v[n - 1]);
    for (unsigned i=1; i+1<n; i++) e->p(i /(double) n, v[i]);
    return e;
}

struct tense_string : instrument
{
    bu_ptr play(double span, params const & p)
    {
        if (p.size() != 6) throw std::runtime_error(
                "tense_string requires 6 params");
        const double d = span * p[0].get();
        en_ptr e = mk_envelope(p[3]);
        return P<attack>(p[2].get() / p[3].get(), p[1].get(), d,
                P<karpluss_strong>(P<movement>(e, d),
                    p[4].get(), p[5].get()));
    }
};

struct mouth : instrument
{
    bu_ptr play(double span, params const & p)
    {
        if (p.size() != 8) throw std::runtime_error(
                "mouth requires 8 params");
        const double d = span * p[0].get();
        mv_ptr f = P<movement>(mk_envelope(p[3]), d);
        bu_ptr a = P<harmonics>(f, mk_envelope(p[4]), 9, p[5].get());
        bu_ptr b = P<harmonics>(f, mk_envelope(p[6]), 9, p[7].get());
        return P<attack>(p[2].get() / p[3].get(), p[1].get(), d,
                P<cross>(a, b, P<movement>(P<punctual>(0, 1), d)));
    }
};

struct fqm : instrument
{
    bu_ptr play(double span, params const & p)
    {
        if (p.size() != 6) throw std::runtime_error(
                "fm requires 6 params");
        const double d = span * p[0].get();
        en_ptr modulator = mk_envelope(p[3]);
        en_ptr index = mk_envelope(p[4]);
        en_ptr carrier = mk_envelope(p[5]);
        bu_ptr m = P<wave>(P<movement>(modulator, d), P<sine>(0));
        mv_ptr i = P<movement>(index, d);
        mv_ptr c = P<movement>(carrier, d); 
        return P<attack>(p[2].get() / p[3].get(), p[1].get(), d,
                P<fm>(m, i, c));
    }
};

std::map<std::string, std::unique_ptr<instrument>> orchestra;

std::string
parse_string(PyObject * s)
{
    if (char * p = PyString_AsString(s))
        return p;
    else
        return "";
}

void
init_orchestra()
{
    orchestra.emplace("tense_string", std::unique_ptr<instrument>(new tense_string));
    orchestra.emplace("mouth", std::unique_ptr<instrument>(new mouth));
    orchestra.emplace("fqm", std::unique_ptr<instrument>(new fqm));
}

namespace fuge { // because ::filter exists

struct filter
{
    bu_ptr  operator()(double span, bu_ptr input, PyObject * list)
    {
        return apply(span, input, parse_params(list));
    }
    virtual bu_ptr apply(double span, bu_ptr input, params const &) = 0;
    virtual ~filter() {}
};

}

struct echo : fuge::filter
{
    bu_ptr apply(double span, bu_ptr input, params const & p)
    {
        if (p.size() != 2) throw std::runtime_error(
                "echo requires 2 params");
        en_ptr mix = mk_envelope(p[0]);
        en_ptr delay = mk_envelope(p[1]);
        const double d = span + p[1].get();
        fl_ptr lf = P<feedback>(P<as_is>(), P<movement>(mix, d), delay);
        return P<timed_filter>(input, lf, d);
    }
};

struct comb : fuge::filter
{
    bu_ptr apply(double span, bu_ptr input, params const & p)
    {
        if (p.size() != 2) throw std::runtime_error(
                "comb requires 2 params");
        en_ptr mix = mk_envelope(p[0]);
        en_ptr delay = mk_envelope(p[1]);
        const double d = span + p[1].get();
        fl_ptr lf = P<feed>(P<as_is>(), P<movement>(mix, d), delay);
        return P<timed_filter>(input, lf, d);
    }
};

std::map<std::string, std::unique_ptr<fuge::filter>> effects;

void
init_effects()
{
    effects.emplace("comb", std::unique_ptr<fuge::filter>(new comb));
    effects.emplace("echo", std::unique_ptr<fuge::filter>(new echo));
}

bu_ptr
parse_note(PyObject * seq)
{
    if ( ! PyTuple_Check(seq)) throw std::runtime_error("note isn't Tuple");
    const int n = PyTuple_Size(seq);
    if (n != 3) throw std::runtime_error("note Tuple isn't size 3");
    const double span = parse_float(PyTuple_GetItem(seq, 0));
    std::string label = parse_string(PyTuple_GetItem(seq, 1));
    return (*orchestra.at(label))(span, PyTuple_GetItem(seq, 2));
}

bu_ptr
parse_score(PyObject * seq);

struct score_entry {
    double t;
    bu_ptr b;
    score_entry() : t() {}
    score_entry(double t, PyObject * data) : t(t)
    {
        if (PyTuple_Check(data)) b = parse_note(data);
        else if (PyList_Check(data)) b = parse_score(data);
        else throw std::runtime_error(
                "score entry data isn't Tuple or List");
    }
};

score_entry
parse_entry(PyObject * seq)
{
    if ( ! PyTuple_Check(seq)) throw std::runtime_error("score entry isn't Tuple");
    const int n = PyTuple_Size(seq);
    if (n != 2) throw std::runtime_error("score entry Tuple doesn't have size 2");
    PyObject * head = PyTuple_GetItem(seq, 0);
    const double t = parse_float(head);
    return score_entry(t, PyTuple_GetItem(seq, 1));
}

bu_ptr
parse_filter(bu_ptr input, PyObject * seq)
{
    const int n = PyTuple_Size(seq);
    if (n != 3) throw std::runtime_error("effect isn't tripple");
    const double span = parse_float(PyTuple_GetItem(seq, 0));
    std::string label = parse_string(PyTuple_GetItem(seq, 1));
    return (*effects.at(label))(span, input, PyTuple_GetItem(seq, 2));
}

bu_ptr
parse_score(PyObject * seq)
{
    const int n = PyList_Size(seq);
    if (n == 0) throw std::runtime_error("empty score-list");
    PyObject * fl = PyList_GetItem(seq, 0);
    if ( ! PyList_Check(fl))
        throw std::runtime_error("first score-entry must be filter-list");
    sc_ptr sc = P<score>();
    bu_ptr rt = sc;
    const int k = PyList_Size(fl);
    for (int j=0; j<k; j++) {
        rt = parse_filter(rt, PyList_GetItem(fl, j));
    }
    for (int i=1; i<n; i++) {
        score_entry se = parse_entry(PyList_GetItem(seq, i));
        sc->add(se.t, se.b);
    }
    return rt;
}

PyObject *
render(PyObject * self, PyObject * args)
{
    PyObject * data;
    if ( ! PyArg_ParseTuple(args, "O", &data))
        return NULL;
    score_entry se(0, data);
    PyObject * o = PyObject_CallObject((PyObject *)&UgenType, NULL);
    reinterpret_cast<UgenObject *>(o)->g = P<limiter>(se.b->build());
    return o;
}

PyObject *
just(PyObject * self, PyObject * args)
{
    double pitch;
    if ( ! PyArg_ParseTuple(args, "d", &pitch))
        return NULL;
    long lower = pitch;
    double fraction = pitch - lower;
    double f = f_of_just(lower);
    if (fraction > 0.0001) {
        const double g = f_of_just(lower + 1) - f;
        f += fraction * g;
    }
    return PyFloat_FromDouble(f);
}

PyMethodDef FugeMethods[] = {
    { "render", render, METH_VARARGS, "render." },
    { "just", just, METH_VARARGS, "just." },
    { NULL, NULL, 0, NULL }, /* Sentinel */
};

} //namespace

PyMODINIT_FUNC
initfuge(void)
{
    UgenType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&UgenType) < 0) return;
    PyObject * m = Py_InitModule("fuge", FugeMethods);
    if (m == NULL) return;
    Py_INCREF(&UgenType);
    PyModule_AddObject(m, "Ugen", (PyObject *)&UgenType);
    init_orchestra();
    init_effects();
}
