# Autonomous Mobile Robot with Manual Control Mode

## Overview

This project is an ESP32-based mobile robot that operates in two independent modes: **Autonomous Mode** and **Manual Control Mode**.

The robot integrates computer vision, object detection, voice recognition, wireless communication, ultrasonic distance measurement, and robotic arm control.

The user selects the desired operating mode at startup using voice commands. Each mode provides a different way of controlling the robot.

The system uses Python and OpenCV for autonomous navigation and object detection, while the ESP32 handles motor control, sensor measurements, Wi-Fi communication, and robotic arm operation.

## Operating Modes

### 1. Autonomous Mode

Activated by saying **"Number One"**.

In this mode, the robot autonomously searches for and approaches specific objects using a camera and YOLOv3 object detection.

**Main Features:**
- Real-time object detection using YOLOv3.
- Detection of bottles and cups using the COCO dataset.
- Object position identification and horizontal alignment.
- Automatic navigation toward the detected object.
- Ultrasonic sensor for distance measurement.
- Automatic stopping based on the target distance.
- Programmed return movement after reaching the target.

### 2. Manual Control Mode

Activated by saying **"Number Two"**.

In this mode, the robot is controlled through a web-based interface hosted by the ESP32.

**Main Features:**

**Mobile Robot Control:**
- Forward movement.
- Backward movement.
- Left and right movement.
- Stop command.

**Robotic Arm Control:**

The robotic arm consists of four servo motors:
- Base
- Shoulder
- Elbow
- Gripper

Each servo motor can be controlled independently through a slider on the web interface.

**Motion Recording and Playback:**
- Record robotic arm movements.
- Save servo positions and movement timing during the recording session.
- Replay previously recorded movements automatically.

## Hardware Components

- ESP32 Development Board
- DC Motors
- Motor Driver
- Ultrasonic Distance Sensor (HC-SR04)
- Camera
- Four Servo Motors
- Mobile Robot Chassis
- Robotic Arm

## Software & Technologies

| Technology | Application |
|---|---|
| Python | Autonomous robot control |
| OpenCV | Image processing |
| YOLOv3 | Object detection |
| NumPy | Numerical operations |
| Vosk | Voice recognition |
| PyAudio | Microphone input |
| pyttsx3 | Text-to-speech |
| Arduino IDE | ESP32 programming |
| Wi-Fi / TCP Sockets | Python-to-ESP32 communication |
| WebSocket | Real-time manual control communication |
| HTML / CSS / JavaScript | Web-based control interface |
| ESP32Servo | Servo motor control |

## System Architecture

The project consists of two main software modules.

### Python Module

Responsible for autonomous operation and voice-based mode selection.

- Recognizes voice commands to select the operating mode.
- Captures real-time camera frames.
- Detects target objects using YOLOv3.
- Determines the object's position relative to the camera center.
- Sends movement commands to the ESP32.
- Requests ultrasonic distance measurements.
- Coordinates autonomous navigation.

### ESP32 Module

Responsible for hardware operation and wireless communication.

- Controls the DC motors.
- Reads ultrasonic sensor measurements.
- Receives movement commands through TCP communication.
- Hosts the manual control web server.
- Handles WebSocket communication.
- Controls the four robotic arm servo motors.
- Records and replays robotic arm movements.

## Communication

The system uses wireless communication between the Python application and the ESP32.

**Autonomous Mode:**
- TCP socket communication.
- Python sends navigation commands to the ESP32.
- ESP32 returns ultrasonic distance measurements.

**Manual Control Mode:**
- ESP32 hosts a web-based control interface.
- WebSocket communication sends control commands.
- Servo positions and robotic arm movements are updated through the interface.

## Project Structure

```text
Autonomous-Mobile-Robot/
│
├── Arduino/
│   └── robot_controller.ino
│
├── Python/
│   └── main.py
│
└── README.md
```

## Installation & Setup

### 1. ESP32 Setup

1. Open `robot_controller.ino` using Arduino IDE.
2. Install the required ESP32 libraries:
   - WiFi
   - AsyncTCP
   - ESPAsyncWebServer
   - ESP32Servo
3. Configure your Wi-Fi credentials in the firmware.
4. Upload the code to the ESP32.

### 2. Python Setup

Install the required Python libraries:

```bash
pip install opencv-python numpy vosk pyaudio pyttsx3
```

### 3. External Model Files

The following files are required for autonomous operation but are not included in this repository:

- `yolov3.weights`
- `yolov3.cfg`
- `coco.names`
- Vosk English speech recognition model (`vosk-model-small-en-us-0.15`)

Place the YOLO files in the appropriate Python working directory and extract the Vosk model into:

```text
Python/
└── Models/
    └── vosk-model-small-en-us-0.15/
```

### 4. Network Configuration

Before running the Python application:

- Configure the ESP32 Wi-Fi credentials.
- Set the ESP32 IP address in the Python script.
- Ensure that the computer and ESP32 are connected to the same local network.
- Verify the TCP port configuration.
- Select the correct camera index.

### 5. Run the Project

Run the Python application:

```bash
python main.py
```

The system will ask the user to select an operating mode through voice commands:

- Say **"Number One"** for Autonomous Mode.
- Say **"Number Two"** for Manual Control Mode.

## Future Improvements

- Support for additional object classes.
- Improved object tracking and navigation accuracy.
- Advanced obstacle avoidance.
- Improved autonomous robotic arm integration.
- Enhanced web-based control interface.
- More advanced autonomous object manipulation.

## Author

Developed as a robotics project integrating embedded systems, computer vision, autonomous navigation, wireless communication, and robotic arm control.
