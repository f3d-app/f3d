"""All of this should go into the `f3d` module's `__init__.py`"""

import re
import warnings

import f3d


################################################################################
# monkey patch `options.update`


def f3d_options_update(self, arg):
    try:
        for k, v in arg.items():
            self[k] = v
        return
    except AttributeError:
        pass

    try:
        for k, v in arg:
            self[k] = v
        return
    except AttributeError:
        pass

    raise ValueError(f"cannot update {self} from {args}")


f3d.options.update = f3d_options_update


################################################################################
# add deprecated warnings


def deprecated_decorator(f, reason):
    def g(*args, **kwargs):
        warnings.warn(reason, DeprecationWarning, 2)
        return f(*args, **kwargs)

    return g


def add_deprecation_warnings():
    for f3d_class in (
        f3d.camera,
        f3d.loader,
        f3d.options,
        f3d.interactor,
        f3d.engine,
        f3d.window,
    ):
        for name, member in f3d_class.__dict__.items():
            if callable(member):
                if m := re.search(r"DEPRECATED(:\s*.+)?", member.__doc__ or ""):
                    reason = m.group(1) or ""
                    msg = f"{f3d_class.__qualname__}.{name} is deprecated{reason}"
                    setattr(f3d_class, name, deprecated_decorator(member, msg))


add_deprecation_warnings()
