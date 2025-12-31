package app.f3d.F3D;

public class Context {

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
