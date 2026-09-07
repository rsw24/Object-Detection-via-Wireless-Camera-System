import argparse
import time
from typing import Dict, Optional, Set

import cv2
import serial
from ultralytics import YOLO


TARGET_CLASSES: Set[str] = {"person", "chair", "bottle", "cell phone", "laptop"}

SERIAL_MESSAGES: Dict[str, str] = {
    "person": "PERSON_DETECTED",
    "chair": "CHAIR_DETECTED",
    "bottle": "BOTTLE_DETECTED",
    "cell phone": "PHONE_DETECTED",
    "laptop": "LAPTOP_DETECTED",
}


def open_serial(port: Optional[str], baudrate: int) -> Optional[serial.Serial]:
    if not port:
        print("Serial disabled. Run with --serial-port COMx to notify ESP32-C6.")
        return None

    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2.0)
        print(f"Serial connected on {port} at {baudrate} baud")
        ser.write(b"PING\n")
        return ser
    except serial.SerialException as exc:
        print(f"Could not open serial port {port}: {exc}")
        return None


def send_detection(ser: Optional[serial.Serial], message: str) -> None:
    if ser is None:
        return

    try:
        ser.write((message + "\n").encode("utf-8"))
    except serial.SerialException as exc:
        print(f"Serial write failed: {exc}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="YOLOv8 ESP32-CAM object detection")
    parser.add_argument(
        "--stream-url",
        required=True,
        help="ESP32-CAM MJPEG URL, for example http://192.168.1.50/stream",
    )
    parser.add_argument(
        "--serial-port",
        default=None,
        help="ESP32-C6 serial port, for example COM5 on Windows or /dev/ttyUSB0 on Linux",
    )
    parser.add_argument("--baudrate", type=int, default=115200)
    parser.add_argument("--confidence", type=float, default=0.45)
    parser.add_argument(
        "--cooldown",
        type=float,
        default=2.0,
        help="Minimum seconds between repeated alerts for the same class",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    print("Loading YOLOv8n model. First run may download yolov8n.pt.")
    model = YOLO("yolov8n.pt")

    ser = open_serial(args.serial_port, args.baudrate)

    cap = cv2.VideoCapture(args.stream_url)
    if not cap.isOpened():
        print("Could not open ESP32-CAM stream.")
        print("Check that the ESP32-CAM IP address is correct and your PC is on the same WiFi.")
        return

    last_sent_time: Dict[str, float] = {}
    window_name = "ESP32-CAM YOLOv8 Detection"

    print("Detection started. Press q in the video window to quit.")

    while True:
        ok, frame = cap.read()
        if not ok:
            print("Frame read failed. Reconnecting in 1 second...")
            time.sleep(1.0)
            cap.release()
            cap = cv2.VideoCapture(args.stream_url)
            continue

        results = model(frame, verbose=False, conf=args.confidence)
        annotated = frame.copy()
        now = time.time()

        for result in results:
            for box in result.boxes:
                class_id = int(box.cls[0])
                class_name = model.names[class_id]
                confidence = float(box.conf[0])

                if class_name not in TARGET_CLASSES:
                    continue

                x1, y1, x2, y2 = map(int, box.xyxy[0])
                label = f"{class_name} {confidence:.2f}"

                cv2.rectangle(annotated, (x1, y1), (x2, y2), (0, 255, 0), 2)
                cv2.putText(
                    annotated,
                    label,
                    (x1, max(25, y1 - 10)),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.7,
                    (0, 255, 0),
                    2,
                )

                previous_time = last_sent_time.get(class_name, 0.0)
                if now - previous_time >= args.cooldown:
                    message = SERIAL_MESSAGES[class_name]
                    send_detection(ser, message)
                    last_sent_time[class_name] = now
                    print(f"Sent to ESP32-C6: {message}")

        cv2.imshow(window_name, annotated)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

    cap.release()
    if ser is not None:
        ser.close()
    cv2.destroyAllWindows()
    print("Detection stopped.")


if __name__ == "__main__":
    main()
