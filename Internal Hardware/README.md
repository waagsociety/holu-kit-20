# Hardware - holu-kit-20

<p align="center"> <img src="images/internal_hardware.jpg" width="500"></p>

Hollandse Luchten sensor kit, version 2.0.

In this folder you find the digital files for the pcbs production used to connect the different modules of the HoLu kit v2.

The Eagle files are the design files of the printed circuit boards. The basic HoLu kit includes:
- HL_sensors_pcb_all (schematic and layout)
- HL_Interface_pcb (schematic and layout)
- hl_bme280_out_pcb (schematic and layout)


The extended HoLu kit has all the modules mentioned above and also the following ones:
- HL_gas_no2_pcb (schematic and layout)
- HL_gas_o3_pcb (schematic and layout)

The Gerber files were submitted to the out-sourced service that produced the 200 HoLu kits boards.
The boards were produced by https://jlcpcb.com/.

The images below show the boards and the components that populate them. The components specifications can be found in the BOM

## Sensors pcb top view
### Basic

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_sensors_top_empty_photo.jpg" width="300"> | <img src="images/HL_sensors_top.png" width="300"> | <img src="images/HL_sensors_top_populated_photo.jpg" width="300"> |

Components:
- JP1
- JP2
- J_PM25
- J_TH_IN

### Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_sensors_top_empty_photo.jpg" width="300"> | <img src="images/HL_sensors_top.png" width="300"> | <img src="images/HL_sensors_top_ext_populated_photo.jpg" width="300"> |

Components:
- JP1
- JP2
- J_PM25
- J_TH_IN
- J_GAS_NO2
- J_GAS_O3

## Sensors pcb bottom view
### Basic

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_sensors_bottom_empty_photo.jpg" width="300"> | <img src="images/HL_sensors_bottom.png" width="300"> | <img src="images/HL_sensors_bottom_populated_photo.jpg" width="300"> |

### Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_sensors_top_empty_photo.jpg" width="300"> | <img src="images/HL_sensors_bottom.png" width="300"> | <img src="images/HL_sensors_bottom_ext_populated_photo.jpg" width="300"> |

Components:
- Q1, Q2
- R1, R2, R3, R4

### Interface pcb top view
## Basic and Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_interface_top_empty_photo.jpg" width="200"> | <img src="images/HL_interface_top.png" width="200"> | <img src="images/HL_interface_top_populated_photo.jpg" width="200"> |

Components:
- SW_RST
- SW_CONFIG
- R3, R4
- C1
- LED1

## Interface pcb bottom view
### Basic and Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_interface_bottom_empty_photo.jpg" width="200"> | <img src="images/HL_interface_bottom.png" width="200"> | <img src="images/HL_interface_bottom_populated_photo.jpg" width="200"> |

Components:
- JP_12
- JP_16

## BME pcb top view
### Basic and Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_bme_top_empty_photo.jpg" width="200"> | <img src="images/HL_bme_top.png" width="200"> | <img src="images/HL_bme_top_popluated_photo.jpg" width="200"> |

Components:
- J_TH

## BME pcb bottom view
### Basic and Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_bme_bottom_empty_photo.jpg" width="200"> | <img src="images/HL_bme_bottom.png" width="200"> | <img src="images/HL_bme_top_popluated_photo.jpg" width="200"> |

Components:
- J_G

## GAS pcbs top view
### Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_gas_pcb_top_empty_photo.jpg" width="200"> | <img src="images/HL_gas_o3_pcb_top.png" width="120"> <img src="images/HL_gas_no2_pcb_top.png" width="120"> | <img src="images/HL_gas_pcb_top_populated_photo.jpg" width="200"> |

Components:
- J_GAS
- J_ADS115

## GAS pcbs bottom view
### Extended

| pcbs  | component locations | populated pcb |
| ------------- | ------------- | ----------|
| <img src="images/HL_gas_pcb_bottom_empty_photo.jpg" width="200"> | <img src="images/HL_gas_o3_pcb_bottom.png" width="120">  <img src="images/HL_gas_no2_pcb_bottom.png" width="120"> | <img src="images/HL_gas_pcb_bottom_populated_photo.jpg" width="200"> |

Components:
- J_O3
- C1, C2

## EXTRA
### Internal plate
The HoLu kit uses an acrylic plate to hold the sensors and the electronic parts in place. The plates were cut in the Fablab at De Waag.
<p align="center"> <img src="images/internal_plate.jpg" width="400"></p>


### Feather M0 with RFM95 LORA Radio
<p align="center"> <img src="images/Feather_M0.jpg" width="200"></p>

It is important to buy the Feather M0 Lora with headers because the connectors that are usually provided attached to the board are too short for this application.
It is necessary to use the long staking headers, see BOM. It is also necessary to solder the coaxial connector for the antenna to the board.
You can find the instruction at this link: https://learn.adafruit.com/adafruit-feather-m0-radio-with-rfm69-packet-radio/antenna-options.


### Adalogger Feather
<p align="center"> <img src="images/adalogger.jpg" width="150"></p>

On this module it is necessary to solder the female connector and insert the battery and the SD-card, https://learn.adafruit.com/adafruit-adalogger-featherwing.


**NOTE**: During the on the field deployment of the HoLu kits, we observed that temperature sensor BME280 module is too fragile for such application and after a while many it tends to break. The Extended HoLu kit requires the temp/hum sensor for the calibration of the gas sensors so we adopted a different breakout board with the same sensor, BME280, see "link to issue web page".
