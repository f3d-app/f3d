package app.f3d.F3D;

public class Context {

    /** Thrown when a shared library required for a context cannot be loaded. */
    public static class LoadingException extends F3DException {
        public LoadingException(String message) { super(message); }
    }

    /** Thrown when a required symbol cannot be resolved from a shared library. */
    public static class SymbolException extends F3DException {
        public SymbolException(String message) { super(message); }
    }

    // Used to load the 'f3d' library on application startup.
    static {
        System.loadLibrary("f3d-java");
    }

    /**
     * Create a GLX context.
     *
     * @return context handle (must be deleted with {@link #delete(long)})
     */
    public static native long glx();

    /**
     * Create a WGL context.
     *
     * @return context handle (must be deleted with {@link #delete(long)})
     */
    public static native long wgl();

    /**
     * Create a COCOA context.
     *
     * @return context handle (must be deleted with {@link #delete(long)})
     */
    public static native long cocoa();

    /**
     * Create an EGL context.
     *
     * @return context handle (must be deleted with {@link #delete(long)})
     */
    public static native long egl();

    /**
     * Create an OSMesa context.
     *
     * @return context handle (must be deleted with {@link #delete(long)})
     */
    public static native long osmesa();

    /**
     * Create a context from a library name and a function name.
     *
     * @param lib the library name (without prefix or extension)
     * @param func the function name to resolve
     * @return context handle (must be deleted with {@link #delete(long)})
     */
    public static native long getSymbol(String lib, String func);

    /**
     * Delete a context and free associated resources.
     *
     * @param contextHandle the context handle to delete
     */
    public static native void delete(long contextHandle);
}
