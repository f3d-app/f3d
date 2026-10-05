package app.f3d.F3D;

import java.util.List;

public class Scene {

    /** Thrown when a file or mesh cannot be loaded into the scene. */
    public static class LoadFailureException extends F3DException {
        public LoadFailureException(String message) { super(message); }
    }

    /** Thrown when a light operation fails (e.g. invalid index). */
    public static class LightException extends F3DException {
        public LightException(String message) { super(message); }
    }

    /** Thrown when a scene hierarchy node operation fails (e.g. invalid index). */
    public static class NodeException extends F3DException {
        public NodeException(String message) { super(message); }
    }

    /**
     * Enumeration of file availability levels.
     */
    public enum FileAvailability {
        SUPPORTED(0),
        UNSUPPORTED_EXTENSION(1),
        UNSUPPORTED_CONTENT(2);

        private final int value;

        FileAvailability(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static FileAvailability fromValue(int value) {
            for (FileAvailability availability : FileAvailability.values()) {
                if (availability.value == value) {
                    return availability;
                }
            }
            throw new IllegalArgumentException("Invalid FileAvailability value: " + value);
        }
    }

    public Scene(long nativeAddress) {
        mNativeAddress = nativeAddress;
        mAnimation = new Animation(mNativeAddress);
    }

    /**
     * Add and load a file into the scene.
     *
     * @param filePath file path to add
     * @return this scene for method chaining
     */
    public native Scene add(String filePath);

    private native Scene addAll(List<String> filePaths);

    /**
     * Add and load multiple files into the scene.
     *
     * @param filePaths list of file paths to add
     * @return this scene for method chaining
     */
    public Scene add(List<String> filePaths)
    {
        return this.addAll(filePaths);
    }

    private native Scene addMesh(Types.Mesh mesh);

    /**
     * Add and load a mesh into the scene.
     *
     * @param mesh mesh to add
     * @return this scene for method chaining
     */
    public Scene add(Types.Mesh mesh)
    {
        return this.addMesh(mesh);
    }

    private native Scene addBuffer(byte[] buffer);

    /**
     * Add and load a buffer containing a file into the scene.
     *
     * @param buffer Memory buffer to load
     * @return this scene for method chaining
     */
    public Scene add(byte[] buffer)
    {
        return this.addBuffer(buffer);
    }

    /**
     * Clear the scene of all added files.
     *
     * @return this scene for method chaining
     */
    public native Scene clear();

    /**
     * Get the list of files currently added to the scene.
     *
     * @return list of added file paths
     */
    public native List<String> getAddedFiles();

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
     * Get the scene hierarchy of all added files, in depth-first pre-order, so that a parent
     * node always precedes its children.
     *
     * @return the list of scene hierarchy nodes
     */
    public native List<Types.NodeState> getSceneHierarchy();

    /**
     * Set the visibility of a scene hierarchy node and of all the nodes in its subtree.
     *
     * @param nodeId index of the node
     * @param visible visibility to set
     * @return this scene for method chaining
     */
    public native Scene setNodeVisibility(int nodeId, boolean visible);

    /**
     * Get information about the contents of the scene, eg. its number of points and cells.
     * All the counters are zero when the scene is empty.
     *
     * @return the scene information
     */
    public native Types.SceneInfo getSceneInfo();

    /**
     * Check if a file path is supported by the scene.
     *
     * @param filePath file path to check
     * @throws IllegalArgumentException if filePath is null
     * @return file availability
     */
    public native FileAvailability supports(String filePath);

    private native FileAvailability supportsBuffer(byte[] buffer);

    /**
     * Check if a memory buffer is supported by the scene.
     *
     * @param buffer memory buffer to check
     * @throws IllegalArgumentException if buffer is null
     * @return file availability
     */
    public FileAvailability supports(byte[] buffer)
    {
        if (buffer == null) {
            throw new IllegalArgumentException("buffer must not be null");
        }
        return this.supportsBuffer(buffer);
    }

    /**
     * Get the animation
     * @return Animation instance
     */
    public Animation getAnimation() { return mAnimation; }

    private long mNativeAddress;
    private Animation mAnimation;
}
