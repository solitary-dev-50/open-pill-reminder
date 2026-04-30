# Hardware Plan

Chinese original: [02_hardware_plan.md](./02_hardware_plan.md)

Current hardware direction:

- ESP32-C3 as the original low-cost target
- ESP32-S3 as the current bring-up and validation board
- 0.96-inch I2C OLED
- Active buzzer
- `OK` button
- `SET/BACK` button
- External LED or WS2812
- USB power

Current default S3 pin mapping in firmware:

- OLED SDA `GPIO8`
- OLED SCL `GPIO9`
- Buzzer `GPIO4`
- OK button `GPIO5`
- SET/BACK button `GPIO6`
- External LED `GPIO7`
- Onboard WS2812 `GPIO38`

The hardware remains simple and does not include battery management in version 0.1.

## Mechanical and Enclosure Constraints

- The PCB is intended to fit a `Raspberry Pi 4B` case
- The PCB outline should be designed with enclosure space in mind
- Mounting holes should align with the enclosure fixing points
- The `USB-C` connector position should align with the enclosure cutout
- The `OK` / `SET-BACK` button positions should align with enclosure button holes

These constraints directly affect:

- Board outline
- Component placement
- Connector orientation
- Button location
- Future enclosure and 3D fit work
