<a id='index'></a>
# Biopharma Scheduling

> Work in progress...

* [Introduction](#intro)
* [Setup](#setup)
    * [Docker](#docker)
    * [macOS](#macos)
    * [Ubuntu 16.04 LTS](#ubuntu)
* [Examples](#demo)

<a id='intro'></a>
## Introduction

This is a genetic algorithm (GA) based optimisation approach for medium-term capacity planning and scheduling of multi-product biopharmaceutical facilities using a continuous-time representation. The continuous-time model is implemented by utilising a variable-length chromosome structure capable of adapting to the problem by growing in length from a single gene corresponding to a production campaign in a manufacturing schedule.

This approach has been presented during a keynote lecture at the 27th European Symposium on Computer Aided Process Engineering (ESCAPE):

> Jankauskas, K., Papageorgiou, L. G., & Farid, S. S. (2017). Continuous-Time Heuristic Model for Medium-Term Capacity Planning of a Multi-Suite, Multi-Product Biopharmaceutical Facility. In *Computer Aided Chemical Engineering* (Vol. 40, pp. 1303-1308). Elsevier. **DOI:** [10.1016/B978-0-444-63965-3.50219-1](https://doi.org/10.1016/B978-0-444-63965-3.50219-1).

<a id='setup'></a>
## Setup 

<a id='docker'></a>
### Docker

* Download and install [docker](https://www.docker.com/community-edition) >= `docker version 17.12.0`
* On Windows 10, [switch to using Linux containers](https://docs.microsoft.com/en-us/virtualization/windowscontainers/quick-start/quick-start-windows-10)
* Run the following in the terminal
    ```
    git clone https://github.com/UCL-Biochemical-Engineering/BiopharmaScheduling
    cd BiopharmaScheduling
    docker build -t biopharma-scheduling/base -f ./docker/base.docker .
    docker build -t biopharma-scheduling/lab -f ./docker/lab.docker .
    ```

<a id='macos'></a>
### macOS

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
* Find the path to the `g++` binary with `brew ls gcc | grep g++`. It should be in       
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
    sudo apt-get update && sudo apt-get install build-essential software-properties-common -y 
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y 
    sudo apt-get update && sudo apt-get install gcc-snapshot -y 
    sudo apt-get update && sudo apt-get install gcc-8 g++-8 -y
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
    export CC=g++-8 && export CXX=g++-8
    ```
* Compile and install the `biopharma-scheduling`
    ```
    git clone https://github.com/UCL-Biochemical-Engineering/BiopharmaScheduling
    cd BiopharmaScheduling
    python setup.py
    pip install dist/*whl
    ```

[back to top](#index)

<a id='examples'></a>
## Examples

* Using `docker`:
    ```
    docker run -it -p 8888:8888 -v <absolute path to BiopharmaScheduling folder>:/BiopharmaScheduling biopharma-scheduling/lab bash -c "jupyter lab --ip 0.0.0.0 --no-browser --allow-root"
    ```
    * Go to `localhost::8888/?token=<token ID>`and navigate to `examples` folder
* Using [Jupyter Lab](https://blog.jupyter.org/jupyterlab-is-ready-for-users-5a6f039b8906) 

    * Setup the `ipykernel` for the environment created earlier
        ```
        pip install ipykernel
        python -m ipykernel install --user --name <environment-name> --display-name "<display-name>"
        ```
    * Create and activate a separate Python enviroment, and run `pip install jupyter jupyterlab`
    * Install [Node.js](https://nodejs.org/en/)
    * Set-up [Plotly extension](https://github.com/jupyterlab/jupyter-renderers/tree/master/packages/plotly-extension) with `jupyter labextension install @jupyterlab/plotly-extension`
    * Launch `jupyter lab` and navigate to `examples` folder

[back to top](#index)

