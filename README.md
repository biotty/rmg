# Rho Media Generators

## Provided Tools

### Sound

* **amuse** editor and renderer of compositions uses **libsynth**
* **libsynth** unit-generator based framework and instruments
* **fuge** python loadable module using **libsynth**
* **compo** example script generating music with fuge
* **noise** stand-alone script generating band-noise
* **mrender** stereo renderer of MIDI with its own synth
* **digitarp** stand-alone wah and Karpluss-Strong guitar sounds

### Graphics

* **rayt** optics of overlapping and intersecting basic shapes
* **fluid** traces a heterogenous viscous fluid in a plane
* **follow** border-hunting zoomer for Mandelbrot and its Julia-set
* **lindo** context-sensitive parametric bracketed L-system
* **avrepr** a module allowing non-text ``__repr__``
* **icons** console using **avrepr** for tuples and lists saves video
* **karaoke** assembles lyrics extracted from **mrender** onto video

## Prerequisites

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
