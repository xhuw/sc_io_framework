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
    cd sc_io_framework/app_sc_framework/

Now make the project:

    cmake -G "Unix Makefiles" -B build
    make -C build/ -j


Setup the hardware
------------------

This demo runs on XK-AUDIO-316-MC board with a few modifications::

    (TODO)

     - Jumpers on ADC
     - Neopixel on MIDI out
     - I2C connected to IO expander
     - Potentiometer with series R and parallel C connected to ....

 
 Modifying the DSP
 -----------------

 The Jupyter notebook containing the design can be found at ``sc_io_framework/app_sc_framework/dsp_design.ipynb``.


See https://github.com/xmos/lib_audio_dsp/tree/develop/doc/programming_guide for more details on lib_audio_dsp operation.
