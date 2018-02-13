'''
    $ python setup.py build_ext --inplace 
'''
import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

sourcefiles  = [
        'src/continuous_time_capacity_planning.pyx',
        ]

if os.name == 'posix':
    os.environ["CC"] = "g++"
    os.environ["CXX"] = "g++"
    os.environ['CFLAGS'] = '-std=c++14 -fopenmp'
    extra_compile_args = []
else:
    extra_compile_args = [
        '/std:c++14',
        '/openmp'
    ]

ext = [
   Extension(
       '*',
       sourcefiles,
       language='c++',
       extra_compile_args=extra_compile_args,
   )
]

setup(
    name='continuous_time_capacity_planning',
    ext_modules=cythonize(ext)
)