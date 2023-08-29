import sys
import os


def pytest_addoption(parser):
    parser.addoption("--cmake-binary-dir")
