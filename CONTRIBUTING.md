# How to Contribute

F3D welcomes all contributors, regardless of skill level or experience!

## Contributing as a User

Contributing to F3D can be as simple as pointing out a spelling mistake on the website,
reporting a bug you encountered, or suggesting a new feature you feel would improve the application.

We use [GitHub](https://github.com/f3d-app/f3d) to manage the project and you can report new issues or weigh
in on existing ones via our [issue tracker](https://github.com/f3d-app/f3d/issues).

Also, do not hesitate to join our [Discord](https://discord.f3d.app)!

## How to contribute to the documentation

Another way to get contribute is to improve the documentation, look for typos, improve phrasing, the examples and the gallery.
This does not require to build F3D or to learn C++ and is a very important work!

You can follow the workflow described below.

## How to Get Started With Development

To contribute to F3D as a developer, first you may want to try and build F3D for yourself.
If you are already familiar with software compilation, you can take a look at our [build guide](doc/dev/BUILD.md).
If not, you may want to look at our [getting started guide](doc/dev/GETTING_STARTED.md), that provide steps to compile F3D.
You may also want to take a look into the [F3D mentoring program](doc/dev/MENTORING.md) in order to get help with all the
contributing steps.

Once you are able to build F3D, you may want to take a look at the open [issues](https://github.com/f3d-app/f3d/issues)!
If you are a beginner, you may want to look at ["good first issues"](https://github.com/f3d-app/f3d/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22).
If you already have some experience with programming and contribution, definitely look at ["help wanted"](https://github.com/f3d-app/f3d/issues?q=is%3Aopen+is%3Aissue+label%3A%22help+wanted%22) issues.
If an issue is interesting to you and is not yet assigned, then you should _comment on the issue_ and ask for any help or clarification needed.
F3D maintainers will see your comment, assign the issue to you and provide guidance as needed.

To get involved more deeply, please take a look at the [roadmaps](doc/dev/ROADMAPS_AND_RELEASES.md) for the upcoming releases.

It is also recommended to reach out on [Discord](https://discord.f3d.app) to simplify communication, but it is not required.

You may also want to understand the overall [architecture](doc/dev/ARCHITECTURE.md) of the F3D project.

You can then fix the issue or implement the feature on your side and contribute it to the F3D repository by following the workflow described below.

Of course, if you are already using F3D and want to improve it for your specific needs, because you want a feature or found a bug,
that is definitely possible. Feel free to reach out for guidance by opening an issue or asking on [Discord](https://discord.f3d.app).

## F3D Development Workflow

F3D uses [GitLab Flow](https://about.gitlab.com/topics/version-control/what-is-gitlab-flow/). In a few words, here is how to contribute:

- [Fork](https://github.com/f3d-app/f3d/fork) the F3D repository on GitHub.
- Comment on a chosen issue, if any, so it can be assigned to you by a maintainer.
- Create and push a feature branch on your fork containing new commits.
- When it is ready for review or when you want to run the CI, create a pull request against `f3d-app/f3d/master`.
- Once the PR has been created, you can run the different parts of the CI by checking checkboxes in your PR description.
- Maintainers and returning contributors will provide help to interpret the CI and fix issues that may arise.
- Your PR will be reviewed by maintainers and returning contributors, please take their feedback into account and resolve discussions when adresssed.
- Once the PR is approved and CI comes back clean, a maintainer will merge your pull request in the master branch.
- The master now contains your changes and will be present in the next minor/major release, any documentation related changes are visible in [https://f3d.app](https://f3d.app)!

## Continuous Integration

F3D has pretty extensive continuous integration trying to cover all usecases for F3D.
It means that if the change in your pull request breaks continuous integration, it will not be merged until the full CI passes successfully.
It also means that adding a new feature or behavior means adding an associated test.
Make sure to check the results for yourself and ask for help if needed.

To run the CI, just add a comment like this in your PR:

- `\ci fast`: Style checks and a fast linux job
- `\ci main`: Cross platform CI that cover most usecases, including coverage, contains `ci:fast`.
- `\ci full`: Complete CI, required before merge, contains `ci:main`.

After this, the CI will always be run every time you push to your branch.
To remove a label, use the same syntax with a `-` before the label, eg: `\ci -fast`.

F3D continuous integration will also check the coverage as it is a good way to evaluate if new features are being tested or not.
When adding code to F3D, always try to cover it by adding/modifying [tests](doc/dev/TESTING.md).

F3D continuous integration also checks formatting using clang-format and other tools and will inform you if changes needs to be made.
Some [formatting rules](doc/dev/CODING_STYLE.md) are not enforced by clang-format and will be checked during the review process.

When making changes to the `libf3d` public API, continuous integration will warn about making related changes to the bindings.
This is required in order to merge the pull request.

When making changes to the `default_versions.json` file, continuous integration will warn about updating the timestamp in the same file.
This is required in order to merge the pull request.

The continuous integration is just code, as the rest of F3D is. If you want to fix or improve it, you are very welcome to!
