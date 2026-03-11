import app.f3d.F3D.*;

public class TestOptions {

  static {
    if (System.getProperty("os.name").startsWith("Windows"))
    {
      System.loadLibrary("opengl32");
    }
  }

  public static void main(String[] args) {
    Engine.autoloadPlugins();

    Engine engine = Engine.createNone();
    Options options = engine.getOptions();

    options.setAsBool("ui.scalar_bar", true);
    options.getAsBool("ui.scalar_bar");
    options.toggle("ui.scalar_bar");

    options.setAsInt("scene.animation.index", 5);
    options.getAsInt("scene.animation.index");

    options.setAsDouble("render.line_width", 2.0);
    options.getAsDouble("render.line_width");

    options.setAsString("model.color.texture", "test.png");
    options.getAsString("model.color.texture");

    options.setAsDoubleVector("render.background.color", new double[]{1.0, 2.0, 3.0});
    options.getAsDoubleVector("render.background.color");

    options.setAsIntVector("scene.animation.indices", new int[]{1, 2});
    options.getAsIntVector("scene.animation.indices");

    options.setAsStringRepresentation("render.point_size", "5.0");
    options.getAsStringRepresentation("render.point_size");

    options.hasValue("ui.scalar_bar");
    options.getNames();
    options.getAllNames();
    options.getClosestOption("ui.scalar");
    options.isOptional("ui.scalar_bar");

    Engine engine2 = Engine.create(true);
    Options options2 = engine2.getOptions();
    options2.copy(options, "ui.scalar_bar");
    options.isSame(options2, "ui.scalar_bar");
    engine2.close();

    options.reset("render.line_width");
    options.removeValue("model.color.texture");

    engine.close();
  }
}
