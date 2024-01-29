# RTESEmbeddedChallenge
Embedded Challenge F23:
"The objective of this semester’s embedded challenge is to build a wearable speedometer which can calculate velocity by measuring angular velocities available from our built-in gyroscope (L3GD20) - without a GPS. Our gyroscope is able to measure 3-axis angular velocity. Strategically placing the sensor on the legs or feet can capture the angular velocities and with a bit of processing can convert those angular velocities to linear forward velocity and calculate distance traveled (using only a gyro!)"

## When running
Project written for the STMF429 Discovery board

All files are included, just upload and run.

When testing, please hold the board against your shoulder or hip joint (use an armband if you have one) with the screen facing outwards

On the touch screen, use the up and down arrows to set the height to the height of your hips, in inches (default value is 36 inches), then press \[CONFIRM\]

## How it works

We approximated the movement of the leg to be a swinging pendulum (and approximated the arms to be a similar movement as the leg).

Angular velocity * length of leg = linear velocity
linear velocity * time = distance traveled over that time.

We collect data from the gyroscope every 250ms, and calculate the distance traveled between each measurement. They are summed every twenty seconds for the distance traveled over twenty seconds. There is also a running total updated every twenty seconds

## Demonstration

https://www.youtube.com/watch?v=GOwbfJdRKY4
