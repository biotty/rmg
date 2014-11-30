# Random Media Generators

## Prerequisites

### To Compile

* python-dev --- for **fuge**
* python-pygame --- for **icons**
* libsdl2-dev --- for **amuse**
* libncurses-dev --- for **amuse**

### To Generate Media

* lame
* avconv

## Provided Tools

### Sound

* **amuse** --- editor and render of compositions uses **libsynth**
* **libsynth** --- a unit-generator based framework and instruments
* **fuge** --- a python loadable module using **libsynth**
* **compo.py** --- an example script generating music with fuge
* **stand_noise.py** --- a stand-alone script generating noise
* **mrender** --- a stereo renderer of MIDI, with its own synth
* **digitarp** --- stand-alone wah and karpluss-strong guitar sounds

### Graphics

* **cray** --- optics of overlapping and intersecting basic shapes
* **cfluid** --- traces a heterogenous viscous fluid in a plane
* **fz.py** --- border-hunting zoomer for mandelbrot and its juliaset
* **ls.py** --- context-sensitive parametric bracketed L-system
* **avrepr** --- a **pygame** module allowing non-text ``__repr__``
* **icons** --- a python console using avrepr for lists, can record
* **karaoke.py** --- assembles lyrics extracted from mrender onto video
