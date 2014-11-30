# Random Media Generators

## Pre-requisites

### To Compile

* python-dev --- for **fuge**
* python-pygame --- for **icons**
* libsdl2-dev --- for **amuse**
* libncurses-dev --- for **amuse**

### To Generate Media

* lame
* avconv

## Tools Provided

### Sound

* **amuse** editor and render compositions uses **libsynth**
* **libsynth** a unit-generator based framework and instruments
* **fuge** is a python loadable module using **libsynth**
* **compo.py** an example script generating music
* **stand_noise.py** a stand-alone script generating noise
* **mrender** a stereo renderer of MIDI, with its own synth
* **digitarp** stand-alone guitar sounds

### Graphics

* **cray** optics of overlapping and intersecting basic shapes
* **cfluid** traces a heterogenous viscous fluid in a plane
* **fz.py** border-hunting mandelbrot and juliaset combined zoomer
* **ls.py** context-sensitive parametric bracketed L-system
* **karaoke.py** assembles lyrics extracted from mrender onto video
* **icons.py** a python console using avrepr for lists, can record
* **avrepr.py** a **pygame** module allowing non-text ``__repr__``

