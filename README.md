# Water-Rocket-Avionics-and-Recovery
Design for a mechanical parachute deployment system by developing a teensy-based flight computer capable of logging altitude and IMU data, and apogee detection.

Background: 
Recovery system in a rocket is crucial for reusability, and also safety of the people and properties on scene. Traditional model rocket uses black powder charges to deploy a parachute, or simply letting it fall out as the rocket descend and points downward. Both of these method comes with their own drawback such as legality/availability of black powder charges, and the unreliable nature of using an analog system for recovery. By using a mechanical system that uses a servo release mechanism, controlled by a flight computer, we should expect a more reliable and safe method of recovering model rocket.

Payload fairing: 
The main parachute will be housed in a 3D printable clamshell payload fairing that is secured by a servo. Upon reaching apogee, the servo releases. The spring that were pushing against the fairing will jettison them away, releasing the parachute. 

Avionics:
The flight computer will be based on a teensy 3.5, with BME280 as the main altimeter, MPU6050 as the main IMU, SD card for datalogging, servo output pins, reverse polarity protection, RGB indicator light, push button, possibly a breakwire circuit as an extra launch detection method for redundancy. 

Sequence of events:
Pad idle:
1. MCU checks whether all sensors are working, SD card pressent, and file creation contain no error. 
2. Red light if error, stop everything.
3. Green light if everything is nominal, proceed to next step.
4. Servo Opens and Closes smoothly, and that the fairing is secured properly.
5. Start datalogging.

Launch:
1. Flight computer detects launch either using IMU or breakwire.
2. Data logging continues.
3. Barometer detects apogee.
4. Servo is opened to allow fairing to seperate.
5. Parachute deployed.

Descend and touchdown:
1. Data logging continues.
2. LED and Buzzer blinking and beeping as IMU detects touchdown.
3. If button is pushed, data logging stops and saves on to SD card.
