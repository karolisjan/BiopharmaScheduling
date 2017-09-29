'''
    $ python setup.py build_ext --inplace 
'''
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

sourcefiles  = [
        'src/pyx/lakhdar2005_continuous.pyx',
        ]

ext = [Extension(
        '*',
        sourcefiles,
        language='c++',
        extra_compile_args=[
                '/std:c++14', 
                '/openmp'
                ])]

setup(
  ext_modules=cythonize(ext)
)