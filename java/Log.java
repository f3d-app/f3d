package app.f3d.F3D;

public class Log {
    /**
     * Enumeration of verbose levels.
     */
    public enum VerboseLevel {
        DEBUG(0),
        INFO(1),
        WARN(2),
        ERROR(3),
        QUIET(4);

        private final int value;

        VerboseLevel(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static VerboseLevel fromValue(int value) {
            for (VerboseLevel level : VerboseLevel.values()) {
                if (level.value == value) {
                    return level;
                }
            }
            throw new IllegalArgumentException("Invalid VerboseLevel value: " + value);
        }
    }

    /**
     * Callback interface for log forwarding.
     */
    @FunctionalInterface
    public interface ForwardCallback {
        /**
         * Called when a log message is forwarded.
         *
         * @param level verbose level of the message
         * @param message log message
         */
        void onLogMessage(VerboseLevel level, String message);
    }

    // Load the native library
    static {
        System.loadLibrary("f3d-java");
    }

    private Log() {
    }

    /**
     * Log a message at the specified verbose level.
     *
     * @param level verbose level for the message
     * @param message message string
     */
    public static native void print(VerboseLevel level, String message);

    /**
     * Log a debug message.
     *
     * @param message message string
     */
    public static native void debug(String message);

    /**
     * Log an info message.
     *
     * @param message message string
     */
    public static native void info(String message);

    /**
     * Log a warning message.
     *
     * @param message message string
     */
    public static native void warn(String message);

    /**
     * Log an error message.
     *
     * @param message message string
     */
    public static native void error(String message);

    /**
     * Set the coloring usage.
     *
     * @param use if true, coloring will be used
     */
    public static native void setUseColoring(boolean use);

    /**
     * Set the verbose level.
     *
     * @param level verbose level to set
     * @param forceStdErr if true, all messages are written to stderr
     */
    public static native void setVerboseLevel(VerboseLevel level, boolean forceStdErr);

    /**
     * Set the verbose level.
     *
     * @param level verbose level to set
     */
    public static void setVerboseLevel(VerboseLevel level) {
        setVerboseLevel(level, false);
    }

    /**
     * Get the current verbose level.
     *
     * @return current verbose level
     */
    public static native VerboseLevel getVerboseLevel();

    /**
     * Set a callback function to forward log messages.
     *
     * @param callback callback function, or null to disable forwarding
     */
    public static native void forward(ForwardCallback callback);
}
