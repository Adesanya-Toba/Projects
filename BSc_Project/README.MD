# Dual Axis Solar tracking with IoT
Sep 2019 – Feb 2020\
This was a really amazing project I enjoyed building because it merged two of main interests perfectly: _Climate and Technology_.


## Project description: 
This project was built for the defense of my BSc thesis where I investigated the potential benefits of dual axis solar tracking in solar farm installations (>500 MW).

The project comprised of **two ESP-32** microcontrollers communicating wirelessly, sending control and data information between one another, over a peer-to-peer network.\
A Master-Slave configuration was adopted for this project, where the master had sensors to track the sun's position and the slave did not have these sensors.
This demonstrated the scalability of the project as more slave modules could be added to increase the network at a lower cost (i.e. no need for expensive sensors on slaves).

Every 5 seconds, the master module would update the slave module with the current solar position, instructing it to move it's motors to this new position.
This is all occurred as the system continuously uploaded data on system parameters such as voltage, current, X-position, Y-position, etc., via MQTT over WiFi to my ThingSpeak dashboard.

The project data showed a **20.9%** increase in the energy generated over time, when compared with conventional non-tracking solar farms while providing a dramatic increase in the ease of monitoring such systems over the internet.