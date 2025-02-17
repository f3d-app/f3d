F3D is a fast and minimalist 3D viewer. Implemented in C++, it has been cross-compiled with emscripten to a WebAssembly module.

While the webassembly module might be quite large (~14MB), it is self contained and requires no external dependency.

The current web build parses some parameters provided as url-params, via a hash search-query:

- `model` and `extension` let you pass a model to load via its url. Extension can be provided in case it is not obvious to detect it automatically. In case extension is provided, the loader will enforce it, otherwise, it can try to get file type from header if available, otherwise from url.
- `axis, grid, fxaa, tone, ssao, ambient` can be set to false to disable toggle
- `up` can take values +Y or +Z

Some passed model urls might be stored on servers which do not set the Cross-Origin request header parameter. In that case, you can still load these with plugins like Allow-Cors that do exist for chrome, firefox etc.

Example url which requires CORS Plugin: `https://f3d.app/web#up=+Y&axis=false&ssao=true&model=https://groups.csail.mit.edu/graphics/classes/6.837/F03/models/teapot.obj`. Also see this [url with all params](https://f3d.app/web#model=https://groups.csail.mit.edu/graphics/classes/6.837/F03/models/teapot.obj&up=%2BY&grid=false&fxaa=false&tone=false&ssao=false&ambient=false&axis=false)
