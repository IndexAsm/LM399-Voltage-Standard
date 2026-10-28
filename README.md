# LM399 10V Voltage Standard

10V Voltage Standard used to calibrate equipment.

<img width="713" height="715" alt="obraz" src="https://github.com/user-attachments/assets/3a56e653-adb8-4d4c-9de5-61b401437008" />


## Functions
  - Uptime tracking
  - Temperature tracking
  - Stores calibration data
  - USB connection
  - Status LED
  - Voltage & Current monitoring

## Specifications
  - Calibrated 10 V reference output with ppm-level stability target
  - Low tempco (I am aiming for < 5ppm but that requires testing)
  - Low long term drift ( Also requires testing, in order to determine the value)

## Calibration
  - Short pins on CAL_EN header.
  - Measure the output and adjust the binary trim network accordingly.
  - Connect it to a computer and write changes to eeprom (a special program will be created after constructing the first prototype).
  - Open CAL_EN header.
  - Done!

## Reference
LM399 is chosen as the main reference, which is later amplified to precise 10V with LT1001. It uses precise Z Foil resistors for gain.
It has an internal heater to keep it at stable temperature, in order to prevent output drift.


## PCB
I decided to go with a 4 layer board. This way I can make L2 a solid ground and L3 power. It not only simplifies routing but also improves performance
There is a little space left empty which also has a purpose. This way I can keep digital and power electronics away from sensitive reference part. Voltage regulator can heat up. If they were close to precision section, the trim network would heat up, which would result in output drift.

The reference section has an exposed copper area on the first layer to attach aluminum shielding.

Also there are a few text fields to input measurements, when the device is first calibrated.


<img width="743" height="727" alt="obraz" src="https://github.com/user-attachments/assets/2b4be532-8f04-475a-87cf-915e5dc6fe1f" />

## Gallery
<img width="1542" height="2047" alt="866519307_2180052555939868_5182472449521369803_n(1)" src="https://github.com/user-attachments/assets/ac208975-fe61-4998-b505-f6458c308ef3" />

<img width="1542" height="2047" alt="876545869_992198979977593_2324098559912250930_n" src="https://github.com/user-attachments/assets/7747ef9a-f7b2-4d48-834a-22c0f7a6b1d5" />

<img width="2048" height="1542" alt="870524854_1154811696968642_4580220781798994946_n" src="https://github.com/user-attachments/assets/b44b0fea-5713-4082-a1db-484bcf827af9" />


<img width="576" height="1024" alt="AQPV5MHPnuaBnd2dGWVxHe5OO-5tEl-Y2_EjqWZW3jtNE19Zy_EjqBOB38XeHsCUpxiR-6BePGkumeMqMDvoQD9K2QbnkCG4WUnZwBHdlQ" src="https://github.com/user-attachments/assets/faf332d7-9911-4009-851a-ffea7493aaf0" />







## Software
I used Kicad to design this board.

## [BOM](LM399VoltageStandard.csv)
Total cost: 473,59zł + shipping
That is $127.10 USD
Some parts are left with no price or link. That means I already have them so I did not include them in price calculation.

## Credits
I relied on MarcoReps's videos to learn everything in order to make this project.
