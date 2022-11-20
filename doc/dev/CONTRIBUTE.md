# How to contribute

F3D welcomes all contributors, regardless of skill level or experience!

## How to get started

To contribute to F3D, you may want to take a look at the opened [issues](https://github.com/f3d-app/f3d/issues),
especially, the ones with the ["good first issue"](https://github.com/f3d-app/f3d/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22) label.
If one sounds interesting to you, then you should just go ahead and comment on the issue and ask for any help or clarification needed.
F3D maintainers will see your comment and provide guidance as needed.
You can also open your own issue or feature request and F3D maintainers will try to help you with it!

You can then fix the issue in your side and contribute it to the F3D repository,
by following the workflow described below.

## F3D Development workflow

F3D uses the [gitlab flow](https://docs.gitlab.com/ee/topics/gitlab_flow.html). In a few words, here is how to contribute:
- [Fork](https://github.com/f3d-app/f3d/fork) F3D repository on github.
- Create and push a feature branch on your fork containing new commits.
- When it is ready for review or when you want to run the CI, create a pull request against `f3d-app/f3d/master`.
- Once the PR is approved and CI comes back clean, a F3D maintainer will merge your pull request in the master branch
- The master now contains your changes and will be present in the next release!

## Continuous Integration

F3D has a pretty extensive continuous integration trying to cover all usecases for F3D.
It means that if your change break the CI in your PR, it will not be merged until the breaking change are addressed.
It also means that adding a new feature or behavior means adding a associated test.
Make sure to check the results for yourself, and ask for help if needed.

F3D continuous integration will also check the coverage as it is a good way to evaluate if new features are being tested or not.
When adding code to F3D, always to to cover it by adding/modifying [tests](TESTING.md).

F3D continuous integration also check formatting using clang-format and will inform you if changes needs to be made.
However, some [formatting rules](CODING_STYLE.md) are not enforced by clang-format and will be checked during the review process.

## F3D architecture

F3D is separated in three main components:
- The F3D application, in the application folder.
- The libf3d, in the library folder.
- The VTKExtensions in the library/VTKExtensions folder.

VTKExtensions are separated in different modules.
- Core, that do not depend on any other VTKExtensions modules are provide services for all modules
- Readers, that depends on Core and implements many new VTK readers and importers
- Rendering, that depends on Core and implements the rendering specificities of F3D
- Applicative, the depends on all other VTKExtension modules and provide services for the libf3d library

The libf3d implements the whole logic of instancing and manipulating the different VTK classes, it is fully documented [here](../libf3d/README.md).

The F3D application itself uses the libf3d but adds an applicative layer on top of it, especially the handling of [command line options](../OPTIONS.md)
and [configuration file](../CONFIGURATION_FILE.md).
