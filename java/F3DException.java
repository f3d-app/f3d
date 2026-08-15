package app.f3d.F3D;

/**
 * Base class for all F3D Java exceptions.
 * Mirrors the f3d::exception C++ hierarchy, allowing callers to
 * catch any F3D error with a single catch block:
 *
 * <pre>{@code
 * try {
 *   engine.getScene().add("file.stl");
 * } catch (F3DException e) {
 *   // handles any f3d error
 * }
 * }</pre>
 */
public class F3DException extends RuntimeException {
    public F3DException(String message) {
        super(message);
    }
}
