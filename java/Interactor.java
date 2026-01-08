package app.f3d.F3D;

import java.util.List;

public class Interactor {

    private long mNativeAddress;

    public enum ModifierKeys {
        ANY,
        NONE,
        CTRL,
        SHIFT,
        CTRL_SHIFT
    }

    public enum BindingType {
        CYCLIC,
        NUMERICAL,
        TOGGLE,
        OTHER
    }

    public enum MouseButton {
        LEFT,
        RIGHT,
        MIDDLE
    }

    public enum WheelDirection {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    }

    public enum InputAction {
        PRESS,
        RELEASE
    }

    public enum InputModifier {
        NONE,
        CTRL,
        SHIFT,
        CTRL_SHIFT
    }

    public enum AnimationDirection {
        FORWARD(0),
        BACKWARD(1);

        private final int value;

        AnimationDirection(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static AnimationDirection fromValue(int value) {
            for (AnimationDirection dir : AnimationDirection.values()) {
                if (dir.value == value) {
                    return dir;
                }
            }
            throw new IllegalArgumentException("Invalid AnimationDirection value: " + value);
        }
    }

    public static class InteractionBind implements Comparable<InteractionBind> {
        public ModifierKeys mod;
        public String inter;

        public InteractionBind() {
            this.mod = ModifierKeys.NONE;
            this.inter = "";
        }

        public InteractionBind(ModifierKeys mod, String inter) {
            this.mod = mod;
            this.inter = inter;
        }

        public String format() {
            switch (mod) {
                case CTRL_SHIFT:
                    return "Ctrl+Shift+" + inter;
                case CTRL:
                    return "Ctrl+" + inter;
                case SHIFT:
                    return "Shift+" + inter;
                case ANY:
                    return "Any+" + inter;
                default:
                    return inter;
            }
        }

        public static InteractionBind parse(String str) {
            InteractionBind bind = new InteractionBind();
            int plusIndex = str.lastIndexOf('+');
            if (plusIndex == -1) {
                bind.inter = str;
            } else {
                bind.inter = str.substring(plusIndex + 1);
                String modStr = str.substring(0, plusIndex);
                if (modStr.equals("Ctrl+Shift")) {
                    bind.mod = ModifierKeys.CTRL_SHIFT;
                } else if (modStr.equals("Shift")) {
                    bind.mod = ModifierKeys.SHIFT;
                } else if (modStr.equals("Ctrl")) {
                    bind.mod = ModifierKeys.CTRL;
                } else if (modStr.equals("Any")) {
                    bind.mod = ModifierKeys.ANY;
                } else if (modStr.equals("None")) {
                    bind.mod = ModifierKeys.NONE;
                }
            }
            return bind;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null || getClass() != obj.getClass()) return false;
            InteractionBind other = (InteractionBind) obj;
            return mod == other.mod && inter.equals(other.inter);
        }

        @Override
        public int compareTo(InteractionBind other) {
            if (mod != other.mod) {
                return mod.compareTo(other.mod);
            }
            return inter.compareTo(other.inter);
        }
    }

    public static class BindingDocumentation {
        public String documentation;
        public String currentValue;

        public BindingDocumentation(String documentation, String currentValue) {
            this.documentation = documentation;
            this.currentValue = currentValue;
        }
    }

    public interface CommandCallback {
        void execute(List<String> args);
    }

