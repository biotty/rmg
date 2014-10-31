from distutils.core import setup, Extension

fuge = Extension('fuge',
        extra_compile_args = ['--std=c++11'],
                    sources = ['fugemodule.cpp',
                        '../sound/unit.cpp',
                        '../sound/envelopes.cpp',
                        '../sound/generators.cpp',
                        '../sound/builders.cpp',
                        '../sound/musicm.cpp',
                        '../sound/filters.cpp'])

setup(name = 'fuge', version = '1.0',
       description = 'fuge',
       ext_modules = [fuge])
