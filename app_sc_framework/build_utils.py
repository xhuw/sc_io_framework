# Copyright 2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
"""
Utility functions for building and running the application within
the jupyter notebook
"""
import subprocess
import ipywidgets as widgets
from IPython import display
from pathlib import Path
import shutil
import time

class _Globals:
    CONFIGURE_DONE = False

APP_DIR = Path(__file__).parent
TARGET = "sc_framework"
XE = APP_DIR / "bin" / f"{TARGET}.xe"

LOG_STR = """
<pre style="font-family: monospace, monospace;">{output}</pre>
"""

def log(process, title=""):
    widget = widgets.HTML(value="")
    accordian = widgets.Accordion(children=[widget])
    accordian.set_title(0, title)
    display.display(accordian)
    output = ""
    for line in process.stdout:
        output += line
        widget.value = LOG_STR.format(output=output)
    process.wait()
    if process.returncode:
        accordian.set_title(0, title + "  Failed ❌ (click for details)")
    else:
        accordian.set_title(0, title + "  ✔")

def build_and_try_run():
    """
    Attempt to build and xrun the application
    """
    build_dir = APP_DIR / "build"
    source_dir = APP_DIR
    cache = build_dir / "CMakeCache.txt"
    makefile = build_dir / "Makefile"
    ninjabuild = build_dir / "build.ninja"
    if (not _Globals.CONFIGURE_DONE) or (not cache.exists()) or not (makefile.exists() or ninjabuild.exists()):
        if cache.exists():
            # Generator is already known by cmake
            ret = subprocess.Popen([*(f"cmake -S {source_dir} -B {build_dir}".split())], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        else:
            # need to configure, default to Ninja because its better
            generator = "Ninja" if shutil.which("ninja") else "Unix Makefiles"
            ret = subprocess.Popen([*(f"cmake -S {source_dir} -B {build_dir} -G".split()), generator], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        log(ret, "Configuring...")
        if ret.returncode:
            return
        _Globals.CONFIGURE_DONE = True

    ret = subprocess.Popen(f"cmake --build {build_dir} --target {TARGET}".split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    log(ret, "Compiling...")
    if ret.returncode:
        return

    ret = subprocess.Popen(f"xrun {XE}".split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    log(ret, "Running...")
    if ret.returncode:
        return
    time.sleep(5) # give some time for the device to enumerate
    print("Done\r")
