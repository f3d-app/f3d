package app.f3d.F3D;

public class Engine implements AutoCloseable {
    // Used to load the 'f3d' library on application startup.
    static {
        System.loadLibrary("f3d-java");
    }

    public Engine() {
        mNativeAddress = construct(); // instantiate the native engine
        mLoader = new Loader(mNativeAddress);
        mOptions = new Options(mNativeAddress);
        mWindow = new Window(mNativeAddress);
    }

    // The engine class is automatically released using the Java
    // try-with-resources idiom
    // see https://www.baeldung.com/java-try-with-resources
    @Override
    public void close() {
        destroy(mNativeAddress);
    }

    public native void setCachePath(String cachePath);

    static public native void loadPlugin(String plugin);
    static public native void autoloadPlugins();

    public Loader getLoader() { return mLoader; }
    public Options getOptions() { return mOptions; }
    public Window getWindow() { return mWindow; }

    private native long construct();
    private native void destroy(long nativeAddress);

    private Loader mLoader;
    private Options mOptions;
    private Window mWindow;

    private long mNativeAddress;
}
