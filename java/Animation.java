package app.f3d.F3D;

import java.util.List;

public class Animation {

    public Animation(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Load files in the scene at provided time value if they contain any animation.
     *
     * @param timeValue time value to load
     * @return this animation for method chaining
     */
    public native Animation loadTime(double timeValue);

    /**
     * Get animation time range of files currently in the scene.
     *
     * @return array of 2 doubles [min_time, max_time]
     */
    public native double[] timeRange();

    /**
     * Get animation keyframe's time of files currently in the scene.
     *
     * @return list of double
     */
    public native double[] keyFrames();

    /**
     * Return the number of animations available in the files currently in the scene.
     *
     * @return number of available animations
     */
    public native int count();

    /**
     * Get the current animation name, if any.
     *
     * @return animation names or string error
     */
    public native String getCurrentName();

    /**
     * Get the animation name of a given animation index, if any.
     *
     * @param index animation index, -1 for current animation
     * @return animation name or string error
     */
    public native String getName(int index);

    /**
     * Get all of the animation names, if any.
     *
     * @return list of animation names
     */
    public native List<String> getNames();

    private long mNativeAddress;
}
