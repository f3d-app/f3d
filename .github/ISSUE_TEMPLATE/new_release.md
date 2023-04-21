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
- Edit `target` branch to `master` if Z is zero, or `release` if not
- Edit `other` branch to `release` if Z is zero, or `master` if not
- Force fetch origin remote tag with `git fetch origin --tags --force`

Release Candidates :

- [ ] Commit, review and merge a `vX.Y.Z-RCN` version change in https://github.com/f3d-app/f3d `CMakeLists.txt` in the `target` branch
- [ ] Tag `vX.Y.Z-RCN` and push it to https://github.com/f3d-app/f3d
- [ ] Trigger a release build using https://github.com/f3d-app/f3d-superbuild actions with `vX.Y.Z-RCN` F3D version, superbuild `main` branch and prerelease publish `true`
- [ ] Download the `vX.Y.Z-RCN` prerelease from https://github.com/f3d-app/f3d/releases
- [ ] Locally test the `vX.Y.Z-RCN` prerelease on Linux
- [ ] Locally test the `vX.Y.Z-RCN` prerelease on macOS
- [ ] Locally test the `vX.Y.Z-RCN` prerelease on Windows
- [ ] If it fails, fix the issue, copy this paragraph and increment `N`
- [ ] Else proceed to the next part

Release :

- [ ] Commit, review and merge `vX.Y.Z` version change in https://github.com/f3d-app/f3d `CMakeLists.txt`
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d
- [ ] Merge https://github.com/f3d-app/f3d `target` branch into the `other` branch
- [ ] Commit, review and merge adding `X.Y.Z` in https://github.com/f3d-app/f3d-superbuild `versions.cmake`
- [ ] Tag `vX.Y.Z` and push it to https://github.com/f3d-app/f3d-superbuild
- [ ] Trigger a release build using https://github.com/f3d-app/f3d-superbuild actions with `vX.Y.Z` F3D version, `vX.Y.Z` sb version and prerelease publish true
- [ ] Write release notes using PR since last release and add it to the release
- [ ] Add contributors and packagers to the release notes
- [ ] Release
- [ ] Communicate on discord
- [ ] Communicate on reddit
- [ ] Communicate on hackernews
- [ ] Communicate on linkedin
