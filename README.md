<a id='index'></a>
# Continuous-Time Capacity Planning for Biopharmaceutical Facility

This is a genetic algorithm (GA) based optimisation approach for medium-term capacity planning of a multi-product, multi-suite biopharmaceutical facility using a continuous-time representation. The continuous-time model is implemented by utilising a dynamic chromosome structure capable of adapting to the problem by growing in length from a single gene corresponding to a production campaign in a manufacturing schedule.

This software has been presented during a keynote lecture at the 27th European Symposium on Computer Aided Process Engineering (ESCAPE) and used in the following publication:

> Jankauskas, K., Papageorgiou, L. G., & Farid, S. S. (2017). Continuous-Time Heuristic Model for Medium-Term Capacity Planning of a Multi-Suite, Multi-Product Biopharmaceutical Facility. In *Computer Aided Chemical Engineering* (Vol. 40, pp. 1303-1308). Elsevier. **DOI:** [10.1016/B978-0-444-63965-3.50219-1](https://doi.org/10.1016/B978-0-444-63965-3.50219-1).

* [Setup](#setup)
    * [macOS](#macos)
    * [Ubuntu 16.04 LTS](#ubuntu)
* [Running the demo](#demo)

<a id='setup'></a>
## Setup 

<a id='macos'></a>
### MacOS

* Install [`brew`](https://brew.sh/)

* Install the necessary build tools
    ```
    brew update && brew install coreutils && brew install gcc --without-multilib
    ```
* Install [`anaconda`](https://www.anaconda.com/download/#linux)
* Create and activate virtual Python environment
    ```
    conda create -n <environment-name> python=3.5
    source activate <environement-name>
    ```
* Install Python libraries
    ```
    python -m pip install pip==10.0.1
    pip install setuptools==38.6.0 Cython==0.26 jupyter
    ```
* Find the path to the `g++` binary with `brew ls gcc`. It should be in       
    ```
    /usr/local/Cellar/gcc/<version>/bin/g++-<version>
    ```
* Export the path to the `g++` binary 
    ```
    export CC=<path to g++ binary> && export CXX=<path to g++ binary>
    ```
* Compile and install the `biopharma-scheduling`
    ```
    git clone https://github.com/karolisjan/BiopharmaScheduling.git
    cd BiopharmaScheduling
    python setup.py
    pip install dist/*whl
    ```

[back to top](#index)

<a id='ubuntu'></a>
### Ubuntu 16.04 LTS

* Install the essentials first
    ```
    sudo apt-get update 
    sudo apt-get install build-essential software-properties-common -y 
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y 
    sudo apt-get update && sudo apt-get install gcc-snapshot -y 
    sudo apt-get update && sudo apt-get install gcc-7 g++-7 -y
    sudo apt-get install git python-dev python3-dev python-pip python3-pip python-wheel python3-wheel python-virtualenv 
    ```
* Create and activate virtual Python environment
    ```
    virtualenv -p python3 ~/<environment-name>
    echo "alias <environment-name>='source ~/<environment-name>/bin/activate'" >> ~/.bash_aliases
    source ~/.bash_aliases
    <environment-name>
    ```
* Install Python libraries
    ```
    python -m pip install pip==10.0.1
    pip install setuptools==38.6.0 Cython==0.26 jupyter
    ```
* Export the path to the `g++` binary 
    ```
    export CC=g++-7 && export CXX=g++-7
    ```
* Compile and install the `biopharma-scheduling`
    ```
    git clone https://github.com/karolisjan/BiopharmaScheduling.git
    cd BiopharmaScheduling
    python setup.py
    pip install dist/*whl
    ```

[back to top](#index)

<a id='demo'></a>
## Running the demo

* Install `jupyter` with `pip install jupyter`
* Setup the `ipykernel` for the environment created earlier
    ```
    pip install ipykernel
    python -m ipykernel install --user --name <environment-name> --display-name "<display-name>"
    ```
* Launch `jupyter notebook` and navigate to `ContinuousTimeCapacityPlanning/demo`

[back to top](#index)

