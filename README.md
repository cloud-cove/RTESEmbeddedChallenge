# RTESEmbeddedChallenge
Embedded Challenge F23

## When running

All files are included, just upload and run.

When testing, please hold the board against your shoulder or hip joint (use an armband if you have one) with the screen facing outwards

On the touch screen, use the up and down arrows to set the height to the height of your hips, in inches (default value is 36 inches), then press \[CONFIRM\]

## How it works

We approximated the movement of the leg to be a swinging pendulum (and approximated the arms to be a similar movement as the leg).

Angular velocity * length of leg = linear velocity
linear velocity * time = distance traveled over that time.

We collect data from the gyroscope every 250ms, and calculate the distance traveled between each measurement. They are summed every twenty seconds for the distance traveled over twenty seconds. There is also a running totgal updated every twenty seconds

