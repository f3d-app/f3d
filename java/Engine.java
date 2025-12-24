package app.f3d.F3D;

import java.util.List;
import java.util.Map;

public class Engine implements AutoCloseable {
    @FunctionalInterface
    public interface ContextFunction {
        /**
         * Get the address of an OpenGL function by name
         * @param name the function name (e.g., "glClear")
         * @return the function pointer address as a long
         */
        long getProcAddress(String name);
    }

    public static class LibInfo {
        public String version;
        public String versionFull;
        public String buildDate;
        public String buildSystem;
        public String compiler;
        public Map<String, Boolean> modules;
        public String vtkVersion;
        public List<String> copyrights;
        public String license;

        public LibInfo(String version, String versionFull, String buildDate, String buildSystem,
                       String compiler, Map<String, Boolean> modules, String vtkVersion,
                       List<String> copyrights, String license) {
            this.version = version;
            this.versionFull = versionFull;
            this.buildDate = buildDate;
            this.buildSystem = buildSystem;
            this.compiler = compiler;
            this.modules = modules;
            this.vtkVersion = vtkVersion;
            this.copyrights = copyrights;
            this.license = license;
        }
    }

    public static class ReaderInfo {
        public String name;
        public String description;
        public List<String> extensions;
        public List<String> mimeTypes;
        public String pluginName;
        public boolean hasSceneReader;
        public boolean hasGeometryReader;

        public ReaderInfo(String name, String description, List<String> extensions,
                          List<String> mimeTypes, String pluginName,
                          boolean hasSceneReader, boolean hasGeometryReader) {
            this.name = name;
            this.description = description;
            this.extensions = extensions;
            this.mimeTypes = mimeTypes;
            this.pluginName = pluginName;
            this.hasSceneReader = hasSceneReader;
            this.hasGeometryReader = hasGeometryReader;
        }
    }

    // Used to load the 'f3d' library on application startup.
    static {
        System.loadLibrary("f3d-java");
    }

    private Engine(long nativeAddress) {
        mNativeAddress = nativeAddress;
        mScene = new Scene(mNativeAddress);
        mOptions = new Options(mNativeAddress);
        mWindow = new Window(mNativeAddress);
    }

    /**
     * Create an engine with automatic window selection
     * @param offscreen if true, window will be hidden
     * @return new Engine instance
     */
    public static Engine create(boolean offscreen) {
        return new Engine(nativeCreate(offscreen));
    }

    /**
     * Create an engine with automatic window selection (non-offscreen)
     * @return new Engine instance
     */
    public static Engine create() {
        return create(false);
    }

    /**
     * Create an engine with no window
     * @return new Engine instance
     */
    public static Engine createNone() {
        return new Engine(nativeCreateNone());
    }

    /**
     * Create an engine with a GLX window (Linux only)
     * @param offscreen if true, window will be hidden
     * @return new Engine instance
     */
    public static Engine createGLX(boolean offscreen) {
        return new Engine(nativeCreateGLX(offscreen));
    }

    /**
     * Create an engine with a GLX window (Linux only, non-offscreen)
     * @return new Engine instance
     */
    public static Engine createGLX() {
        return createGLX(false);
    }

    /**
     * Create an engine with a WGL window (Windows only)
     * @param offscreen if true, window will be hidden
     * @return new Engine instance
     */
    public static Engine createWGL(boolean offscreen) {
        return new Engine(nativeCreateWGL(offscreen));
    }

    /**
     * Create an engine with a WGL window (Windows only, non-offscreen)
     * @return new Engine instance
     */
    public static Engine createWGL() {
        return createWGL(false);
    }

    /**
     * Create an engine with an offscreen EGL window
     * @return new Engine instance
     */
    public static Engine createEGL() {
        return new Engine(nativeCreateEGL());
    }

    /**
     * Create an engine with an offscreen OSMesa window
     * @return new Engine instance
     */
    public static Engine createOSMesa() {
        return new Engine(nativeCreateOSMesa());
    }

