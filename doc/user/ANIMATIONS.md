#Animations

F3D is able to play animations for any files which contain them.
Play them either interactively or by selecting a specific time to display.
For files containing multiple animations, F3D allows the user to either play each animation separately or play all animations at the same time.
## Demonstration
This specific example uses an animation file which can be downloaded [here](https://github.com/f3d-app/f3d/blob/606089959c9520085a9cbf70660fb0ffc68fb934/testing/data/InterpolationTest.glb).

<img width="1024" alt="1" src="https://github.com/f3d-app/f3d/assets/2395780/1402c513-b3fd-454b-84a4-738eb41a1ccf">

Load the example animation file shown above by executing within command line: `f3d InterpolationTest.glb`

<img width="1024" alt="2" src="https://github.com/f3d-app/f3d/assets/2395780/eb3fbb74-5ccd-482f-9de7-92017285c73a">
To view current animation name, press <kbd>H</kbd> to open up cheatsheet menu

<img width="1024" alt="3" src="https://github.com/f3d-app/f3d/assets/2395780/5eedaaf5-a355-4742-b2f4-bdae27054f17">
Press <kbd>W</kbd> to cycle through available animations

<img width="1024" alt="4" src="https://github.com/f3d-app/f3d/assets/2395780/446e9d31-fc95-4b99-81b0-e54ca8ef1998">
Press <kbd>space</kbd> to play/pause current animation.
Note: A red bar runs along the bottom of screen to indicate the current time interval of the animation sequence.

<img width="1024" alt="5" src="https://github.com/f3d-app/f3d/assets/2395780/59e1e8ae-78e6-4edc-9c87-c7b06d3bba5f">
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