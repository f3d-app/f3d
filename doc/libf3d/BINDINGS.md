# Bindings

## Python Bindings

If the python bindings have been generated using the `F3D_BINDINGS_PYTHON` CMake option, the libf3d can be used directly from python.
Make sure to set `PYTHONPATH` to path where the python module is built.
Here is an example showing how to use libf3d python bindings:

```python
import f3d

f3d.engine.autoloadPlugins()

eng = f3d.engine(f3d.window.NATIVE)
eng.getOptions()
  .set("model.scivis.array-name", "Normals")
  .set("model.scivis.component", 0)
  .set("ui.bar", True)
  .set("scene.grid", True)

eng.getLoader().addFile("f3d/testing/data/dragon.vtu").loadFile()
eng.getInteractor().start()
```

## Java Bindings

If the Java bindings have been generated using the `F3D_BINDINGS_JAVA` CMake option, the libf3d can be used directly from Java.
You can import the `f3d.jar` package and use the provided Java classes directly.
Make sure to set `java.library.path` to the path where the JNI library is built.
Here is an example showing how to use libf3d Java bindings:

```java
import app.f3d.F3D.*;

public class F3DExample {
  public static void main(String[] args) {

    Engine.autoloadPlugins();

    // Always use try-with-resources idiom to ensure the native engine is released
    try (Engine engine = new Engine(Window.Type.NATIVE)) {
      Loader loader = engine.getLoader();
      loader.addFile("f3d/testing/data/dragon.vtu");
      loader.loadFile(Loader.LoadFileEnum.LOAD_FIRST);

      engine.getWindow().render();
    }
  }
}
```
