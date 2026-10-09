package app.f3d.F3D;

import java.util.List;

public class Animation {

    public Animation(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public enum Direction {
        FORWARD(0),
        BACKWARD(1);

        private final int value;

        Direction(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static Direction fromValue(int value) {
            for (Direction dir : Direction.values()) {
                if (dir.value == value) {
                    return dir;
                }
            }
            throw new IllegalArgumentException("Invalid Direction value: " + value);
        }
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
    public native double[] getTimeRange();

    /**
     * Get animation keyframe's time of files currently in the scene.
     *
     * @return list of double
     */
    public native double[] getKeyFrames();

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
     * @param index animation index
     * @return animation name or string error
     */
    public native String getName(int index);

    /**
     * Get all of the animation names, if any.
     *
     * @return list of animation names
     */
    public native List<String> getNames();

    /**
     * Toggle animation state.
     *
     * @param direction animation direction
     * @return this interactor for method chaining
     */
    public native Interactor toggle(Direction direction);

    /**
     * Toggle animation state with default forward direction.
     *
     * @return this interactor for method chaining
     */
    public Interactor toggle() {
        return toggle(Direction.FORWARD);
    }

    /**
     * Start animation.
     *
     * @param direction animation direction
     * @return this interactor for method chaining
     */
    public native Interactor start(Direction direction);

    /**
     * Start animation with default forward direction.
     *
     * @return this interactor for method chaining
     */
    public Interactor start() {
        return start(Direction.FORWARD);
    }

    /**
     * Stop animation.
     *
     * @return this interactor for method chaining
     */
    public native Interactor stop();

    /**
     * Check if animation is playing.
     *
     * @return true if playing, false otherwise
     */
    public native boolean isPlaying();

    /**
     * Get the current animation direction.
     *
     * @return animation direction
     */
    public native Direction getDirection();

    private long mNativeAddress;
}
