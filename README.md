# Rho Media Generators

Here is my source code for generation of sound and graphics as possible
tools and ingredients in artwork.  Algorithms and formulae were
studied, understood and incorporated in this collection of small
projects as a process over time.  The work is original and only
certain data immitated from other sources, in which case the
material was publicly available; i-e standard gamma-correction,
the points on an arhcimedic body, the SDL2 interface, the MIDI format.
It resulted from curiousity and study of friendly material on
subjects such as optics, fluid-mechanics, vocal-formants and fractals.
Note that unfortunately the material provided in scientific
communities are neither open nor friendly, which is why I thought
good to try and democratize these fascinating subjects.  Maybe
it may thus engage or at least inspire some fun programming.

## Provided Tools

Beware that the following index is not complete, as certain hidden gems are
also contained, such as a [carnatic](sound/py/carnatic.py) music generator
given a selected "raga" and implementation of the [Feigenbaum](graphics/baum)
fractal both in FORTH and x86 assembly parallelized by utilizing the SIMD mechanisms.
Also, the "rayt" ray-tracer has components that may be re-used such as "work.hpp"
that contains a sequential-work dispatcher that is here used for the (independent)
rendering of output pixels.  In addition to functions that calculate intersection
and normal for the convex quadratic shapes.  In the sound compartment, you find
signal processing tools such as the biquad and comb filter, but also synthetic
instruments such as Carpluss-Strong.

### Sound

* [amuse](sound/amuse/amuse.cpp) editor and renderer of compositions uses **libsynth**
* [libsynth](sound/synth) unit-generator based framework and instruments
* [fuge](sound/fuge/lib.cpp) python loadable module using **libsynth**
* [compo](sound/py/demo.py) example script generating music with fuge
* [noise](sound/py/noise.py) stand-alone script generating band-noise
* [mrender](sound/mrender) stereo renderer of MIDI with its own synth
* [digitarp](sound/digitarp.cpp) stand-alone wah and Karpluss-Strong guitar sounds

### Graphics

* [rayt](graphics/ray) optics of overlapping and intersecting basic shapes
* [fluid](graphics/flow/fluid.hpp) traces a heterogenous viscous fluid in a plane
* [follow](graphics/follow.cpp) border-hunting zoomer for Mandelbrot and its Julia-set
* [lindo](graphics/py/lindo.py) context-sensitive parametric bracketed L-system
* [avrepr](graphics/py/rmg/avrepr.py) a module allowing non-text ``__repr__``
* [icons](graphics/py/icons.py) console using **avrepr** for tuples and lists saves video
* [karaoke](graphics/py/karaoke.py) assembles lyrics extracted from **mrender** onto video

## Prerequisites

Prefer a POSIX compliant operating system.

### To Compile

* **C and C++ tool-chain** for the binaries
* **python3-dev** to build the **fuge** module
* **python-pygame** available to python for **icons**
* **libsdl2-dev** and **libncurses5-dev** for **amuse**

### To Generate Media

* **netpbm** used with a unix pipe in graphic tools internals
* **pngquant** for **fluid** which operates on a limited palette
* **lame** for adding sound to videos like done with **karaoke**
* **libav-tools** for generating video from images of f.ex  **follow**
