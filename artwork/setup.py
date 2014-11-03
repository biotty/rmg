from distutils.core import setup, Extension

fuge = Extension('fuge',
        extra_compile_args = ['--std=c++11'],
        extra_objects = ['../sound/synth.a'],
        sources = ['fugemodule.cpp'])

setup(name = 'fuge', version = '1.0',
       description = 'fuge',
       ext_modules = [fuge])
