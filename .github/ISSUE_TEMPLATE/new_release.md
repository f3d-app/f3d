---
name: New release
about: An issue to create a new release
title: ''
labels: release
assignees: ''

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
- [ ] Locally test the prerelease on Linux: run, render, raytracing, config file, drag&drop, thumbnails, exec from file manager, sdk, python wheel
- [ ] Locally test the prerelease on macOS (Intel): run, render, raytracing, config file, drag&drop, exec from file manager, python wheel
- [ ] Locally test the prerelease on macOS (Silicon): run, render, config file, drag&drop, exec from file manager
- [ ] Locally test the prerelease on Windows: run, render, raytracing, config file, drag&drop, thumbnails, exec from file manager, sdk, python wheel
- [ ] Download all python wheels and upload them to https://test.pypi.org/project/f3d/, check `pip install f3d`
- [ ] If it fails, fix the issues in `release` branch, copy this paragraph and increment `N`
- [ ] Else proceed to the next part

Release :

- [ ] Commit, review and merge `vX.Y.Z` version change in https://github.com/f3d-app/f3d `CMakeLists.txt` and `pyproject.toml` in the `release` branch
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d: `git tag vX.Y.Z -m vX.Y.Z`
- [ ] Commit, review and merge adding `X.Y.Z` in https://github.com/f3d-app/f3d-superbuild `versions.cmake` in the `main` branch
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d-superbuild: `git tag vX.Y.Z -m vX.Y.Z`
- [ ] Merge F3D release into master: https://github.com/f3d-app/f3d/compare/master...release
- [ ] Trigger a release build using https://github.com/f3d-app/f3d-superbuild actions with `vX.Y.Z` F3D version, `vX.Y.Z` sb version and prerelease publish true
- [ ] Finalize release note and add them to the release
- [ ] Download all python wheels and upload them to https://pypi.org/project/f3d/
- [ ] Release
- [ ] Update download links and update changelog in https://github.com/f3d-app/f3d `doc`
- [ ] Update `.github/ISSUE_TEMPLATE/new_release.md` in https://github.com/f3d-app/f3d if needed
- [ ] Communicate on discord
- [ ] Communicate on reddit
- [ ] Communicate on hackernews
- [ ] Communicate on linkedin
