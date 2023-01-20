[![ESP-IDF](https://github.com/sztvka/solarcharger/actions/workflows/cmake.yml/badge.svg)](https://github.com/sztvka/solarcharger/actions/workflows/cmake.yml)
# SolarCharger
This project is my first shot at programming real-time systems featuring multiple tasks. I wanted to create a dual axis solar tracker with clean and reliable code in the field where most of the similar projects use poorly writen Arduino code.  
The device is able to:
- Track the sun in two axis
- Charge a battery
- Display a web UI displaying the current status of the device
![photo1](img/photo1.jpg?raw=true "Photo")
![webUI](img/scr_1.jpg?raw=true "Web UI")

## Hardware

### Mechanical
The mechanical design is a remix of the [Dual Axis Solar Tracker](https://www.thingiverse.com/thing:53321) by [OpenSourceClassroom](https://www.thingiverse.com/opensourceclassroom/designs). My updated design fixes some major mechanical issues apparent in the original design, mainly related to poor alignment of the two axis. In my new design, the pan axis is kept aligned with bearings, the gears are screwed on instead of being plastic welded and my design features proper cable management with strain relief.  
Required materials for assembly:
- Printed parts
- M4x10mm Screws
- M4x20mm Screws
- M3x6mm Screws
- M3 standoffs (Optional, extra space under the pcb allows cable management)
- M6x100mm threaded rod with two M6 nuts
- MG-996R Servo (pan)
- MG90S or SG90 Servo (tilt)
- Cables (silicone insulation is optimal)
- Cable conduit


### Electronics
![Hardware flowchart](img/flowchart_hardware.jpg?raw=true "Hardware flowchart")  
Everything is placed onto a custom pcb. For more details refer to schematics and kicad files in the repository. This design uses a 3W solar panel.

## Software
The code is written in ESP-IDF with FreeRTOS and is designed to run on an ESP32. 

## Compiling

#### Prerequisites
 - [ESP32 Toolchain](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#step-2-set-up-the-toolchain)
 - Python interpreter

#### Steps
1. Clone the repository `git clone https://github.com/sztvka/solarcharger.git`
2. Make sure your ESP-IDF path is set correctly and `idf.py` is resolvable
3. Install the required python packages `pip install rjsmin css_html_js_minify`
4. Configure your install in `config_default.h` file
5. Run the `compile_extras.py` script to setup the web UI (this will minify web files and set your config for compiling)
6. Run `idf.py build` to compile the project
7. Flash the device with `idf.py -p (PORT) flash`



## Extra media


![photo](https://user-images.githubusercontent.com/47701797/213795986-06cec554-1c33-434d-908f-fc9d81b5e190.jpg)

https://user-images.githubusercontent.com/47701797/213793889-40177bec-c784-479d-9bc1-420ded0e9b07.mp4


## License

This project is licensed under the Creative Comms Attribution Share Alike 4.0 International - see the [LICENSE](LICENSE) file for details  
![cc](https://upload.wikimedia.org/wikipedia/commons/e/e5/CC_BY-SA_icon.svg)
