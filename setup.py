'''
    Run `python setup.py` to build the whl package
    Install with `pip install dist/*whl`
'''
import os
from setuptools import setup, Extension
from setuptools.dist import Distribution
from pip._internal.req import parse_requirements


class BinaryDistribution(Distribution):
    def is_pure(self):
        return False


if __name__ == "__main__":

    if os.name == 'posix':
        os.environ['CFLAGS'] = '-std=c++14 -fopenmp -m64 -O3'
        os.environ['CPPFLAGS'] = os.environ['CFLAGS']
        extra_compile_args = []
    else:
        extra_compile_args = [
            '/std:c++14',
            '/openmp'
        ]

    try: 
        import cython
        
        if cython.__version__ != '0.26':
            raise ImportError("Cython v0.26 is required. Install it with `pip install Cython==0.26`.")

        from Cython.Build import cythonize

        ext_modules = cythonize([
            Extension(
                '*',
                [
                    'biopharma_scheduling/deterministic/single_site_simple.pyx'
                ],
                language='c++',
                extra_compile_args=extra_compile_args
            )
        ])
    except ImportError as e:
        raise ImportError("Cython v0.26 is required. Install it with `pip install Cython==0.26`.")

    setup(
        name='biopharma-scheduling',
        packages=['biopharma_scheduling'],
        version='1.0',
        description='''
            Experimental tool for capacity planning and scheduling of biopharmaceutical facilities.
            ''',
        author='Karolis Jankauskas',
        author_email='karolis.jankauskas@gmail.com',
        url='https://github.com/karolisjan/ContinuousTimeCapacityPlanning',
        keywords = ['genetic algorithmn', 'biopharmaceutical', 'capacity planning', 'scheduling'], 
        classifiers=[
            'Programming Language :: Python',
            'Programming Language :: Python :: 3.5',
            'Development Status :: 4 - Beta',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: Microsoft :: Windows',
            'Operating System :: POSIX',
            'Intended Audience :: Developers',
            'Intended Audience :: Science/Research',
        ],
        python_requires='>=3.5',
        install_requires=[str(r.req) for r in parse_requirements('requirements.txt', session='hack')],
        script_args=['build_ext', 'bdist_wheel'],
        options={'build_ext': {'inplace': True, 'force': True}},
        ext_modules=ext_modules,
        include_package_data=True,
        distclass=BinaryDistribution
    )