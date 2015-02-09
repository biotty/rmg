#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from distutils.core import setup, Extension

fuge = Extension('fuge',
        extra_compile_args = ['-std=c++11', '-Iinclude'],
        extra_objects = ['lib/synth.a'],
        sources = ['src/fugemodule.cpp'])

setup(name = 'fuge', version = '1.0',
       description = 'fuge',
       ext_modules = [fuge])
