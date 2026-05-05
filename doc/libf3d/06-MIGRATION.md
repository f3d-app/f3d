# Migrate from 3.x

## User callback

When calling `f3d::interactor::start()` or `f3d::interactor::playInteraction()`, it was possible to set a user callback automatically called at each event loop.
If you was setting such callback, please call `f3d::interactor::setEventLoopUserCallBack()` manually before calling `start()` or `playInteractor()`.
The callback takes an argument of type `f3d::interactor::interactor_state_t` allowing you to retrieve useful information about the interactor state.
