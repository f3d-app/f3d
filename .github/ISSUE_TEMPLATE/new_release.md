---
name: New release
about: An issue to create a new release
title: ""
labels: release
assignees: ""
---

When creating this issue

- Edit `vX.Y.Z-RCN` to the right version
- Edit `vX.Y.Z` to the right version

Before release:

- [ ] Force fetch origin remote tag with `git fetch origin --tags --force`
- [ ] Write and format release notes from the ongoing changelog, including contributors and packagers

Release Split :

- [ ] Create, review and merge a MR from `master` branch into `release`: https://github.com/f3d-app/f3d/compare/release...master

Release Candidates :

- [ ] Commit, review and merge a `vX.Y.Z-RCN` version change in https://github.com/f3d-app/f3d `CMakeLists.txt` and `pyproject.toml` in the `release` branch
- [ ] Tag and push to https://github.com/f3d-app/f3d: `git tag vX.Y.Z-RCN -m vX.Y.Z-RCN`
- [ ] Trigger a release build using https://github.com/f3d-app/f3d-superbuild actions with `vX.Y.Z-RCN` F3D version, superbuild `main` branch and prerelease publish `true`
- [ ] Download the prerelease from https://github.com/f3d-app/f3d/releases
- [ ] Locally test the prerelease on Linux
- [ ] Locally test the prerelease on macOS (Intel)
- [ ] Locally test the prerelease on macOS (Silicon)
- [ ] Locally test the prerelease on Windows
- [ ] Locally test the python wheels on Linux
- [ ] Locally test the python wheels on macOS (Silicon)
- [ ] Locally test the python wheels on Windows
- If it fails,
  - [ ] Fix the issues in `release` branch,
  - [ ] Edit this issuethis paragraph and increment `N`
  - [ ] Merge F3D release into master: https://github.com/f3d-app/f3d/compare/master...release
- [ ] Else proceed to the next part

Release :

- [ ] Commit, review and merge `vX.Y.Z` version change in https://github.com/f3d-app/f3d `CMakeLists.txt`, `pyproject.toml`, `webassembly/package.json` and the multiple `doxygen\conf.py` in the `release` branch
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d: `git tag vX.Y.Z -m vX.Y.Z`
- [ ] Commit, review and merge adding `X.Y.Z` in https://github.com/f3d-app/f3d-superbuild `versions.cmake` in the `main` branch
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d-superbuild: `git tag vX.Y.Z -m vX.Y.Z`
- [ ] Merge F3D release into master: https://github.com/f3d-app/f3d/compare/master...release
- [ ] Trigger a release build using https://github.com/f3d-app/f3d-superbuild actions with `vX.Y.Z` F3D version, `vX.Y.Z` sb version and prerelease publish true
- [ ] Finalize release note and add them to the release
- [ ] Release
- [ ] Update download links and update changelog in https://github.com/f3d-app/f3d `doc`
- [ ] Communicate on discord
- [ ] Communicate on reddit
- [ ] Communicate on hackernews
- [ ] Communicate on linkedin
- [ ] Communicate on mastodon

- [ ] Merge F3D release into master: https://github.com/f3d-app/f3d/compare/master...release- [ ] Communicate on bluesky
- [ ] Move all issue from current milestone to next milestone, close current roadmap issue and open a next roadmap issue
- [ ] Update `doc/dev/ROADMAPS_AND_RELEASES.md` for next release
- [ ] Create an issue for updating dependencies in CI and superbuild
- [ ] Update `.github/ISSUE_TEMPLATE/new_release.md` in https://github.com/f3d-app/f3d if needed

Linux testing protocol:

- Install F3D-X.Y.Z-RCN-Linux-x86_64-raytracing system wide
- `f3d testing/data/cow.vtp` -> check render, orientation, press R and check raytracing
- `f3d testing/data/f3d.stp` -> check render, orientation
- `f3d testing/data/f3d.glb` -> press "Space", check animation is going smoothly and takes 4 seconds
- `f3d /path/to/any/new/file.format` -> check render
- `rm -rf ~/.cache/thumbnails`
- `pcmanfm` (or another supported file manager)
- Check that all supported files in testing/data have a generated thumbnails, especially for new file formats if any
- Double click on supported file in testing/data, especially for new file formats if any and check it opens in F3D
- Drag&Drop cow.vtp, Drag&Drop palermo_park.hdr, check render
- Check that CTRL+O (file dialog) is working
- Press "Esc" and check the following commands `reload_current_file_group`, `set_camera top`, `toggle_volume_rendering`, `exit`
- `cd examples/libf3d && mkdir build && cd build && cmake ../ && make`
- `./cpp/check-engine/check-engine`
- `./cpp/render-interact/render-interact ../../../testing/data/cow.vtp`

macOS testing protocol:

- Install F3D-X.Y.Z-RCN-macOS-x86_64-raytracing.dmg
- Double click on cow.vtp, check render and orientation, press R and check raytracing
- Double click on f3d.stp, check render and orientation
- Double click on f3d.glb, press "Space", check animation is going smoothly and takes 4 seconds
- Double click on any new supported file.format, check render
- Check all supported file in testing/data directory in finder have the f3d mark, if not, check that "open with" suggest F3D
- Drag&Drop cow.vtp, Drag&Drop palermo_park.hdr, check render
- Check that CTRL+O (file dialog) is working
- Press "Esc" and check the following commands `reload_current_file_group`, `set_camera top`, `toggle_volume_rendering`, `exit`

Windows testing protocol:

- Install F3D-X.Y.Z-RCN-Windows-x86_64-raytracing.exe, add F3D to path for all users
- Start -> Disk Cleanup -> select thumbnails -> Ok -> Delete Files
- Open testing/data directory in file manager
- Wait for thumbnails to be generated
- Check that every supported file has a generated thumbnails, except .tif.
- Check that every supported file has a small "f3d" mark, for those that don't check that "Open with" -> "Use another app" suggest F3D.
- Double click on cow.vtp, check render and orientation, press R and check raytracing
- Double click on f3d.stp, check render and orientation
- Double click on f3d.glb, press "Space", check animation is going smoothly and takes 4 seconds
- Double click on any new supported file format, check render
- Check that CTRL+O (file dialog) is working
- Press "Esc" and check the following commands `reload_current_file_group`, `set_camera top`, `toggle_volume_rendering`, `exit`
- `cd examples\libf3d && mkdir build && cd build && cmake ../ && cmake --build . --config Release`
- `.\cpp\check-engine\Release\check-engine`
- `.\cpp\render-interact\Release\render-interact ..\..\..\testing\data\cow.vtp`

Python testing protocol:

- Use a venv or make sure f3d is not installed in your python environment
- pip install f3d==X.Y.Z-RCN
- python

```
import f3d
eng = f3d.Engine.create()
eng.scene.add("/path/to/cow.vtp")
eng.window.render() # No effect on Windows
eng.interactor.start()
```

- Check the following
  - The name of the window is `f3d` at all time
  - Python provides suggestions whenever the "Tab" key is pressed