    Interactor(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Remove all existing commands and add all default commands.
     *
     * @return this interactor for method chaining
     */
    public native Interactor initCommands();

    /**
     * Add a command to be called using triggerCommand.
     *
     * @param action action name for the command
     * @param callback callback to execute when command is triggered
     * @return this interactor for method chaining
     */
    public native Interactor addCommand(String action, CommandCallback callback);

    /**
     * Remove a command for provided action.
     *
     * @param action action name to remove
     * @return this interactor for method chaining
     */
    public native Interactor removeCommand(String action);

    /**
     * Return a list containing all currently defined actions of commands.
     *
     * @return list of command actions
     */
    public native List<String> getCommandActions();

    /**
     * Trigger provided command.
     *
     * @param command command string to execute
     * @param keepComments if true, comments with # are supported
     * @return true if command succeeded, false otherwise
     */
    public native boolean triggerCommand(String command, boolean keepComments);

    /**
     * Trigger provided command with keepComments enabled.
     *
     * @param command command string to execute
     * @return true if command succeeded, false otherwise
     */
    public boolean triggerCommand(String command) {
        return triggerCommand(command, true);
    }

    /**
     * Remove all existing interaction commands and add all default bindings.
     *
     * @return this interactor for method chaining
     */
    public native Interactor initBindings();

    /**
     * Add binding to trigger commands for a specific bind.
     *
     * @param bind interaction bind (key combination)
     * @param commands list of commands to trigger
     * @param group optional group name for organization
     * @param type optional binding type
     * @return this interactor for method chaining
     */
    private native Interactor addBindingCommands(InteractionBind bind, List<String> commands, String group, BindingType type);

    /**
     * Add binding to trigger commands for a specific bind.
     *
     * @param bind interaction bind (key combination)
     * @param commands list of commands to trigger
     * @param group optional group name for organization
     * @param type optional binding type
     * @return this interactor for method chaining
     */
    public Interactor addBinding(InteractionBind bind, List<String> commands, String group, BindingType type) {
        return addBindingCommands(bind, commands, group, type);
    }

    /**
     * Add binding to trigger commands for a specific bind with default group and type.
     *
     * @param bind interaction bind (key combination)
     * @param commands list of commands to trigger
     * @return this interactor for method chaining
     */
    public Interactor addBinding(InteractionBind bind, List<String> commands) {
        return addBindingCommands(bind, commands, "", BindingType.OTHER);
    }

    /**
     * Add binding to trigger a single command for a specific bind.
     *
     * @param bind interaction bind (key combination)
     * @param command command to trigger
     * @param group optional group name for organization
     * @param type optional binding type
     * @return this interactor for method chaining
     */
    private native Interactor addBindingCommand(InteractionBind bind, String command, String group, BindingType type);

    /**
     * Add binding to trigger a single command for a specific bind.
     *
     * @param bind interaction bind (key combination)
     * @param command command to trigger
     * @param group optional group name for organization
     * @param type optional binding type
     * @return this interactor for method chaining
     */
    public Interactor addBinding(InteractionBind bind, String command, String group, BindingType type) {
        return addBindingCommand(bind, command, group, type);
    }

    /**
     * Add binding to trigger a single command for a specific bind with default group and type.
     *
     * @param bind interaction bind (key combination)
     * @param command command to trigger
     * @return this interactor for method chaining
     */
    public Interactor addBinding(InteractionBind bind, String command) {
        return addBindingCommand(bind, command, "", BindingType.OTHER);
    }

    /**
     * Remove binding corresponding to provided bind.
     *
     * @param bind binding to remove
     * @return this interactor for method chaining
     */
    public native Interactor removeBinding(InteractionBind bind);

    /**
     * Return a vector of available bind groups, in order of addition.
     *
     * @return list of bind group names
     */
    public native List<String> getBindGroups();

    /**
     * Return a vector of bind for the specified group, in order of addition.
     *
     * @param group group name
     * @return list of binds for the group
     */
    public native List<InteractionBind> getBindsForGroup(String group);

    /**
     * Return a vector of all binds, in order of addition.
     *
     * @return list of all binds
     */
    public native List<InteractionBind> getBinds();

    /**
     * Get documentation for a binding.
     *
     * @param bind binding to get documentation for
     * @return documentation pair (description, current value)
     */
    public native BindingDocumentation getBindingDocumentation(InteractionBind bind);

    /**
     * Get the type of a binding.
     *
     * @param bind binding to get type for
     * @return binding type
     */
    public native BindingType getBindingType(InteractionBind bind);

    /**
     * Toggle animation state.
     *
     * @param direction animation direction
     * @return this interactor for method chaining
     */
    public native Interactor toggleAnimation(AnimationDirection direction);

    /**
     * Toggle animation state with default forward direction.
     *
     * @return this interactor for method chaining
     */
    public Interactor toggleAnimation() {
        return toggleAnimation(AnimationDirection.FORWARD);
    }

    /**
     * Start animation.
     *
     * @param direction animation direction
     * @return this interactor for method chaining
     */
    public native Interactor startAnimation(AnimationDirection direction);

    /**
     * Start animation with default forward direction.
     *
     * @return this interactor for method chaining
     */
    public Interactor startAnimation() {
        return startAnimation(AnimationDirection.FORWARD);
    }

    /**
     * Stop animation.
     *
     * @return this interactor for method chaining
     */
    public native Interactor stopAnimation();

    /**
     * Check if animation is playing.
     *
     * @return true if playing, false otherwise
     */
    public native boolean isPlayingAnimation();

    /**
     * Get the current animation direction.
     *
     * @return animation direction
     */
    public native AnimationDirection getAnimationDirection();

    /**
     * Enable camera movement.
     *
     * @return this interactor for method chaining
     */
    public native Interactor enableCameraMovement();

    /**
     * Disable camera movement.
     *
     * @return this interactor for method chaining
     */
    public native Interactor disableCameraMovement();

    /**
     * Trigger a modifier update.
     *
     * @param mod modifier state
     * @return this interactor for method chaining
     */
    public native Interactor triggerModUpdate(InputModifier mod);

    /**
     * Trigger a mouse button event.
     *
     * @param action press or release action
     * @param button mouse button
     * @return this interactor for method chaining
     */
    public native Interactor triggerMouseButton(InputAction action, MouseButton button);

    /**
     * Trigger a mouse new position event.
     *
     * @param xpos x position in pixels
     * @param ypos y position in pixels
     * @return this interactor for method chaining
     */
    public native Interactor triggerMousePosition(double xpos, double ypos);

    /**
     * Trigger a mouse wheel event.
     *
     * @param direction wheel direction
     * @return this interactor for method chaining
     */
    public native Interactor triggerMouseWheel(WheelDirection direction);

    /**
     * Trigger a keyboard key event.
     *
     * @param action press or release action
     * @param keySym key symbol (e.g., "A", "Left", "Space")
     * @return this interactor for method chaining
     */
    public native Interactor triggerKeyboardKey(InputAction action, String keySym);

    /**
     * Trigger a text character input event.
     *
     * @param codepoint Unicode codepoint
     * @return this interactor for method chaining
     */
    public native Interactor triggerTextCharacter(int codepoint);

    /**
     * Manually trigger the event loop.
     *
     * @param deltaTime time delta in seconds (must be positive)
     * @return this interactor for method chaining
     */
    public native Interactor triggerEventLoop(double deltaTime);

    /**
     * Play a VTK interaction file.
     *
     * @param file path to interaction file
     * @param deltaTime time delta in seconds
     * @return true on success, false otherwise
     */
    public native boolean playInteraction(String file, double deltaTime);

    /**
     * Play a VTK interaction file with default deltaTime.
     *
     * @param file path to interaction file
     * @return true on success, false otherwise
     */
    public boolean playInteraction(String file) {
        return playInteraction(file, 1.0 / 30);
    }

    /**
     * Start interaction and record it in a VTK interaction file.
     *
     * @param file path to save interaction file
     * @return true on success, false otherwise
     */
    public native boolean recordInteraction(String file);

    /**
     * Start the interactor event loop.
     *
     * @param deltaTime time delta in seconds (must be positive)
     * @return this interactor for method chaining
     */
    public native Interactor start(double deltaTime);

    /**
     * Start the interactor event loop with default deltaTime.
     *
     * @return this interactor for method chaining
     */
    public Interactor start() {
        return start(1.0 / 30);
    }

    /**
     * Start the interactor event loop with a callback.
     * The callback is called on each event loop iteration.
     *
     * @param deltaTime time delta in seconds (must be positive)
     * @param callback callback to execute on each iteration
     * @return this interactor for method chaining
     */
    private native Interactor startWithCallback(double deltaTime, Runnable callback);

    /**
     * Start the interactor event loop with a callback.
     * The callback is called on each event loop iteration.
     *
     * @param deltaTime time delta in seconds (must be positive)
     * @param callback callback to execute on each iteration
     * @return this interactor for method chaining
     */
    public Interactor start(double deltaTime, Runnable callback) {
        return startWithCallback(deltaTime, callback);
    }

    /**
     * Stop the interactor.
     *
     * @return this interactor for method chaining
     */
    public native Interactor stop();

    /**
     * Request a render to be done on the next event loop.
     *
     * @return this interactor for method chaining
     */
    public native Interactor requestRender();

    /**
     * Request the interactor to stop on the next event loop.
     *
     * @return this interactor for method chaining
     */
    public native Interactor requestStop();
}
