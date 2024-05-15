F3D can play animations for a number of file formats (.ex2/.e/.exo/.g, .gltf/.glb, .fbx, .dae, .x, .usd) if the file contains an animation.
## Demonstration
This specific example uses an animation file which can be downloaded [here](https://github.com/f3d-app/f3d/blob/606089959c9520085a9cbf70660fb0ffc68fb934/testing/data/InterpolationTest.glb).

<img width="305" alt="Screenshot 2024-05-07 at 3 25 14 PM" src="https://github.com/f3d-app/f3d/assets/2395780/9b92e833-d497-4fca-a75c-ce651ae973f6">

Load the example animation file shown above by executing within command line: `f3d InterpolationTest.glb`

<img width="1105" alt="Screenshot 2024-05-07 at 1 16 37 PM" src="https://github.com/f3d-app/f3d/assets/2395780/2668544e-7744-4adc-8867-e83579e9c915">
To view current animation name, press <kbd>H</kbd> to open up cheatsheet menu

<img width="1079" alt="Screenshot 2024-05-07 at 4 03 08 PM" src="https://github.com/f3d-app/f3d/assets/2395780/8789180c-b470-441c-8395-c010a44941e6">
Press <kbd>W</kbd> to cycle through available animations

<img width="1112" alt="Screenshot 2024-05-07 at 3 59 44 PM" src="https://github.com/f3d-app/f3d/assets/2395780/073c4670-1db6-4411-b571-85c41be93039">
Press <kbd>space</kbd> to play/pause current animation.
Note: A red bar runs along the bottom of screen to indicate the current time interval of the animation sequence.

<img width="1108" alt="Screenshot 2024-05-07 at 3 48 15 PM" src="https://github.com/f3d-app/f3d/assets/2395780/5f2ccff1-7c71-44eb-95dc-38d67161e247">
"All Animations" will play all animations at the same time.

## Command line options

F3D animation behavior can be fully controlled from the command line using the following options.

| Options                      | Default             | Description                                    |
| ---------------------------- | ------------------- | ---------------------------------------------- |
| \-\-animation\-index         |                     | Select the animation to play.                  |
| \-\-animation\-index=-1      |                     | Play all animations at once (.gltf/.glb only). |
| \-\-animation\-speed\-factor | Time Unit = Seconds | Adjust time unit.                              |
| \-\-animation\-frame\-rate   | 60 FPS              | Adjust animation frame rate.                   |
## Animation Interactions
- Press <kbd>W</kbd> to cycle through animations
- Press <kbd>Space</kbd> to play/pause animation
## Time Units
- When F3D plays an animation, it assumes the time unit is in seconds to show accurate speed of animation.