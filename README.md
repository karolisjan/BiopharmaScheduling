<a id='index'></a>
# Continuous-Time Capacity Planning for Biopharmaceutical Facility

* [Setup](#setup)
    * [OS dependent steps](#OS_dependent)
        * [MacOS](#macos)
        * [Ubuntu](#ubuntu)
        * [Windows](#windows)
    * [OS independent steps](#OS_independent)
* [Running the demo](#demo)

This is a genetic algorithm (GA) based optimisation approach for medium-term capacity planning of a multi-product, multi-suite biopharmaceutical facility using a continuous-time representation. The continuous-time model is implemented by utilising a dynamic chromosome structure capable of adapting to the problem by growing in length from a single gene corresponding to a production campaign in a manufacturing schedule.

This software has been presented during a keynote lecture at the 27th European Symposium on Computer Aided Process Engineering (ESCAPE) and used in the following publication:

> Jankauskas, K., Papageorgiou, L. G., & Farid, S. S. (2017). Continuous-Time Heuristic Model for Medium-Term Capacity Planning of a Multi-Suite, Multi-Product Biopharmaceutical Facility. In *Computer Aided Chemical Engineering* (Vol. 40, pp. 1303-1308). Elsevier. **DOI:** [10.1016/B978-0-444-63965-3.50219-1](https://doi.org/10.1016/B978-0-444-63965-3.50219-1).

<a id='setup'></a>
## Setup 

<a id='OS_dependent'></a>
### OS dependent steps

<a id='macos'></a>
#### MacOS

* Install [`brew`](https://brew.sh/).
* Install the build tools with `brew install gcc --without-multilib`.
* Find the path to the `g++` binary with `brew ls gcc`. It should be `/usr/local/Cellar/gcc/<version>/bin/g++-<version>`. Only `g++ version 7.2.0` has been tested.
* Export the path to the `g++` binary with `export CC=<path to g++ binary> && export CXX=<path to g++ binary>` where `<path to g++ binary>` contains the path obtained from the previous step. 

[back to top](#index)

<a id='ubuntu'></a>
#### Ubuntu 16.04

* `echo "deb http://ftp.de.debian.org/debian testing main" >> /etc/apt/sources.list`
* `apt-get --force-yes update && apt-get --assume-yes install g++`

[back to top](#index)

<a id='windows'></a>
#### Windows

* Download and install the build [tools](https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017) for Visual Studio.

[back to top](#index)

<a id='OS_independent'></a>
### OS independent steps

* Install Python 3.6
* Create a virtual environment with `conda create -n <environment-name> python=3.6` if you have [`anaconda`](https://www.anaconda.com/download/#macos) installed or `virtualenv <environment-name>` if you are running vanilla Python (you might need to install `virtualenv` first with `pip install virtualenv`).
* Activate the virtual environment that was just created with `source activate <environment-name>` if you are using [`anaconda`](https://www.anaconda.com/download/#macos) or `source <environment-name>/bin/activate` otherwise. 
* Run the following commands one by one in the root directory of `ContinuousTimeCapacityPlanning`:
    ```
    python -m pip install pip==10.0.1
    pip install setuptools==38.6.0
    pip install Cython==0.26
    python setup.py
    cd dist
    pip install *.whl 
    ```

[back to top](#index)

<a id='demo'></a>
## Running the demo

* Setup the `ipykernel` for the environment created earlier
```
python -m ipykernel install --user --name <environment-name> --display-name "<display-name>"
```
* Launch `jupyter` in `ContinuousTimeCapacityPlanning/demo`
```
jupyter notebook
```

[back to top](#index)

