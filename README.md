# SkiMonitor

A tracking system for skiers looking to monitor (and spice up) their trips to the slopes! I've been working on this project since the middle of Summer 2023, when I wanted a way to track my ski trips and make skiing in the Midwest a bit more fun :). With this project, I can control the LEDs mounted on my skis through an access point web server hosted on the ESP32, as well as view run statistics and track my skiing overall.

Please let me know if there's any functionality that would be fun to add!

## SkiMonitor Interface:
![Image](/SkiMonitor_dashboard.png)


## Software Tools Used:
- HTML, CSS, JS
- Arduino C++

## Hardware Used:
- Adafruit ESP32 Feather
- MakerFocus 10000 mAh LiPo battery
- Adafruit BMP390 Altimeter
- NEO6M GPS module
- WS2812B LED Strips
- 2x 5V @ 1A Boost Converter

## Roadmap

### Version 5 (When I have time)
| Planned | Implementation |
| ---           | ---         |
| Track carving statistics | Add 2 mounted gyro sensors to skis |
|  | Save carving statistics per run |
|  | Compare carving statistics |
|  | Implement live carving grading & symmetry analysis |


### Version 4 (In progress):
- Version 4.8: Applied to LEDs to Skis, soldered and assembled first SkiMonitor module
- Version 4.7: Cleaned up code for readability
- Version 4.6: Improved mobile compatibility, finalized LED functionality, added battery percentage monitoring
- Version 4.5: Added graphing and run data saving
- Version 4.4: Added auto-refresh data without page reload
- Version 4.3: Added multithreading to avoid delays caused by LED code
- Version 4.2: Added live GPS monitoring
- Version 4.1: Added live temperature/altitude sensing


## Acknowledgements

 - [Random Nerd Tutorials](https://randomnerdtutorials.com/): Helped me get started working with microcontrollers since I had no prior experience!
