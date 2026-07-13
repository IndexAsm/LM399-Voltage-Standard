# LM399 10V Voltage Standard

## Overview
This project is a 10V Voltage Standard which I will use to calibrate my equipment.

## Functions
  - Uptime tracking
  - Temperature tracking
  - Stores calibration data
  - USB connection
  - Status LED
  - Voltage & Current monitoring

## Specifications
  - Stable 10.000000V output
  - Low tempco (I am aiming for < 5pp but that requires long term tests)
  - Low long term drift ( Also requires testing, in order to determine the value)



## Reference
LM399 is chosen as the main reference, which is later amplified to precise 10V with LT1001. It uses precise Z Foil resistors for gain.
It has an internal heater to keep it at stable temperature, in order to prevent output drift.


## PCB
I decided to go with a 4 layer board. This way I can make L2 a solid ground and L3 power. It not only simplifies routing but also improves performance
There is a little space left empty which also has a purpose. This way I can keep digital and power electronics away from sensitive reference part. Voltage regulator can heat up. If they were close to precision section, the trim network would heat up, which would result in output drift.

The reference section has an exposed copper area on the first layer to attach aluminum shielding.

Also there are a few text fields to input measurements, when the device is first calibrated.

<img width="819" height="691" alt="obraz" src="https://github.com/user-attachments/assets/f24fcce6-77ae-45a3-b81c-41e4847a5c61" />
<img width="596" height="598" alt="obraz" src="https://github.com/user-attachments/assets/25df28c9-238e-42e3-aba1-659fbdc19282" />
