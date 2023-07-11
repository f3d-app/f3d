import sys
import os


def pytest_addoption(parser):
    parser.addoption("--f3d-source-dir")
    parser.addoption("--cmake-binary-dir")
