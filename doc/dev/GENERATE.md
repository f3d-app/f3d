# How to generate the full coverage report

Requires `gcovr` program and `gcc` toolchain.

1. Build with `F3D_COVERAGE` option enabled.
2. Run all tests.
3. Generate the report with: `gcovr -r /path/to/sources --html --html-details -o coverage.html`.

# How to build and test with sanitizer

Requires `clang` toolchain.

1. Build with `F3D_SANITIZER` option to any of the possible values.
2. `export LSAN_OPTIONS=suppressions=/path/to/f3d/.lsan.supp:use_tls=0`.
3. `export TSAN_OPTIONS=suppressions=/path/to/f3d/.tsan.supp`.
4. Run all tests.

# How to locally generate the jekyll based website

1. Install `ruby` and make sure ruby binaries directory is added to your `PATH`
2. Install jekyll and all dependencies: `gem install jekyll jekyll-remote-theme jekyll-relative-links jekyll-seo-tag jekyll-optional-front-matter jekyll-titles-from-headings`
3. Run jekyll locally: `jekyll server`

Please note the favicon and search bar are not working locally, this is expected.
