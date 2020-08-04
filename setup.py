from distutils.util import convert_path
import os
import re
import sys
import platform
import subprocess

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __str__(self):
        import pybind11
        return pybind11.get_include()




# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    import os
    with tempfile.NamedTemporaryFile('w', suffix='.cc', delete=False) as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        fname = f.name
    try:
        compiler.compile([fname], extra_postargs=[flagname])
    except setuptools.distutils.errors.CompileError:
        return False
    finally:
        try:
            os.remove(fname)
        except OSError:
            pass
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14/17] compiler flag.
    The newer version is prefered over c++11 (when it is available).
    """
    #flags = ['-std=c++17', '-std=c++14', '-std=c++11']
    flags = ['-std=c++11']

    for flag in flags:
        if has_flag(compiler, flag):
            return flag

    raise RuntimeError('Unsupported compiler -- at least C++11 support '
                       'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }
    l_opts = {
        'msvc': [],
        'unix': [],
    }

    if sys.platform == 'darwin':
        darwin_opts = ['-stdlib=libc++', '-mmacosx-version-min=10.7']
        c_opts['unix'] += darwin_opts
        l_opts['unix'] += darwin_opts

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        link_opts = self.l_opts.get(ct, [])
        if ct == 'unix':
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')

        for ext in self.extensions:
            ext.define_macros = [('VERSION_INFO', '"{}"'.format(self.distribution.get_version()))]
            ext.extra_compile_args = opts
            ext.extra_link_args = link_opts
        build_ext.build_extensions(self)

"""
Each individual extension, i.e. .so file that needs to be compiled and created
in order for the qufilab package to successfully import these files needs to be
included below.
Each extension needs to have it's included source files, i.e. .cc files (check
implementation and see which files needs to be included).

OBSERVE! If the source-files haven't been included correctly, python will
raise an ImportError. For example, if 'stat' extension doesn't include 
'trend.cc' sourcefile, the function 'std (standard deviation)' won't work since it
depends on functions from the trend source file.
"""
ext_modules = [
    # Models extension
    Extension(
        'qufilab.models',
        sorted(['qufilab/models.cc',
            'qufilab/common/time.cc', 
            'qufilab/indicators/volatility.cc',
            'qufilab/indicators/trend.cc']),
        include_dirs=[
            get_pybind_include(),
        ],
        language='c++'
    ),
    # Trend extension
    Extension(
        'qufilab.indicators.trend',
        sorted(['qufilab/indicators/trend.cc']),
        include_dirs=[
            get_pybind_include(),
        ],
        language='c++'
    ),
    # Volatility extension
    Extension(
        'qufilab.indicators.volatility',
        sorted(['qufilab/indicators/volatility.cc',
            'qufilab/indicators/trend.cc',
            'qufilab/indicators/stat.cc']),
        include_dirs=[
            get_pybind_include(),
        ],
        language='c++'
    ),
    # Momentum extension
    Extension(
        'qufilab.indicators.momentum',
        sorted(['qufilab/indicators/momentum.cc',
            'qufilab/indicators/trend.cc']),
        include_dirs=[
            get_pybind_include(),
        ],
        language='c++'
    ),
    # Volume extension
    Extension(
        'qufilab.indicators.volume',
        sorted(['qufilab/indicators/volume.cc',
            'qufilab/indicators/trend.cc']),
        include_dirs=[
            get_pybind_include(),
        ],
        language='c++'
    ),
    # Stat extension
    Extension(
        'qufilab.indicators.stat',
        sorted(['qufilab/indicators/stat.cc',
            'qufilab/indicators/trend.cc']),
        include_dirs=[
            get_pybind_include(),
        ],
        language='c++'
    ),
]

PACKAGES = ['qufilab']

setup(
    name="qufilab",
    author="Anton Normelius",
    author_email="a.normelius@gmail.com",
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.6",
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt},
    zip_safe = False,
)
    
