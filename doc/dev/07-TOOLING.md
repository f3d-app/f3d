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

## How to generate the doc for latest master

1. Generate and run as described above
2. Update the doc: `npm run update-doc`
3. Refresh the website to see the new doc

## How to update the doc for a new release

1. Generate and run as described above
2. Update the doc for release branch: `npm run update-doc release`
3. Add a new versioned doc: `npm run docusaurus docs:version X.Y`
4. Add `X.Y` in the `docsVersionDropdown` in `docusaurus.config.ts`
5. Rebuild the website: `npm run start`
6. Refresh the website to see the new versioned doc

## How to run the XR mode

1. VTK should already be built with the `F3D_MODULE_OPENXR` option enabled. See [Build](./05-BUILD.md) for more information.

2. Set up an OpenXR runtime. Choose one of the following:
   - **Physical HMD:** Attach an HMD and start SteamVR.
   - **Monado:** Install Monado (installation from source is recommended: [Monado installation guide](https://monado.freedesktop.org/getting-started.html#installation-from-source)) and use the following script to start it in simulation mode:

   ```bash
   #!/bin/bash
   # Set Monado as the active OpenXR runtime
   rm ~/.config/openxr/1/active_runtime.json
   ln -s /usr/share/openxr/1/openxr_monado.json ~/.config/openxr/1/active_runtime.json

   # Enable Monado simulation
   export XR_RUNTIME_JSON=/usr/share/openxr/1/openxr_monado.json
   export QWERTY_ENABLE=1
   export SIMULATED_ENABLE=1
   export XRT_DEBUG_GUI=1
   export XRT_COMPOSITOR_FORCE_XCB=1

   # Start the service
   monado-service
   ```

   In simulation mode, use the right mouse button to move around or adjust the HMD's position and orientation in the Monado UI.

3. Run F3D: `f3d ../testing/data/<data>.<datatype> --rendering-backend=xr`
