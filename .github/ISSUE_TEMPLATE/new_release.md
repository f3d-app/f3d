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
- [ ] Write and format release notes

Release Split :

- [ ] Create, review and merge a MR from `master` branch into `release` branch in https://github.com/f3d-app/f3d

Release Candidates :

- [ ] Commit, review and merge a `vX.Y.Z-RCN` version change in https://github.com/f3d-app/f3d `CMakeLists.txt` in the `release` branch
- [ ] Add a Tag and push it to https://github.com/f3d-app/f3d: `git tag vX.Y.Z-RCN -m vX.Y.Z-RCN`
- [ ] Trigger a release build using https://github.com/f3d-app/f3d-superbuild actions with `vX.Y.Z-RCN` F3D version, superbuild `main` branch and prerelease publish `true`
- [ ] Download the prerelease from https://github.com/f3d-app/f3d/releases
- [ ] Locally test the prerelease on Linux: run, render, config file, drag&drop, thumbnails, exec from file manager, sdk
- [ ] Locally test the prerelease on macOS: run, render, config file, drag&drop, exec from file manager
- [ ] Locally test the prerelease on Windows: run, render, config file, drag&drop, thumbnails, exec from file manager, sdk
- [ ] If it fails, fix the issues in `release` branch, backport `release` branch to `master` branch, copy this paragraph and increment `N`
- [ ] Else, backport `release` branch to `master` branch and proceed to the next part

Release :

- [ ] Commit, review and merge `vX.Y.Z` version change in https://github.com/f3d-app/f3d `CMakeLists.txt`
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d: `git tag vX.Y.Z -m vX.Y.Z`
- [ ] Commit, review and merge adding `X.Y.Z` in https://github.com/f3d-app/f3d-superbuild `versions.cmake`
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d-superbuild: `git tag vX.Y.Z -m vX.Y.Z`
- [ ] Trigger a release build using https://github.com/f3d-app/f3d-superbuild actions with `vX.Y.Z` F3D version, `vX.Y.Z` sb version and prerelease publish true
- [ ] Finalize release note and add them to the release
- [ ] Add contributors and packagers to the release notes
- [ ] Release
- [ ] Update download links and update changelog in https://github.com/f3d-app/f3d `doc`
- [ ] Communicate on discord
- [ ] Communicate on reddit
- [ ] Communicate on hackernews
- [ ] Communicate on linkedin
