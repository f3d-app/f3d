import sys
import os


def pytest_sessionstart(session):
    if sys.platform.startswith("win32"):
        paths = session.config.getoption("windows_runtime_paths", default="")
        for path in paths.split(";"):
            os.add_dll_directory(path)


def pytest_addoption(parser):
    parser.addoption("--windows-runtime-paths")
    parser.addoption("--f3d-source-dir")
    parser.addoption("--cmake-binary-dir")
