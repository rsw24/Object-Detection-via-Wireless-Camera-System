# RTOS-Based Real-Time Object Detection and Alert System using ESP32-CAM and ESP32-C6

## 1. Project Overview

This mini project streams live video from an ESP32-CAM board with an ESP32-S module over WiFi, runs YOLOv8n object detection on a Python computer application, and sends detection messages to an ESP32-C6. The ESP32-CAM is programmed and powered through the ESP32-CAM-MB USB base board. The ESP32-C6 runs FreeRTOS tasks to receive object detection commands and activate LED/buzzer alerts in real time.

This version does not use an ultrasonic sensor. The RTOS focus is still clear: multitasking, scheduling, task priorities, queues, semaphores/mutexes, periodic tasks, and inter-task communication.

## 2. Component List

| Component | Quantity | Purpose | Optional upgrade |
|---|---:|---|---|
| ESP32-C6-DevKitC-1 | 1 | Main RTOS controller | ESP32-S3 DevKit if more GPIO or AI acceleration is needed |
| ESP32-CAM board with ESP32-S module | 1 | WiFi video streaming camera | ESP32-S3 CAM for better camera performance |
| ESP32-CAM-MB USB base board | 1 | USB programming and easy power/reset for ESP32-CAM | FTDI programmer if CAM-MB is unavailable |
| Active buzzer | 1 | Audible alert | Piezo buzzer with transistor driver |
| LED | 1 | Visual alert | High brightness LED module |
| 220 ohm resistor | 1 | LED current limiting | LED module with onboard resistor |
| Breadboard | 1 | Circuit assembly | Soldered perfboard |
| Jumper wires | 8-12 | LED and buzzer connections | Dupont ribbon wires |
| USB cables | 2 | ESP32-CAM-MB upload/power and ESP32-C6 serial communication | Powered USB hub |

## 3. Wiring Table

### ESP32-C6 Connections

| Device | Pin | ESP32-C6 pin | Notes |
|---|---|---|---|
| LED | Anode | GPIO 2 through 220 ohm resistor | Visual alert |
| LED | Cathode | GND | Common ground |
| Active buzzer | Positive | GPIO 8 | Use active buzzer for simple ON/OFF control |
| Active buzzer | Negative | GND | Common ground |
| ESP32-C6 | USB | PC | Receives serial messages from Python |

No HC-SR04 ultrasonic sensor is required for this version.

### ESP32-CAM + ESP32-CAM-MB Connections

| Board/pin | Connection |
|---|---|
| ESP32-CAM | Plug directly into ESP32-CAM-MB header |
| ESP32-CAM-MB USB | PC USB port for programming, power, and Serial Monitor |
| ESP32-CAM 5V/GND | Usually supplied by ESP32-CAM-MB during demo |
| RST/RESET button | Press after upload to start streaming |
| GPIO 0 | Usually handled by CAM-MB upload circuit; no manual wiring normally needed |

If upload fails, hold the ESP32-CAM-MB `BOOT/IO0` button if your board has one, click upload, then release it when Arduino IDE shows `Connecting...`. Some CAM-MB boards do this automatically.

## 4. Folder Structure

```text
project/
|
+-- esp32_cam/
|   +-- esp32_cam_stream.ino
|
+-- esp32_c6_rtos/
|   +-- esp32_c6_rtos.ino
|
+-- python_detection/
|   +-- detect.py
|   +-- requirements.txt
|
+-- diagrams/
|   +-- block_diagram.png
|   +-- flowchart.png
|
+-- report/
    +-- abstract.txt
    +-- viva_questions.txt
```

## 5. Block Diagram

![Block diagram](diagrams/block_diagram.png)

## 6. Flowchart

![Flowchart](diagrams/flowchart.png)

## 7. Arduino IDE Setup

1. Install Arduino IDE 2.x.
2. Open Preferences.
3. Add this Boards Manager URL:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

4. Open Boards Manager and install `esp32 by Espressif Systems`.
5. For your ESP32-CAM with ESP32-S module on ESP32-CAM-MB, select `AI Thinker ESP32-CAM`.
6. For ESP32-C6, select `ESP32C6 Dev Module` or `ESP32-C6-DevKitC-1`.
7. Set Serial Monitor baud rate to `115200`.

## 8. ESP32-CAM-MB Camera Installation

1. Open `esp32_cam/esp32_cam_stream.ino`.
2. Replace:

