# Water-Rocket-Avionics-and-Recovery
Design for a mechanical parachute deployment system by developing a teensy-based flight computer capable of logging altitude and IMU data, and apogee detection.

# Background: 

Recovery system in a rocket is crucial for reusability, and also safety of the people and properties on scene. Traditional model rocket uses black powder charges to deploy a parachute, or simply letting it fall out as the rocket descend and points downward. Both of these method comes with their own drawback such as legality/availability of black powder charges, and the unreliable nature of using an analog system for recovery. By using a mechanical system that uses a servo release mechanism, controlled by a flight computer, we should expect a more reliable and safe method of recovering model rocket.

# Payload fairing: 

The main parachute will be housed in a 3D printable clamshell payload fairing that is secured by a servo. Upon reaching apogee, the servo releases. The spring that were pushing against the fairing will jettison them away, releasing the parachute. 

# Avionics:

The flight computer will be based on a teensy 4.1, with BME388 as the main altimeter, BNO055 as the main IMU, SD card for datalogging, servo output pins, reverse polarity protection, RGB indicator light, push button, possibly a breakwire circuit as an extra launch detection method for redundancy. 

# Project Breakdown:

![Screenshot 2022-10-06 231814](https://user-images.githubusercontent.com/92670627/194480652-db67a31d-d348-492e-95a4-ad474ac19b33.jpg)
