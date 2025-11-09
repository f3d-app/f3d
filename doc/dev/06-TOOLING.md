# Tooling

## How to generate the full coverage report

Requires `gcovr` program and `gcc` toolchain.

1. Build with `F3D_COVERAGE` option enabled.
2. Run all tests, `xdotool` needs to be installed. This will generate many `.gcda` files.
3. Generate the report with: `gcovr -r /path/to/sources --html --html-details -o coverage.html`.

## How to build and test with sanitizer

Requires `clang` toolchain.

1. Build with `F3D_SANITIZER` option to any of the possible values.
2. `export LSAN_OPTIONS=suppressions=/path/to/f3d/.lsan.supp:use_tls=0`.
3. `export TSAN_OPTIONS=suppressions=/path/to/f3d/.tsan.supp`.
4. Run all tests.

## How to locally generate and run the website

1. Install `npm`
2. Clone https://github.com/f3d-app/f3d-website
3. Install needed npm packages: `npm install`
4. Build and serve the website: `npm run start`

Please note the search bar is not working locally, this is expected.

## How to update the doc using latest master

1. Generate and run as described above
2. Select master: `export F3D_TAG=master`
2. Update the doc: `npm run update-doc`
3. Refresh the website to see the new doc

## How to update the doc for a new release

1. Generate and run as described above
2. Select release: `export F3D_TAG=release`
2. Update the doc: `npm run update-doc`
3. Add a new versioned doc: `npm run docusaurus docs:version X.Y`
4. Add `X.Y` in the `docsVersionDropdown` in `docusaurus.config.ts`
5. Rebuild the website: `npm run start`
6. Refresh the website to see the new versionned doc
