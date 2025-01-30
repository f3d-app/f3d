# How to Contribute

F3D welcomes all contributors, regardless of skill level or experience!

## Contributing as a User

Contributing to F3D can be as simple as pointing out a spelling mistake on the website,
reporting a bug you encountered, or suggesting a new feature you feel would improve the application.

We use [GitHub](https://github.com/f3d-app/f3d) to manage the project and you can report new issues or weigh
in on existing ones via our [issue tracker](https://github.com/f3d-app/f3d/issues).

Also, do not hesitate to join our [Discord](https://discord.f3d.app)!

## How to Get Started With Development

To contribute to F3D as a developer, first you may want to try and build F3D for yourself.
If you are already familiar with software compilation, you can take a look at our [build guide](doc/dev/BUILD.md).
If not, you may want to look at our [getting started guide](doc/dev/GETTING_STARTED.md), that provide steps to compile F3D.

Once you are able to build F3D, you may want to take a look at the open [issues](https://github.com/f3d-app/f3d/issues)!
If you are a beginner, you may want to look at ["good first issues"](https://github.com/f3d-app/f3d/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22).
If you already have some experience with programming and contribution, definitely look at ["help wanted"](https://github.com/f3d-app/f3d/issues?q=is%3Aopen+is%3Aissue+label%3A%22help+wanted%22) issues.
If an issue is interesting to you and is not yet assigned, then you should *comment on the issue* and ask for any help or clarification needed.
F3D maintainers will see your comment, assign the issue to you and provide guidance as needed.

To get involved more deeply, please take a look at the [roadmaps](doc/dev/ROADMAPS_AND_RELEASES.md) for the upcoming releases.

It is also recommended to reach out on [Discord](https://discord.f3d.app) to simplify communication, but it is not required.

You can then fix the issue or implement the feature on your side and contribute it to the F3D repository by following the workflow described below.

Of course, if you are already using F3D and want to improve it for your specific needs, because you want a feature or found a bug,
that is definitely possible. Feel free to reach out for guidance by opening an issue or asking on [Discord](https://discord.f3d.app).

Another way to get started is to improve the documentation, look for typos, or improve the examples and the gallery. Please share any ideas you may have!

## F3D Development Workflow

F3D uses [GitLab Flow](https://about.gitlab.com/topics/version-control/what-is-gitlab-flow/). In a few words, here is how to contribute:
- [Fork](https://github.com/f3d-app/f3d/fork) the F3D repository on GitHub.
- Create and push a feature branch on your fork containing new commits.
- When it is ready for review or when you want to run the CI, create a pull request against `f3d-app/f3d/master`.
- Once the PR is approved and CI comes back clean, a F3D maintainer will merge your pull request in the master branch.
- The master now contains your changes and will be present in the next release!

## Continuous Integration

F3D has pretty extensive continuous integration trying to cover all usecases for F3D.
It means that if the change in your pull request breaks continuous integration, it will not be merged until it passes successfully.
It also means that adding a new feature or behavior means adding an associated test.
Make sure to check the results for yourself and ask for help if needed.

F3D continuous integration will also check the coverage as it is a good way to evaluate if new features are being tested or not.
When adding code to F3D, always try to cover it by adding/modifying [tests](doc/dev/TESTING.md).

F3D continuous integration also checks formatting using clang-format and will inform you if changes needs to be made.
Some [formatting rules](doc/dev/CODING_STYLE.md) are not enforced by clang-format and will be checked during the review process.

When making changes to the `libf3d` public API, continuous integration will warn about making related changes to the bindings.
This is required in order to merge the pull request.

## Changelog

Whenever you make significant changes, either bugfixes or features, please update the [changelog](doc/CHANGELOG.md) "Ongoing development" section.
There are three subsections. "For F3D Users" are changes that impact actual F3D users and are related to F3D behaviors changing. "For developers" are changes
that impact developers that build F3D. "For F3D packagers" are changes that impact anyone packaging F3D, eg: new CMake options.

## F3D Architecture

F3D is divided into different components:
- The `f3d` executable, in the `application` folder.
- The `libf3d` library, in the `library` folder.
- The extensions to VTK, in the `vtkext` folder.
- The bindings for Python, Java, and WebAssembly, in their respective folders.
- The plugins, providing all the different readers in the plugins directory.

The extensions to VTK are divided into two modules:
- The [private module](vtkext/private) is used exclusively by `libf3d`.
- The [public module](vtkext/public) is used by `libf3d` and also by plugins. Additional documentation can be found in [PLUGINS.md](doc/libf3d/PLUGINS.md).

The `libf3d` library implements the whole logic of instancing and manipulating the different VTK classes, it is fully documented in the [libf3d README](doc/libf3d/README_LIBF3D.md).

The `f3d` executable itself uses `libf3d` and adds an application layer on top of it, especially the handling of [command line options](doc/user/OPTIONS.md)
and a [configuration file](doc/user/CONFIGURATION_FILE.md).
