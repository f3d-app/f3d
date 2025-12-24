package app.f3d.F3D;

import java.util.List;

public class Options {

    /**
     * Package-private constructor for creating an Options instance from a native pointer.
     *
     * @param nativeAddress the native memory address of the underlying f3d::options object
     */
    Options(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Sets a boolean option value.
     *
     * @param name the name of the option to set
     * @param value the boolean value to set
     * @throws IllegalArgumentException if the option name does not exist or is not a boolean type
     */
    public native void setAsBool(String name, boolean value);

    /**
     * Sets an integer option value.
     *
     * @param name the name of the option to set
     * @param value the integer value to set
     * @throws IllegalArgumentException if the option name does not exist or is not an integer type
     */
    public native void setAsInt(String name, int value);

    /**
     * Sets a double option value.
     *
     * @param name the name of the option to set
     * @param value the double value to set
     * @throws IllegalArgumentException if the option name does not exist or is not a double type
     */
    public native void setAsDouble(String name, double value);

    /**
     * Sets a string option value.
     *
     * @param name the name of the option to set
     * @param value the string value to set
     * @throws IllegalArgumentException if the option name does not exist or is not a string type
     */
    public native void setAsString(String name, String value);

    /**
     * Sets a double vector option value.
     *
     * @param name the name of the option to set
     * @param values the array of double values to set
     * @throws IllegalArgumentException if the option name does not exist or is not a double vector type
     */
    public native void setAsDoubleVector(String name, double[] values);

    /**
     * Sets an integer vector option value.
     *
     * @param name the name of the option to set
     * @param values the array of integer values to set
     * @throws IllegalArgumentException if the option name does not exist or is not an integer vector type
     */
    public native void setAsIntVector(String name, int[] values);

    /**
     * Gets a boolean option value.
     *
     * @param name the name of the option to retrieve
     * @return the boolean value of the option
     * @throws IllegalArgumentException if the option name does not exist or is not a boolean type
     */
    public native boolean getAsBool(String name);

    /**
     * Gets an integer option value.
     *
     * @param name the name of the option to retrieve
     * @return the integer value of the option
     * @throws IllegalArgumentException if the option name does not exist or is not an integer type
     */
    public native int getAsInt(String name);

    /**
     * Gets a double option value.
     *
     * @param name the name of the option to retrieve
     * @return the double value of the option
     * @throws IllegalArgumentException if the option name does not exist or is not a double type
     */
    public native double getAsDouble(String name);

    /**
     * Gets a string option value.
     *
     * @param name the name of the option to retrieve
     * @return the string value of the option
     * @throws IllegalArgumentException if the option name does not exist or is not a string type
     */
    public native String getAsString(String name);

    /**
     * Gets the string representation of an option value regardless of its underlying type.
     *
     * @param name the name of the option to retrieve
     * @return the string representation of the option value
     * @throws IllegalArgumentException if the option name does not exist
     */
    public native String getAsStringRepresentation(String name);

    /**
     * Sets an option value from its string representation.
     *
     * @param name the name of the option to set
     * @param str the string representation to parse and set
     * @throws IllegalArgumentException if the option name does not exist or the string cannot be parsed
     */
    public native void setAsStringRepresentation(String name, String str);

    /**
     * Gets a double vector option value.
     *
     * @param name the name of the option to retrieve
     * @return the array of double values
     * @throws IllegalArgumentException if the option name does not exist or is not a double vector type
     */
    public native double[] getAsDoubleVector(String name);

    /**
     * Gets an integer vector option value.
     *
     * @param name the name of the option to retrieve
     * @return the array of integer values
     * @throws IllegalArgumentException if the option name does not exist or is not an integer vector type
     */
    public native int[] getAsIntVector(String name);

    /**
     * Toggles a boolean option value.
     *
     * @param name the name of the boolean option to toggle
     * @throws IllegalArgumentException if the option name does not exist or is not a boolean type
     */
    public native void toggle(String name);

    /**
     * Checks if the specified option has the same value in this instance and another Options instance.
     *
     * @param other the other Options instance to compare with
     * @param name the name of the option to compare
     * @return true if both options have the same value, false otherwise
     * @throws IllegalArgumentException if the option name does not exist
     */
    public native boolean isSame(Options other, String name);

    /**
     * Checks if the specified option has a value set.
     *
     * @param name the name of the option to check
     * @return true if the option has a value, false otherwise
     * @throws IllegalArgumentException if the option name does not exist
     */
    public native boolean hasValue(String name);

    /**
     * Copies the value of an option from another Options instance to this instance.
     *
     * @param other the Options instance to copy from
     * @param name the name of the option to copy
     * @throws IllegalArgumentException if the option name does not exist
     */
    public native void copy(Options other, String name);

    /**
     * Gets a list of all available option names in F3D.
     *
     * @return a list containing all option names
     */
    public static native List<String> getAllNames();

    /**
     * Gets a list of option names that have values set in this Options instance.
     *
     * @return a list containing names of options with values
     */
    public native List<String> getNames();

    /**
     * Finds the closest matching option name to the provided string.
     *
     * @param option the option name to find a match for
     * @return a ClosestOptionResult containing the closest match name and edit distance
     */
    public native ClosestOptionResult getClosestOption(String option);

    /**
     * Checks if the specified option is optional.
     *
     * @param name the name of the option to check
     * @return true if the option is optional, false if it is required
     * @throws IllegalArgumentException if the option name does not exist
     */
    public native boolean isOptional(String name);

    /**
     * Resets the specified option to its default value.
     *
     * @param name the name of the option to reset
     * @throws IllegalArgumentException if the option name does not exist
     */
    public native void reset(String name);

    /**
     * Removes the value of the specified option, making it unset.
     *
     * @param name the name of the option to remove the value from
     * @throws IllegalArgumentException if the option name does not exist
     */
    public native void removeValue(String name);

    /**
     * Result class for the closest option search operation.
     */
    public static class ClosestOptionResult {
        /**
         * The name of the closest matching option.
         */
        public String name;

        /**
         * The edit distance to the match.
         */
        public int distance;

        /**
         * Constructs a ClosestOptionResult.
         *
         * @param name the name of the closest matching option
         * @param distance the edit distance to the match
         */
        public ClosestOptionResult(String name, int distance) {
            this.name = name;
            this.distance = distance;
        }
    }

    /**
     * Native pointer to the underlying f3d::options object.
     */
    private long mNativeAddress;
}
