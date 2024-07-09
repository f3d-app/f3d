# How to generate the full coverage report

Requires `gcovr` program and `gcc` toolchain.

1. Build with `F3D_COVERAGE` option enabled.
2. Run all tests, `xdotool` needs to be installed. This will generate many `.gcda` files.
3. Generate the report with: `gcovr -r /path/to/sources --html --html-details -o coverage.html`.

# How to build and test with sanitizer

Requires `clang` toolchain.

1. Build with `F3D_SANITIZER` option to any of the possible values.
2. `export LSAN_OPTIONS=suppressions=/path/to/f3d/.lsan.supp:use_tls=0`.
3. `export TSAN_OPTIONS=suppressions=/path/to/f3d/.tsan.supp`.
4. Run all tests.

# How to locally generate the Jekyll based website

1. Install `ruby` and make sure ruby binaries directory is added to your `PATH`
2. Install Jekyll and all dependencies: `gem install jekyll jekyll-remote-theme jekyll-relative-links jekyll-seo-tag jekyll-optional-front-matter jekyll-titles-from-headings jekyll-include-cache`
3. Run Jekyll locally: `jekyll server`
4. Open http://127.0.0.1:4000/ in a browser

Please note the favicon and search bar are not working locally, this is expected.

# How to locally generate the doxygen documentation

1. Install Python, pip and doxygen
2. `pip install sphinx sphinx-book-theme exhale`
3. `sphinx-build -M html ./library/doxygen /path/to/build`
4. `/path/to/build` directory now contains a HTML website