    /**
     * Create an engine with an external context using a custom getProcAddress function
     * @param getProcAddress callback function to resolve OpenGL function addresses
     * @return new Engine instance
     */
    public static Engine createExternal(ContextFunction getProcAddress) {
        return new Engine(nativeCreateExternal(getProcAddress));
    }

    /**
     * Create an engine with an external GLX context (Linux only)
     * @return new Engine instance
     */
    public static Engine createExternalGLX() {
        return new Engine(nativeCreateExternalGLX());
    }

    /**
     * Create an engine with an external WGL context (Windows only)
     * @return new Engine instance
     */
    public static Engine createExternalWGL() {
        return new Engine(nativeCreateExternalWGL());
    }

    /**
     * Create an engine with an external COCOA context (macOS only)
     * @return new Engine instance
     */
    public static Engine createExternalCOCOA() {
        return new Engine(nativeCreateExternalCOCOA());
    }

    /**
     * Create an engine with an external EGL context
     * @return new Engine instance
     */
    public static Engine createExternalEGL() {
        return new Engine(nativeCreateExternalEGL());
    }

    /**
     * Create an engine with an external OSMesa context
     * @return new Engine instance
     */
    public static Engine createExternalOSMesa() {
        return new Engine(nativeCreateExternalOSMesa());
    }

    @Override
    public void close() {
        nativeDestroy(mNativeAddress);
    }

    /**
     * Set the cache path directory
     * @param cachePath path to cache directory
     */
    public native void setCachePath(String cachePath);

    /**
     * Set options for this engine
     * @param options the options to set
     */
    public native void setOptions(Options options);

    /**
     * Get the options
     * @return Options instance
     */
    public Options getOptions() { return mOptions; }

    /**
     * Get the scene
     * @return Scene instance
     */
    public Scene getScene() { return mScene; }

    /**
     * Get the window
     * @return Window instance
     */
    public Window getWindow() { return mWindow; }

    /**
     * Get the interactor
     * @return Interactor instance
     */
    public native Interactor getInteractor();

    /**
     * Load a plugin by path or name
     * @param plugin path or name of the plugin
     */
    public static native void loadPlugin(String plugin);

    /**
     * Automatically load all static plugins
     */
    public static native void autoloadPlugins();

    /**
     * List plugins available in the given directory
     * @param pluginPath path to search for plugins
     * @return list of available plugin names
     */
    public static native List<String> getPluginsList(String pluginPath);

    /**
     * Get information about libf3d
     * @return LibInfo object containing library information
     */
    public static native LibInfo getLibInfo();

    /**
     * Get information about supported readers
     * @return list of ReaderInfo objects
     */
    public static native List<ReaderInfo> getReadersInfo();

    /**
     * Get list of rendering backends supported by libf3d
     * @return map of backend names to availability boolean
     */
    public static native Map<String, Boolean> getRenderingBackendList();

    /**
     * Set a reader option value
     * @param name option name
     * @param value option value
     */
    public static native void setReaderOption(String name, String value);

    /**
     * Get all reader option names
     * @return list of option names
     */
    public static native List<String> getAllReaderOptionNames();

    // Native methods
    private static native long nativeCreate(boolean offscreen);
    private static native long nativeCreateNone();
    private static native long nativeCreateGLX(boolean offscreen);
    private static native long nativeCreateWGL(boolean offscreen);
    private static native long nativeCreateEGL();
    private static native long nativeCreateOSMesa();
    private static native long nativeCreateExternal(ContextFunction getProcAddress);
    private static native long nativeCreateExternalGLX();
    private static native long nativeCreateExternalWGL();
    private static native long nativeCreateExternalCOCOA();
    private static native long nativeCreateExternalEGL();
    private static native long nativeCreateExternalOSMesa();
    private static native void nativeDestroy(long nativeAddress);

    // Member variables
    private long mNativeAddress;
    private Scene mScene;
    private Options mOptions;
    private Window mWindow;
}
