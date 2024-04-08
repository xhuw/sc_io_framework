# sc_io_framework


Application framework and DSP pipeline for the soundcard project.


Installation
------------

Make sure you have the XTC tools version 15.2.1 installed and sourced along with xcommon cmake. https://github.com/xmos/xcommon_cmake/blob/develop/doc/quick_start.rst


Install the app and its dependancies:

    mkdir mysandbox
    cd mysandbox/
    git clone git@github.com:ed-xmos/sc_io_framework.git
    cd sc_io_framework/
    cd app_sc_framework/
    cmake -G "Unix Makefiles" -B build

Create a virtual environment in the sandbox and install lib_audio_dsp required for the autogeneration of files step:

    cd ../..
    python -m venv .venv
    . .venv/bin/activate
    pip install -e lib_audio_dsp/python

Note you will need to activate the environment each time you start a new session. There are scripts in lib_audio_dsp which are called during the firmware build process. If you *do not* do this you will see the message ``Excluding lib_audio_dsp stages as audio_dsp python package not available`` in the cmake stage.

Now make the project:

    cd sc_io_framework/app_sc_framework/
    cmake -G "Unix Makefiles" -B build
    make -C build/ -j


Setup the hardware
------------------

This demo runs on XK-AUDIO-316-MC board with a few modifications. Please see this page https://xmosjira.atlassian.net/wiki/spaces/~870418189/pages/4048945164/FRJ+Demo+HW+Setup+Guide for details.


Running the demo
----------------

Use the standard 
 
Modifying the DSP
-----------------

 The Jupyter notebook containing the design can be found at ``sc_io_framework/app_sc_framework/dsp_design.ipynb``.


See https://github.com/xmos/lib_audio_dsp/tree/develop/doc/programming_guide for more details on lib_audio_dsp operation.
