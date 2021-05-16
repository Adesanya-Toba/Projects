# BIOMETRIC ATTENDANCE DEVICE
Jun 2018 – Sep 2018

This was a really cool and tasking school project I worked on with a couple of friends, developing a digitized means of registering for courses and taking class attendance, for students and teachers.\
I was in charge of developing the firmware for the project.

The project consisted of: 
- a TFT LCD touchscreen,
- a Fingerprint scanner, 
- an External Memory card and
- 2 Atmel Atmega328 microcontrollers.

We decided to use two microcontrollers because of additonal pin count for the LCD screen and access to specific I/O buses.

Essentially, the two microcontrollers communicated with one another over UART (Serial communication) using a _"throw and catch"_ protocol to ensure the data authenticity between sender and receiver.