```cpp
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

3. Plug the ESP32-CAM into the ESP32-CAM-MB base board.
4. Connect the ESP32-CAM-MB to the PC using USB.
5. Select the ESP32-CAM-MB COM port in Arduino IDE.
6. Upload the sketch.
7. If upload does not start, use the CAM-MB `BOOT/IO0` button method described above.
8. After upload, press `RST/RESET` on the ESP32-CAM-MB.
9. Open Serial Monitor and copy the printed stream URL, for example:

```text
http://192.168.1.50/stream
```

## 9. ESP32-C6 Installation

1. Open `esp32_c6_rtos/esp32_c6_rtos.ino`.
2. Select the ESP32-C6 board and correct COM port.
3. Upload the sketch.
4. Open Serial Monitor at `115200`.
5. Confirm this message appears:

```text
ESP32-C6 FreeRTOS object detection alert system started
```

You can manually test the alert from Serial Monitor by sending:

```text
TEST_ALERT
```

Set the Serial Monitor line ending to `Newline` or `Both NL & CR`.

## 10. Python Setup

Open a terminal in `project/python_detection` and run:

```bash
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
```

On Linux/macOS:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

The first run may download `yolov8n.pt` automatically.

## 11. Serial Communication Setup

The Python application sends newline-terminated serial commands to the ESP32-C6:

```text
PERSON_DETECTED
CHAIR_DETECTED
BOTTLE_DETECTED
PHONE_DETECTED
LAPTOP_DETECTED
```

The ESP32-C6 code also accepts this manual test command:

```text
TEST_ALERT
```

Find the ESP32-C6 COM port in Arduino IDE under Tools > Port. Use that port in the Python command.

Example for Windows:

```bash
python detect.py --stream-url http://192.168.1.50/stream --serial-port COM5
```

Example for Linux:

```bash
python detect.py --stream-url http://192.168.1.50/stream --serial-port /dev/ttyUSB0
```

## 12. How to Run the Project

1. Power the ESP32-CAM through the ESP32-CAM-MB USB board and power the ESP32-C6 through its USB port.
2. Make sure ESP32-CAM and the computer are on the same WiFi network.
3. Open the ESP32-CAM-MB Serial Monitor and note the stream URL.
4. Upload and run the ESP32-C6 FreeRTOS sketch.
5. Close Arduino Serial Monitor before running Python, because only one program can use the COM port at a time.
6. Run:

```bash
python detect.py --stream-url http://ESP32_CAM_IP/stream --serial-port ESP32_C6_COM_PORT
```

7. Show a person, chair, bottle, phone, or laptop to the camera.
8. The Python window displays bounding boxes.
9. ESP32-C6 activates the LED and buzzer when it receives a detection message.

## 13. Expected Output

Python terminal:

```text
Loading YOLOv8n model. First run may download yolov8n.pt.
Serial connected on COM5 at 115200 baud
Detection started. Press q in the video window to quit.
Sent to ESP32-C6: PERSON_DETECTED
```

ESP32-C6 Serial Monitor:

```text
ESP32-C6 FreeRTOS object detection alert system started
Hardware: LED on GPIO 2, active buzzer on GPIO 8
STATUS | Uptime: 4 s | Free heap: 312000 bytes | YOLO alerts: 0 | Test alerts: 0 | Heartbeats: 4 | Last command: none
ALERT: PERSON detected by YOLO
STATUS | Uptime: 8 s | Free heap: 312000 bytes | YOLO alerts: 1 | Test alerts: 0 | Heartbeats: 8 | Last command: 1 s ago
```

## 14. RTOS Explanation

The ESP32-C6 firmware uses FreeRTOS to split the embedded controller into independent tasks. Each task has one clear responsibility. This makes the system easier to understand and allows time-critical actions, such as alerts, to execute quickly.

Tasks used:

| Task | Priority | Function |
|---|---:|---|
| Alert Task | 4 | Highest priority, turns buzzer and LED on/off |
| Communication Task | 3 | Reads serial commands from Python |
| Status Task | 2 | Prints periodic logs and reads heartbeat queue |
| Heartbeat Task | 1 | Sends periodic system-health messages to queue |

## 15. Scheduling Explanation

FreeRTOS uses priority-based preemptive scheduling. If a higher-priority task becomes ready, it can run before a lower-priority task. In this project, the Alert Task has the highest priority, so alert response is fast when an object detection message arrives.

`vTaskDelay()` is used to create periodic behavior. While one task is delayed, the CPU can run other ready tasks.

## 16. Queue Explanation

Queues are used for safe inter-task communication.

`alertQueue` carries detection alert events from Communication Task to Alert Task.

`heartbeatQueue` carries periodic health messages from Heartbeat Task to Status Task.

This avoids unsafe sharing of large data between tasks and makes the system modular.

## 17. Semaphore Explanation

The project uses a mutex named `serialMutex`. A mutex is a binary semaphore used for mutual exclusion. Multiple tasks may print to Serial, but only one task should print at a time. The mutex prevents mixed or corrupted Serial output.

## 18. Advantages of RTOS Over Superloop

| RTOS design | Superloop design |
|---|---|
| Tasks are separated by responsibility | All logic is inside one loop |
| Priority controls urgent behavior | Every action waits for previous code |
| Queues provide clean communication | Shared variables become harder to manage |
| vTaskDelay allows other tasks to run | delay() blocks the whole loop |
| Easier to expand | Becomes messy as features increase |

## 19. Code Modules

| File | Purpose |
|---|---|
| `esp32_cam/esp32_cam_stream.ino` | ESP32-CAM ESP32-S module MJPEG WiFi stream, uploaded through ESP32-CAM-MB |
| `esp32_c6_rtos/esp32_c6_rtos.ino` | ESP32-C6 FreeRTOS LED/buzzer alert controller |
| `python_detection/detect.py` | YOLOv8 OpenCV detection and serial sender |
| `python_detection/requirements.txt` | Python dependencies |
| `report/abstract.txt` | Mini-project abstract |
| `report/viva_questions.txt` | Viva preparation |

## 20. Mini-Project Abstract

This project implements a real-time object detection and alert system using an ESP32-CAM with ESP32-S module, ESP32-CAM-MB programmer board, ESP32-C6, Python OpenCV, and YOLOv8. The ESP32-CAM streams live video through WiFi. A Python application detects selected objects and sends serial commands to the ESP32-C6. The ESP32-C6 runs FreeRTOS tasks to manage serial communication, periodic status monitoring, and LED/buzzer alert generation. Queues and a mutex demonstrate RTOS inter-task communication and resource protection. The project shows how an RTOS improves responsiveness and modularity in an embedded monitoring system.

## 21. Future Scope

1. Replace serial communication with WiFi MQTT or UDP.
2. Add an OLED display for detected object name and alert count.
3. Store alert events on an SD card or cloud database.
4. Add object-specific alert patterns.
5. Use ESP32-S3 for edge AI inference.
6. Add a web dashboard for live status.
7. Add battery power and enclosure for portable use.
8. Add sensors later, such as PIR, IR obstacle, or ultrasonic distance sensor.
