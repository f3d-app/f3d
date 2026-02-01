import app.f3d.F3D.*;

public class TestInteractor {

  // On Windows, try to load opengl32 from Java path
  // It's only useful in order to force Mesa software OpenGL
  static {
    if (System.getProperty("os.name").startsWith("Windows"))
    {
      try {
        System.loadLibrary("opengl32");
      } catch (UnsatisfiedLinkError e) {
        // Ignore if opengl32 is not available
      }
    }
  }

  public static void main(String[] args) {
    Engine.autoloadPlugins();

    Engine engine = Engine.create(true);
    Interactor interactor = engine.getInteractor();

    interactor.addCommand("test::hello", cmdArgs -> {
      System.out.println("Hello command called with " + cmdArgs.size() + " args");
    });

    interactor.getCommandActions();
    interactor.removeCommand("test::hello");

    Interactor.InteractionBind bind = new Interactor.InteractionBind();
    bind.mod = Interactor.ModifierKeys.CTRL;
    bind.inter = "A";

    bind.format();
    Interactor.InteractionBind parsed = Interactor.InteractionBind.parse("Ctrl+A");
    Interactor.InteractionBind parsed2 = Interactor.InteractionBind.parse("Shift+B");
    Interactor.InteractionBind parsed3 = Interactor.InteractionBind.parse("C");

    bind.equals(parsed);
    parsed.compareTo(parsed2);
    parsed2.hashCode();

    interactor.toggleAnimation();
    interactor.toggleAnimation(Interactor.AnimationDirection.FORWARD);
    interactor.toggleAnimation(Interactor.AnimationDirection.BACKWARD);
    interactor.startAnimation();
    interactor.startAnimation(Interactor.AnimationDirection.FORWARD);
    interactor.startAnimation(Interactor.AnimationDirection.BACKWARD);
    interactor.stopAnimation();
    interactor.isPlayingAnimation();
    interactor.getAnimationDirection();

    Interactor.AnimationDirection.FORWARD.getValue();
    Interactor.AnimationDirection.BACKWARD.getValue();
    Interactor.AnimationDirection.fromValue(0);
    Interactor.AnimationDirection.fromValue(1);

    interactor.enableCameraMovement();
    interactor.disableCameraMovement();

    interactor.requestRender();

    interactor.toggleAnimation()
             .enableCameraMovement();

    engine.close();
  }
}
