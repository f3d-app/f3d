import sys
import os


def pytest_sessionstart(session):
    if sys.platform.startswith("win32"):
        os.add_dll_directory(session.config.getoption("vtk_common"))


def pytest_addoption(parser):
    parser.addoption("--vtk-common")
    parser.addoption("--f3d-source-dir")
    parser.addoption("--cmake-binary-dir")
