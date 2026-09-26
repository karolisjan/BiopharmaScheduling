# Biopharma Scheduling

Biopharma Scheduling is a genetic algorithm based tool for medium-term capacity planning and scheduling of multi-product biopharmaceutical facilities.

The continuous-time scheduling model was presented at the 27th European Symposium on Computer Aided Process Engineering:

> Jankauskas, K., Papageorgiou, L. G., & Farid, S. S. (2017). Continuous-Time Heuristic Model for Medium-Term Capacity Planning of a Multi-Suite, Multi-Product Biopharmaceutical Facility. In *Computer Aided Chemical Engineering* (Vol. 40, pp. 1303-1308). Elsevier. [DOI: 10.1016/B978-0-444-63965-3.50219-1](https://doi.org/10.1016/B978-0-444-63965-3.50219-1).

## Requirements

- Python 3.9 or later
- A C++14 compiler

On Apple Silicon, the system Clang compiler works out of the box. The extension uses a serial implementation by default, so installing OpenMP is optional.

## Install on macOS or Linux

Create and activate a virtual environment, then install the project and its notebook dependencies:

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -e '.[notebooks]'
```

For a minimal runtime installation, use `python -m pip install -e .`. The editable install compiles the Cython/C++ extension for the active Python interpreter and machine architecture.

Docker is optional for local development. A virtual environment keeps the compiled extension and Jupyter kernel tied to the same Python interpreter, avoiding imports from a different Conda environment or machine architecture.

To enable parallel evaluation on macOS, install GCC with Homebrew and build using its compiler (replace the version with the installed one):

```sh
brew install gcc
CC=/opt/homebrew/bin/gcc-15 CXX=/opt/homebrew/bin/g++-15 BIOPHARMA_OPENMP=1 \
  python -m pip install -e '.[notebooks]'
```

Without `BIOPHARMA_OPENMP=1`, the extension builds without OpenMP and uses one thread. Linux builds can opt into OpenMP the same way when GCC is installed.

## Docker

Build and start the JupyterLab image from the repository root:

```sh
docker build -t biopharma-scheduling/lab -f docker/lab.docker .
docker run --rm -it -p 8888:8888 -v "$PWD":/BiopharmaScheduling \
  biopharma-scheduling/lab \
  jupyter lab --ip 0.0.0.0 --no-browser --allow-root
```

Open the URL and token printed by JupyterLab, then browse the `examples` directory.

## Examples

The `examples` directory contains Jupyter notebooks for deterministic and stochastic single-site and multi-suite scheduling models. Activate the environment used for installation and launch JupyterLab from the repository root:

```sh
jupyter lab
```

If JupyterLab is installed in a different environment, register the project environment as a kernel and select it in the notebook:

```sh
source .venv/bin/activate
python -m ipykernel install --user --name biopharma-scheduling \
  --display-name "Biopharma Scheduling (Python)"
```

Restart the notebook kernel after reinstalling the package or changing compiled code. Campaign and task Gantt charts use Plotly timelines. Stochastic objectives and constraints use names ending in `_mean` (for example, `total_kg_inventory_deficit_mean`).

## Tests

With the project environment active, run the Python tests with:

```sh
python -m unittest discover -s tests -p 'tests.py'
```

The C++ test suite can be compiled and run with a C++14 compiler:

```sh
c++ -O2 -std=c++14 tests/tests.cpp -o /tmp/biopharma-scheduling-tests
/tmp/biopharma-scheduling-tests
```

Seeded genetic algorithm runs can produce different schedules across C++ standard-library implementations. Tests that assert a particular optimizer outcome may therefore differ across platforms, while schedule validity and objective calculations can be checked independently.
