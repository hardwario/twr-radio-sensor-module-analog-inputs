<a href="https://www.hardwario.com/"><img src="https://www.hardwario.com/ci/assets/hw-logo.svg" width="200" alt="HARDWARIO Logo" align="right"></a>

# Sensor Module Radio voltage report

[![build](https://github.com/hardwario/twr-lora-tester/actions/workflows/main.yml/badge.svg)](https://github.com/hardwario/twr-radio-sensor-module-analog-inputs/actions/workflows/main.yml)
[![Release](https://img.shields.io/github/release/hardwario/twr-lora-tester.svg)](https://github.com/hardwario/twr-radio-sensor-module-analog-inputs/releases)
[![License](https://img.shields.io/github/license/hardwario/twr-lora-tester.svg)](https://github.com/hardwario/twr-radio-sensor-module-analog-inputs/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/hardwario_en.svg?style=social&label=Follow)](https://twitter.com/hardwario_en)

## Used TOWER Modules

- Core Module
- Sensor Module
- Radio Dongle

## Maximal measurement voltage is 3.3 V !

**Maximal voltage on Sensor Module pins is 3.3 V !**

Sensor Module is connecting screw terminals A, B and C directly to Core Module's MCU.

You can mod the PCB and create a voltage divider thanks to some unpopulated SMD resistors on the PCB. [See the schematics](https://github.com/hardwario/bc-hardware/tree/master/out/bc-module-sensor).

## Firmware update

- Connect your Core Module to the computer
- Download latest bin file from releases page.
- Use [HARDWARIO Playground](https://www.hardwario.com/download/) go to the firmware tab, click on `...` and select downloaded bin file.

## Functionality

Flash firmware and [pair your module](https://tower.hardwario.com/en/latest/basics/playground-tabs/devices/#pairing-new-devices) to Radio Dongle.

Module measures voltages on Sensor Module channels A,B and C. The period is hard-coded to 10 minutes and you can change it with `ADC_MEASUREMENT_INTERVAL_MS` define in the code.

You can force measurement by pressing the button on the Core Module.

Core Module also measures temperature and battery voltage.

## MQTT topics

```
node/sensor-module/info {"firmware": "sensor-module", "version": "vdev", "mode": 3}
node/sensor-module/thermometer/0:1/temperature 24.75
node/sensor-module/sensormodule/a/voltage 3.26
node/sensor-module/sensormodule/b/voltage 3.16
node/sensor-module/sensormodule/c/voltage 0.77
```

## VDD screw terminal

If you have Sensor Module with revision R1.1 or higer, you have also VDD pin on the screw terminal. This pin is enables 500 ms before measurement. You can use this voltage to power any sensor with 3.0V and measure the ADC value from that sensor.
Thanks to this, you can keep your external device powered-off when not measuring.

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT/) - see the [LICENSE](LICENSE) file for details.

---

Made with &#x2764;&nbsp; by [**HARDWARIO s.r.o.**](https://www.hardwario.com/) in the heart of Europe.
