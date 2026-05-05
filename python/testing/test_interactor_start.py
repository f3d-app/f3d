from functools import partial

import f3d


# def stop_fn(_state: f3d.InteractorState, inter: f3d.Interactor):
#     inter.stop()
# 
# 
# def test_start_stop():
#     engine = f3d.Engine.create(True)
#     inter = engine.interactor
#     inter.set_event_loop_user_callback(partial(stop_fn, inter=inter))
#     inter.start(1 / 30)
# 
# 
# def test_request_render():
#     engine = f3d.Engine.create(True)
#     inter = engine.interactor
#     inter.request_render()
#     inter.set_event_loop_user_callback(partial(stop_fn, inter=inter))
#     inter.start(1 / 30)
# 
# 
# def test_request_stop():
#     engine = f3d.Engine.create(True)
#     inter = engine.interactor
#     inter.set_event_loop_user_callback(lambda _state: inter.request_stop())
#     inter.start(1 / 30)
