# *libf3d API Public Review*

Hello and welcome to the public review of the libf3d

> Wait, what is the libf3d ?

The libf3d is a simple BSD licensed C++ library to render meshes, currently in developpement.

> Ok, but what is a public review ?

The idea of a public review is to let anyone interested give us their remarks and ideas about the
API of the libf3d. 

While we have some understanding on how the libf3d could be used, there is many usecases
that we may have missed. If you have such usecase in mind, now is the time to share.

Secondly, this is the first time we create an complete, modern, C++11 API from scratch.
We tried to do it right, but maybe you have some insight, which would be very welcome !

> Sounds good, can I try the libf3d already ?

Yes, you can ! take a look here, the F3D project contains the libf3d as well as F3D, a executable
using it. But let's focus on the review for now

> Ok, where do I start ?

Take a look at the *Files Changed* tab in this pull request.

For starters, you can review README_libf3d.md, it will give you some examples about how to use the libf3d,
the different classes and options.

Then you may want to reader README_F3D.md. While not everything there is related to the libf3d, it will give you some hindsight
about how you could use the libf3d and what we do with it for now.

Finally, you can take a look in the library/public directory where each public class headers can be found.
The in-header doc is quite extensive, although it may not be enough. Do not hesitate to ask questions, we will clarify and improve the doc in the process.

So you are welcome to ask questions, make suggestions either here in the PR body or on specific part of the diff.

We will make sure to try to adress all of them.

> Ok, for how long will this run ?

We plan to close this review by the end of august.
