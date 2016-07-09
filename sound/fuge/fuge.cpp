//      © Christian Sommerfeldt Øien
//      All rights reserved
#include "Python.h"
#include "fugelib.hpp"
#include "generators.hpp"
#include "builders.hpp"
#include "musicm.hpp"
#include <algorithm>
#include <string>

namespace {

struct UgenObject {
    PyObject_HEAD
    generator * master; // owned here

    sum * left; // borrowed from above
    sum * right; // ^
};

PyObject *
Ugen_new(PyTypeObject * type, PyObject * /*args*/, PyObject * /*kw*/)
{
    PyObject * self = type->tp_alloc(type, 0);
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    u.master = nullptr;
    u.left = nullptr;
    u.right = nullptr;
    return self;
}

void
Ugen_dealloc(PyObject * self)
{
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    delete u.master;  // left and right goes with it
    self->ob_type->tp_free(self);
}

PyObject *
Ugen_call(PyObject * self, PyObject * /*args*/, PyObject * /*kw*/)
{
    UgenObject & u = *reinterpret_cast<UgenObject *>(self);
    int16_t e[unit::size];
    int z = sizeof e;
    if (u.master && u.master->more()) {
        unit a;
        u.master->generate(a);
        std::transform(std::begin(a.y), std::end(a.y),
                std::begin(e),
                [](double q) -> int16_t { return 32766 * q; });
    } else {
        z = 0;
    }
    return PyBytes_FromStringAndSize((char *)e, z);
}

PyTypeObject UgenType;

} // namespace

namespace fuge {

PyObject *
mono(PyObject * /*self*/, PyObject * args)
{
    if ( ! PyArg_ParseTuple(args, ""))
        return NULL;
    PyObject * o = PyObject_CallObject((PyObject *)&UgenType, NULL);
    UgenObject * u = reinterpret_cast<UgenObject *>(o);
    u->right = new sum();
    // leave left
    u->master = new limiter(ug_ptr(u->right));
    return o;
}

PyObject *
stereo(PyObject * /*self*/, PyObject * args)
{
    if ( ! PyArg_ParseTuple(args, ""))
        return NULL;
    PyObject * o = PyObject_CallObject((PyObject *)&UgenType, NULL);
    UgenObject * u = reinterpret_cast<UgenObject *>(o);
    u->left = new sum();
    u->right = new sum();
    u->master = new limiter(U<inter>(ug_ptr(u->left), ug_ptr(u->right)));
    return o;
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
parse_score(PyObject * seq)
{
    const int n = PyList_Size(seq);
    if (n == 0) throw std::runtime_error("empty score-list");
    PyObject * fl = PyList_GetItem(seq, 0);
    if ( ! PyList_Check(fl))
        throw std::runtime_error("first score-entry must be filter-list");
    score * sc = new score();
    bu_ptr rt = bu_ptr(sc);
    const int k = PyList_Size(fl);
    for (int j=0; j<k; j++) {
        rt = parse_filter(std::move(rt), PyList_GetItem(fl, j));
    }
    for (int i=1; i<n; i++) {
        score_entry && se = parse_entry(PyList_GetItem(seq, i));
        sc->add(se.t * fuge::tempo, std::move(se.b));
    }
    return rt;
}

PyObject *
pan(PyObject * /*self*/, PyObject * args)
{
    PyObject * data;
    PyObject * uobj;
    double p;
    if ( ! PyArg_ParseTuple(args, "O!Od", &UgenType, &uobj, &data, &p))
        return NULL;
    UgenObject * ugen = reinterpret_cast<UgenObject *>(uobj);
    score_entry se(0, data);
    if ( ! ugen->left) { // mono
        ugen->right->c(se.b->build(), p);
    } else { // stereo
        const double pi = 3.141592653589;
        const double r = p * pi * .5;
        std::shared_ptr<generator> g = P<ncopy>(2, se.b->build());
        ugen->left->c(U<wrapshared>(g), std::cos(r));
        ugen->right->c(U<wrapshared>(g), std::sin(r));
    }
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
just(PyObject * /*self*/, PyObject * args)
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

} // namespace

namespace {

PyMethodDef methoddef[] = {
    { "mono", fuge::mono, METH_VARARGS, "mono." },
    { "stereo", fuge::stereo, METH_VARARGS, "stereo." },
    { "pan", fuge::pan, METH_VARARGS, "pan." },
    { "just", fuge::just, METH_VARARGS, "just." },
    { NULL, NULL, 0, NULL }, /* sentinel */
};

struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "fuge",
    "doc",
    -1,
    methoddef,
    NULL, NULL, NULL, NULL
};

} //namespace

PyMODINIT_FUNC
PyInit_fuge(void)
{
    char * e = getenv("FUGE_TEMPO");
    if (e && sscanf(e, "%lf", &fuge::tempo) != 1)
        return NULL;

    Py_TYPE(&UgenType) = &PyType_Type;
    Py_REFCNT(&UgenType) = 1;
    UgenType.tp_name = "fuge.Ugen";
    UgenType.tp_basicsize = sizeof (UgenObject);
    UgenType.tp_dealloc = Ugen_dealloc;
    UgenType.tp_call = Ugen_call;
    UgenType.tp_flags = Py_TPFLAGS_DEFAULT;
    UgenType.tp_doc = "audio unit generator";
    UgenType.tp_new = Ugen_new;
    if (PyType_Ready(&UgenType) < 0)
        return NULL;

    PyObject * m = PyModule_Create(&moduledef);
    if ( ! m)
        return NULL;

    Py_INCREF(&UgenType);
    PyModule_AddObject(m, "Ugen", (PyObject *)&UgenType);

    fuge::init_orchestra();
    fuge::init_filters();

    return m;
}
