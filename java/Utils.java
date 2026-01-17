package app.f3d.F3D;

import java.util.List;

public class Utils {

    /**
     * Enumeration of supported Windows known folders.
     */
    public enum KnownFolder {
        ROAMINGAPPDATA(0),
        LOCALAPPDATA(1),
        PICTURES(2);

        private final int value;

        KnownFolder(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static KnownFolder fromValue(int value) {
            for (KnownFolder folder : KnownFolder.values()) {
                if (folder.value == value) {
                    return folder;
                }
            }
            throw new IllegalArgumentException("Invalid KnownFolder value: " + value);
        }
    }

    // Load the native library
    static {
        System.loadLibrary("f3d-java");
    }

    private Utils() {
    }

    /**
     * Compute the Levenshtein distance between two strings.
     *
     * @param strA first string
     * @param strB second string
     * @return Levenshtein distance between the two strings
     */
    public static native int textDistance(String strA, String strB);

    /**
     * Tokenize a string using the same logic as bash.
     *
     * @param str input string to tokenize
     * @param keepComments if true, keep comments with '#'
     * @return list of tokens
     */
    public static native List<String> tokenize(String str, boolean keepComments);

    /**
     * Tokenize a string with keepComments enabled.
     *
     * @param str input string to tokenize
     * @return list of tokens
     */
    public static List<String> tokenize(String str) {
        return tokenize(str, true);
    }

    /**
     * Collapse a filesystem path.
     *
     * @param path input path
     * @param baseDirectory base directory for relative paths
     * @return collapsed absolute path string
     */
    public static native String collapsePath(String path, String baseDirectory);

    /**
     * Collapse a filesystem path using current directory as base.
     *
     * @param path input path
     * @return collapsed absolute path string
     */
    public static String collapsePath(String path) {
        return collapsePath(path, "");
    }

    /**
     * Converts a glob expression to a regular expression.
     *
     * @param glob glob expression
     * @param pathSeparator path separator character
     * @return regular expression string
     */
    public static native String globToRegex(String glob, char pathSeparator);

    /**
     * Converts a glob expression to a regular expression with default separator '/'.
     *
     * @param glob glob expression
     * @return regular expression string
     */
    public static String globToRegex(String glob) {
        return globToRegex(glob, '/');
    }

    /**
     * Get the value of an environment variable.
     *
     * @param env environment variable name
     * @return value of the environment variable, or null if not set
     */
    public static native String getEnv(String env);

    /**
     * Get a Windows known folder.
     *
     * @param knownFolder known folder identifier
     * @return folder path, or null on non-Windows platforms or error
     */
    public static native String getKnownFolder(KnownFolder knownFolder);

    /**
    * Get the primary monitor system zoom scale base on DPI.
    * 
    * Supported on Windows and Linux platforms.
    * 
    * @return DPI scale in double, or hardcoded 1.0 on other platforms.
    */
    public static native double getDPIScale();
}
