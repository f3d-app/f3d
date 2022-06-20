import pathlib

from skbuild import setup


def exclude_static_libraries(cmake_manifest):
    return list(filter(lambda name: not (name.endswith(".a") and not name.endswith(".lib")), cmake_manifest))

here = pathlib.Path(__file__).parent.resolve()
long_description = (here / "README.md").read_text(encoding="utf-8")

setup(
    name="f3d",
    description="F3D, a fast and minimalist 3D viewer",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://f3d-app.github.io/f3d/",
    project_urls={
        "Source Code": "https://github.com/f3d-app/f3d",
        "Bug Tracker": "https://github.com/f3d-app/f3d/issues",
        "Documentation": "https://f3d-app.github.io/f3d/docs/",
    },
    author="Michael Migliore and Mathieu Westphal",
    author_email="",  # TODO
    license="BSD 3-Clause",
    license_files=["LICENSE", "THIRD-PARTY-LICENSES.md"],
    classifiers=[  # TODO
        "License :: OSI Approved :: BSD License",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX",
        "Operating System :: MacOS",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
    ],
    keywords="",  # TODO
    python_requires=">=3.7",
    packages=["f3d"],
    package_dir={
        "f3d": "python/packaging",
    },
    install_requires=[],
    entry_points={"console_scripts": ["f3d=f3d.__main__:main"]},
    cmake_source_dir=".",
    cmake_install_dir="python/packaging",
    cmake_args=[  # TODO: enable/disable modules with env variables?
        "-DF3D_PYTHON_BINDINGS:BOOL=ON",
        "-DBUILD_SHARED_LIBS:BOOL=OFF",
        "-DF3D_MODULE_OCCT:BOOL=ON",
        "-DF3D_MODULE_ASSIMP:BOOL=ON",
        "-DF3D_MODULE_ALEMBIC:BOOL=ON",
    ],
    cmake_process_manifest_hook=exclude_static_libraries,
    zip_safe=False,
)
