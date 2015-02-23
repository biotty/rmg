# Random Media Generators

## Prerequisites

### To Compile

* **python-dev** for **fuge**
* **python-pygame** for **icons**
* **libsdl2-dev** for **amuse**
* **libncurses-dev** for **amuse**

### To Generate Media

* **pngquant** for **cfluid** which operates on a limited palette
* **lame** for adding sound to videos like done with **karaoke**
* **avconv** for generating video from images of f.ex  **border**

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

* **cray** optics of overlapping and intersecting basic shapes
* **cfluid** traces a heterogenous viscous fluid in a plane
* **border** border-hunting zoomer for Mandelbrot and its Julia-set
* **lindo** context-sensitive parametric bracketed L-system
* **avrepr** a **pygame** module allowing non-text ``__repr__``
* **icons** console using avrepr for tuples and lists saves video
* **karaoke** assembles lyrics extracted from **mrender** onto video
