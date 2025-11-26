# Landmark Detection and ESP32 Communication System

This project integrates computer vision-based landmark detection with ESP32 communication for robotics applications.

## Overview

The system uses a phone camera to detect colored landmarks and communicates detection events to an ESP32 microcontroller in real-time.

## Features

- **Color-based Landmark Detection**: Detects 4 types of landmarks using HSV color filtering:
  - RED-GREEN
  - BLUE-GREEN
  - GREEN-RED
  - GREEN-BLUE

- **Distance Estimation**: Calculates approximate distance to detected landmarks using perspective projection

- **ESP32 Integration**: Sends detection/loss events to ESP32 via TCP socket communication

- **Real-time Processing**: Processes camera feed in real-time with visual feedback

## Requirements

```bash
pip install opencv-python numpy requests
```

## Configuration

### Phone Camera
Set your phone's IP address running IP Webcam or similar app:
```python
PHONE_URL = "http://YOUR_PHONE_IP:8080/shot.jpg"
```

### ESP32
Configure your ESP32's IP address and port:
```python
ESP32_IP = "YOUR_ESP32_IP"
ESP32_PORT = 8888
```

## Usage

Run the integrated notebook cell (Cell 3) to start the system. The program will:

1. Connect to your phone camera
2. Detect colored landmarks in real-time
3. Send `LANDMARK_DETECTED_<type>` when a landmark appears
4. Send `LANDMARK_LOST_<type>` when a landmark disappears
5. Display visual feedback with bounding boxes and distance estimates

Press ESC to exit.

## ESP32 Commands

The system sends these commands to the ESP32:
- `LANDMARK_DETECTED_RED_GREEN\n`
- `LANDMARK_DETECTED_BLUE_GREEN\n`
- `LANDMARK_DETECTED_GREEN_RED\n`
- `LANDMARK_DETECTED_GREEN_BLUE\n`
- `LANDMARK_LOST_RED_GREEN\n`
- `LANDMARK_LOST_BLUE_GREEN\n`
- `LANDMARK_LOST_GREEN_RED\n`
- `LANDMARK_LOST_GREEN_BLUE\n`

## License

MIT
