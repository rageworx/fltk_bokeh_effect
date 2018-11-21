# An experimental proejct for Bokeh effect image processing.
This is an experimental image processing project for bokeh effect.

## Referenced from,
https://www.scratchapixel.com/code.php?id=51&origin=/lessons/digital-imaging/simple-image-manipulations

## Required libraries
* FLTK-1.3.4-2-ts
* fl_imgtk

## Required platform
* Above OpenMP 3.5
* Any, with G++ ( above C++11 )
* Should be avoid C++11 error by using Makefile.linuxgcc5

## Lastest update
 * Version 0.1.8.35
     1. Developing ProcessFastBokeh().
	 1. ProcessFastBokeh() using less memory for mask.
	 1. Little bit more faster than legacy code.

## How to build ?
* Reserved conditions :
    - fltk-1.3.4-2-ts was built and installed on your system.
    - make sure fl_imgtk was built.
    - put fl_imgtk in same level of directory of this project.
```
(project root)/fl_imgtk
(project root)/fltk_bokeh_effect
```
* just type make.
