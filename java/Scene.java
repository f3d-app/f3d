package app.f3d.F3D;

import java.util.List;

public class Scene {

    public Scene(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Add and load a file into the scene.
     *
     * @param filePath file path to add
     * @return this scene for method chaining
     */
    public native Scene add(String filePath);

    /**
     * Add and load multiple files into the scene.
     *
     * @param filePaths list of file paths to add
     * @return this scene for method chaining
     */
    public native Scene addAll(List<String> filePaths);

    /**
     * Add and load a mesh into the scene.
     *
     * @param mesh mesh to add
     * @return this scene for method chaining
     */
    public native Scene addMesh(Types.Mesh mesh);

    /**
     * Clear the scene of all added files.
     *
     * @return this scene for method chaining
     */
    public native Scene clear();

    /**
     * Add a light based on a light state.
     *
     * @param lightState light state
     * @return index of the added light
     */
    public native int addLight(Types.LightState lightState);

    /**
     * Get the number of lights.
     *
     * @return number of lights in the scene
     */
    public native int getLightCount();

    /**
     * Get the light state at provided index.
     *
     * @param index index of the light
     * @return light state
     */
    public native Types.LightState getLight(int index);

    /**
     * Update a light at provided index with the provided light state.
     *
     * @param index index of the light to update
     * @param lightState new light state
     * @return this scene for method chaining
     */
    public native Scene updateLight(int index, Types.LightState lightState);

    /**
     * Remove a light at provided index.
     *
     * @param index index of the light to remove
     * @return this scene for method chaining
     */
    public native Scene removeLight(int index);

    /**
     * Remove all lights from the scene.
     *
     * @return this scene for method chaining
     */
    public native Scene removeAllLights();

    /**
     * Check if a file path is supported by the scene.
     *
     * @param filePath file path to check
     * @return true if supported, false otherwise
     */
    public native boolean supports(String filePath);

    /**
     * Load added files at provided time value if they contain any animation.
     *
     * @param timeValue time value to load
     * @return this scene for method chaining
     */
    public native Scene loadAnimationTime(double timeValue);

    /**
     * Get animation time range of currently added files.
     *
     * @return array of 2 doubles [min_time, max_time]
     */
    public native double[] animationTimeRange();

    /**
     * Return the number of animations available in the currently loaded files.
     *
     * @return number of available animations
     */
    public native int availableAnimations();

    private long mNativeAddress;
}
