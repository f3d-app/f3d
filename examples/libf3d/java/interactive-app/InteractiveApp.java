import app.f3d.F3D.*;

public class InteractiveApp {

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
        if (args.length < 1) {
            System.err.println("Usage: InteractiveApp <file> [timeout]");
            System.exit(1);
        }

        String file = args[0];

        Engine.autoloadPlugins();

        try (Engine engine = Engine.create(false)) {
            Options options = engine.getOptions();

            options.setAsBool("render.grid.enable", true);
            options.setAsBool("render.show_edges", true);

            options.setAsBool("ui.axis", true);
            options.setAsBool("ui.fps", true);
            options.setAsBool("ui.animation_progress", true);
            options.setAsBool("ui.filename", true);

            Scene scene = engine.getScene();
            try {
                scene.add(file);
            } catch (Exception e) {
                System.err.println("Failed to load file: " + file);
                System.err.println(e.getMessage());
                System.exit(1);
            }

            Window window = engine.getWindow();
            window.render();

            Interactor interactor = engine.getInteractor();

            if (args.length > 1) {
                // For testing purposes, stop after timeout seconds
                int timeout = Integer.parseInt(args[1]);
                interactor.start(timeout, () -> interactor.stop());
            } else {
                interactor.start();
            }
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}
