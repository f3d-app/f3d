# Animations

F3D can play animations from files that contain them.
Play animations either interactively, by selecting a specific time to display, or by selecting which animations to play.
When a file contains multiple animations, either play each animation separately or select multiple animations to play at the same time.

## Demonstration

This specific example uses an animation file available [here](https://github.com/f3d-app/f3d/blob/606089959c9520085a9cbf70660fb0ffc68fb934/testing/data/InterpolationTest.glb).

<img width="1024" alt="1" src="https://media.githubusercontent.com/media/f3d-app/f3d-website/refs/heads/main/docs/user/animation_0.png" />

Load the example animation file from the command line: `f3d InterpolationTest.glb`

<img width="1024" alt="2" src="https://media.githubusercontent.com/media/f3d-app/f3d-website/refs/heads/main/docs/user/animation_1.png" />
To view the current animation name, press <kbd>H</kbd> to open the Cheatsheet.

<img width="1024" alt="3" src="https://media.githubusercontent.com/media/f3d-app/f3d-website/refs/heads/main/docs/user/animation_2.png" />
Press <kbd>W</kbd> to cycle through available animations.

<img width="1024" alt="4" src="https://media.githubusercontent.com/media/f3d-app/f3d-website/refs/heads/main/docs/user/animation_3.png" />
Press <kbd>Space</kbd> to play or pause the current animation.
Note: A blue bar runs along the bottom of screen to indicate the current time interval of the animation sequence if `--animation-progress` is set to `default` or `advanced`. The `advanced` mode additionally shows the time range, animation name and current time, and marks each keyframe with a vertical line on the bar; both modes let you click or drag the bar to jump to a given time. While hovering the `advanced` bar near a keyframe marker, the tooltip reports that keyframe's time.

<img width="1024" alt="5" src="https://media.githubusercontent.com/media/f3d-app/f3d-website/refs/heads/main/docs/user/animation_4.png" />
"All animations" plays all animations at the same time when the file format supports it.

## Command-line options

Control F3D animation behavior from the command line with these options.

| Options                      | Default             | Description                                          |
| ---------------------------- | ------------------- | ---------------------------------------------------- |
| \-\-animation\-indices       |                     | Select the animations to play.                       |
| \-\-animation\-indices=-1    |                     | Play all animations at once (only if supported)      |
| \-\-animation\-speed\-factor | Time Unit = Seconds | Adjust time unit.                                    |
| \-\-frame\-rate              | 60 FPS              | Adjust animation (and others components) frame rate. |
| \-\-animation\-time          |                     | Load a specific time value on start.                 |

## Export animation frames

F3D can export multiple frames from an animation to image files. Include `{frame}` in the output filename template:

```bash
f3d example.file --output=frame_{frame:04}.png
```

This saves frames as `frame_0000.png`, `frame_0001.png`, etc.

Set the number of frames with `--frame-rate`:

```bash
f3d example.file --output=frame_{frame}.png --frame-rate=30
```

Use `--animation-time` to start exporting from a specific time instead of the beginning:

```bash
f3d example.file --output=frame_{frame}.png --frame-rate=10 --animation-time=1.5
```

See [Filename templating](03-OPTIONS.md#filename-templating) for more template variables.

## Animation interactions

- Press <kbd>W</kbd> to cycle through animations.
- Press <kbd>Space</kbd> to play or pause animation.
- Press <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Space</kbd> to play or pause animation backward.

See [COMMANDS](07-COMMANDS.md) for commands like `jump_to_frame`, `jump_to_keyframe` and `jump_to_time`.

## Cycle animations

Press <kbd>W</kbd> to cycle the animation to show.

When cycling, F3D uses this order:

- Multiple animations or all animations
- Animation index 0
- Animation index 1
- ...
- Animation index N
- All animations (if multiple animations supported and more than one animation is available)
- No animations

If you selected multiple animation indices, F3D does not cycle back to that selection.
F3D cycles all animations only when the currently loaded files support it.

## Time units

F3D assumes animation time uses seconds.

## Coloring range

When coloring while loading a time value or playing the animation, F3D automatically expands the coloring range from previously loaded time values.
This includes `--animation-time`, which first loads the initial time before loading the provided time value.

## Animation support level

The supported file formats page lists the animation support level for each reader [here](02-SUPPORTED_FORMATS.md).

- NONE: Animation is not supported, either by the file format or the implementation.
- UNIQUE: No more than one animation exists.
- SINGLE: Multiple animations can exist, but F3D selects only one at a time.
- MULTI: F3D can select multiple animations at a time.

When opening multiple files at the same time using the `--multi-file-mode` option and selecting multiple animations to show,
F3D may warn when combining SINGLE and non SINGLE files, but it still tries to respect the chosen animation indices.

## Animation keyframes

Use the `jump_to_keyframe` command to load an animation at a specific keyframe. See [COMMANDS](07-COMMANDS.md) for more information and examples on how to use this command.

This command is currently supported only by the following readers:

- `vtkF3DGLTFImporter`
- `vtkF3DQuakeMDLImporter`

Follow the issue below to track the progress of animation support for other readers:
[F3D Issue – Improve Animation System #2637](https://github.com/f3d-app/f3d/issues/2637#:~:text=Access%20to%20timesteps)
