"""Build the Cython extension used by the scheduling models."""

import os
import sys

from Cython.Build import cythonize
from setuptools import Extension, setup


openmp = os.environ.get("BIOPHARMA_OPENMP", "0").lower() in {"1", "true", "yes"}
compile_args = ["/std:c++14"] if sys.platform == "win32" else ["-std=c++14", "-O2"]
link_args = []

if openmp:
    if sys.platform == "win32":
        compile_args.append("/openmp")
    else:
        compile_args.append("-fopenmp")
        link_args.append("-fopenmp")

extensions = [
    Extension(
        "biopharma_scheduling.single_site.deterministic",
        ["biopharma_scheduling/single_site/deterministic.pyx"],
        language="c++",
        extra_compile_args=compile_args,
        extra_link_args=link_args,
    ),
    Extension(
        "biopharma_scheduling.single_site.stochastic",
        ["biopharma_scheduling/single_site/stochastic.pyx"],
        language="c++",
        extra_compile_args=compile_args,
        extra_link_args=link_args,
    ),
]

setup(
    ext_modules=cythonize(
        extensions,
        compiler_directives={"language_level": 3},
    )
)
