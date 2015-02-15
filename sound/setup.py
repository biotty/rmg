#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from distutils.core import setup, Extension

fuge = Extension('fuge',
        extra_compile_args = ['-std=c++11', '-Isynth'],
        extra_objects = ['synth/sizr.a'],
        sources = ['fugemodule.cpp'])

setup(name = 'fuge', version = '1.1',
       description = 'fuge',
       ext_modules = [fuge])
