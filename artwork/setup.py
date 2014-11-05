from distutils.core import setup, Extension

fuge = Extension('fuge',
        extra_compile_args = ['--std=c++11', '-I../sound/include'],
        extra_objects = ['../sound/lib/synth.a'],
        sources = ['fugemodule.cpp'])

setup(name = 'fuge', version = '1.0',
       description = 'fuge',
       ext_modules = [fuge])
