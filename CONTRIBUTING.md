# How to contribute

F3D welcomes all contributors, regardless of skill level or experience!

## Contributing as a user

Contributing to F3D can be as simple as pointing out a spelling mistake on the website,
reporting a bug you encountered, or suggesting a new feature you feel would improve the application.

We use [GitHub](https://github.com/f3d-app/f3d) to manage the project and you can report new issues or weigh
in on existing ones via our [issue tracker](https://github.com/f3d-app/f3d/issues).

Also, do not hesitate to join our [Discord](https://discord.f3d.app)!

## How to contribute to the documentation

Another way to contribute is to improve the documentation, look for typos, and improve phrasing, examples, and the gallery.
This does not require building F3D or learning C++ and is very important work!
When writing or editing documentation, follow the [documentation style guide](doc/dev/15-DOCUMENTATION_STYLE.md).

You can follow the workflow described below.

## How to get started with development

Where to start depends on the kind of contribution you have in mind:

- **Documentation only**: no build is required. Write your changes following the [documentation style guide](doc/dev/15-DOCUMENTATION_STYLE.md), then preview them with the [website preview recipe](doc/dev/07-TOOLING.md#how-to-locally-generate-and-run-the-website).
- **Code contributions**: start with our [Getting Started guide](doc/dev/04-GETTING_STARTED.md), then follow the [Build guide](doc/dev/05-BUILD.md). After that, read the [Architecture](doc/dev/08-ARCHITECTURE.md) page and the [F3D mentoring program](doc/dev/13-MENTORING.md) if you want more guidance.

Once you know which path applies, take a look at the open [issues](https://github.com/f3d-app/f3d/issues).
If you are a beginner, you may want to start with ["good first issues"](https://github.com/f3d-app/f3d/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22).
Please note that [generative AI should not be used](AI_POLICY.md) in that context.
If you already have some experience with programming and contribution, definitely look at ["help wanted"](https://github.com/f3d-app/f3d/issues?q=is%3Aopen+is%3Aissue+label%3A%22help+wanted%22) issues.
If an issue is interesting to you and is not yet assigned, then you should _comment on the issue_ and ask for any help or clarification needed.
F3D maintainers will see your comment, assign the issue to you and provide guidance as needed.

To get involved more deeply, please take a look at the [roadmaps](doc/dev/10-ROADMAPS_AND_RELEASES.md) for the upcoming releases.

It is also recommended to reach out on [Discord](https://discord.f3d.app) to simplify communication, but it is not required.

You can then fix the issue or implement the feature on your side and contribute it to the F3D repository by following the workflow described below.

Of course, if you are already using F3D and want to improve it for your specific needs, because you want a feature or found a bug,
that is definitely possible. Feel free to reach out for guidance by opening an issue or asking on [Discord](https://discord.f3d.app).

## AI policy

F3D has a dedicated [AI policy](AI_POLICY.md). If you are an AI user, make sure to read it and comply with it.

## F3D development workflow

F3D uses [GitLab Flow](https://about.gitlab.com/topics/version-control/what-is-gitlab-flow/). In a few words, here is how to contribute:

- [Fork](https://github.com/f3d-app/f3d/fork) the F3D repository on GitHub.
- Comment on a chosen issue, if any, so it can be assigned to you by a maintainer.
- Create and push a new feature branch on your fork containing new commits, do not use a `main` or `master` branch.
- As soon as possible, create a _draft_ pull request against `f3d-app/f3d/master` so that maintainers are aware and design can be discussed.
- Once the PR has been created, even as draft, it is possible to [run the CI](#continuous-integration), feel free to do so.
- When it is ready for review, undraft your pull request, which will notify maintainers. Also make sure to check the needed checkboxes in the description.
- To ask for a review or if you need help with CI, use the review system of GitHub to request a review from [the maintainers](doc/dev/12-MAINTAINERS_AND_CONTRIBUTORS.md#maintainers).
- You can also tag maintainers on GitHub or [Discord](#discord-usage) to ask for help and review.
- Your PR will then be reviewed by maintainers and returning contributors, please take their feedback into account and resolve discussions when addressed.
- This process of reviewing and then addressing reviews may be repeated and take some time depending on the complexity of the PR.
- In general, do not merge with `master`, even if GitHub suggests you do so.
- In general, there is no need to rebase with `master` but doing so is a good practice when pushing new changes.
- Once the PR is approved and CI comes back clean, a maintainer will merge your pull request in the master branch.
- If the PR was linked with an issue, please ensure the issue is closed or update to reflect the change in master.
- The master now contains your changes and will be present in the next minor/major release, any documentation related changes are visible on [the website](https://f3d.app)!

## Discord usage

- If you are a Discord user, then you are most welcome to join the [F3D Discord](https://discord.f3d.app)
- When discussing a specific issue or pull request, create a dedicated thread in the "Contribute" channel
- As a first message, post a link to the issue, a moderator will pin it to the thread
- Discussing issues, design, and investigation on Discord is usually more efficient than on GitHub, but always try to write down the discussion conclusions on GitHub

## Continuous integration

F3D has pretty extensive continuous integration trying to cover all use cases for F3D.
It means that if the change in your pull request breaks continuous integration, it will not be merged until the full CI passes successfully.
It also means that adding a new feature or behavior means adding an associated test.
Make sure to check the results for yourself and ask for help if needed.

To run the CI, add one of these comments in your PR:

- `\ci fast`: Fast Linux job without optional dependencies. Make this pass first.
- `\ci extended`: Extended Linux job with dependencies and a recent VTK. Make this pass second.
- `\ci linux`: Linux jobs across CMake options, dependencies, and VTK versions.
- `\ci windows`: Windows jobs across VTK versions.
- `\ci macos_intel`: macOS Intel jobs across VTK versions.
- `\ci macos_arm`: macOS arm64 jobs across VTK versions.
- `\ci coverage`: Linux job for coverage computation.
- `\ci sanitizer`: Linux jobs with different sanitizer settings.
- `\ci analysis`: Linux cppcheck job.
- `\ci external`: Linux job that builds libf3d as a subproject of a larger project.
- `\ci python`: Cross-platform jobs that build libf3d for Python versions.
- `\ci wasm`: Build Docker images and then build libf3d with WebAssembly.
- `\ci android`: Build Docker images and then build libf3d for Android.
- `\ci website`: Build the f3d.app website using the current documentation.
- `\ci full`: All of the above, required before merging.

After this, CI runs every time you push to your branch.
To remove a label, use the same syntax with a `-` before the label, for example: `\ci -fast`.

Please add only the labels required to work on your feature, in order to avoid using the limited pool of runners for no good reason.

F3D continuous integration will also check the coverage as it is a good way to evaluate if new features are being tested or not.
When adding code to F3D, always try to cover it by adding/modifying [tests](doc/dev/06-TESTING.md).

F3D continuous integration also checks formatting using clang-format and other tools and will inform you if changes need to be made.
Some [formatting rules](doc/dev/09-CODING_STYLE.md) are not enforced by clang-format and will be checked during the review process.

When making changes to the `libf3d` public API, continuous integration will warn about making related changes to the bindings.
The pull request cannot be merged without this.

When making changes to the `default_versions.json` file, continuous integration will warn about updating the docker timestamp in the same file.
The pull request cannot be merged without this. This also requires running the `cache` part of the CI, which can only be triggered by maintainers.

Continuous integration is code, like the rest of F3D. You are welcome to fix or improve it!
