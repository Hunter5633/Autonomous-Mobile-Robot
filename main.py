import cv2
import numpy as np
import socket
import time
import os  # Import the os module to handle file paths
from vosk import Model, KaldiRecognizer
import pyaudio
import json
import pyttsx3
cnt = 0
cnt2 = 0 
cnt3 = 0 
def recognize_mode():
    # Keep the extracted Vosk model in Models/vosk-model-small-en-us-0.15
    base_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(base_dir, "Models", "vosk-model-small-en-us-0.15")


    if not os.path.exists(model_path):
        print("Vosk model not found. Place it in Models/vosk-model-small-en-us-0.15.")
        return None
    
    model = Model(model_path)
    recognizer = KaldiRecognizer(model, 16000)

    # Initialize microphone audio input
    audio = pyaudio.PyAudio()
    stream = audio.open(format=pyaudio.paInt16, channels=1, rate=16000, input=True, frames_per_buffer=8000)
    stream.start_stream()

    print("Say 'number one' for Autonomous Mode or 'number two' for Manual Mode:")
    try:
        while True:
            data = stream.read(4000, exception_on_overflow=False)
            if recognizer.AcceptWaveform(data):
                result = json.loads(recognizer.Result())
                recognized_text = result.get("text", "").lower()
                print(f"Recognized: {recognized_text}")

                if "number one" in recognized_text:
                    text2 = recognized_text
                    MODE = 0
                    
                    print(text2)
                    
                    break
                elif "number two" in recognized_text:
                    text2 = recognized_text
                    print(text2)
                    MODE = 1
                    break
                else:
                    print("Didn't recognize a valid num. Try again.")
               
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        stream.stop_stream()
        stream.close()
        audio.terminate()
    return MODE if "MODE" in locals() else None

engine = pyttsx3.init()
# Set the text to speak
tex2= "which mode do you wanna pick TEAM2?"
# Speak the text
engine.say(tex2)
engine.runAndWait()
MODE = recognize_mode()
if MODE is None:
    raise SystemExit("Mode selection was not completed. Exiting.")

cnt = 0
cnt2 = 0
cnt3 = 0

# Arduino setup
ARDUINO_IP = "192.168.1.6" # Replace with your Arduino's IP address
ARDUINO_PORT = 8080
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((ARDUINO_IP, ARDUINO_PORT))
print("Connected\n")
time.sleep(2)  # Allow Arduino to initialize

if MODE:
 sock.sendall(b'MODE\n')
 print("Sent")

# Load YOLO model
net = cv2.dnn.readNet('yolov3.weights', 'yolov3.cfg')
layer_names = net.getLayerNames()
output_layers = [layer_names[i - 1] for i in net.getUnconnectedOutLayers()]

# Load COCO class labels
with open('coco.names', 'r') as f:
    classes = [line.strip() for line in f.readlines()]

# Load Camera
cap = cv2.VideoCapture(3)

# Thresholds
alignment_threshold = 50  # Pixels for alignment margin of error
distance_threshold = 9  # Distance in cm to stop

# Alignment state
aligned = False

def getdistance():
    sock.sendall(b'DISTANCE\n')
    sock.settimeout(1)  # Set socket timeout for receiving data
    distance = None
    timeout = time.time() + 1
    
    try:
        while time.time() < timeout:
            try:
                response = sock.recv(1024).decode('utf-8').strip()
                print(f"Received: {response}")  # Debugging to check response
                
                if response.startswith('DIST:'):
                    try:
                        distance = int(response[5:])
                        return distance  # Return immediately after receiving valid distance
                    except ValueError:
                        print("Invalid distance format")
                        pass
            except socket.timeout:
                print("Timeout waiting for distance. Retrying...")
                sock.sendall(b'DISTANCE\n')  # Resend request on timeout
        
    except Exception as e:
        print(f"Error during socket communication: {e}")
    
    return distance


while MODE == 0:
    print(cnt2)
    ret, frame = cap.read()
    if not ret:
        break

    height, width, channels = frame.shape
    frame_center_x = width // 2

    # YOLO object detection
    blob = cv2.dnn.blobFromImage(frame, 0.00392, (320, 320), (0, 0, 0), True, crop=False)
    net.setInput(blob)
    outs = net.forward(output_layers)

    class_ids = []
    confidences = []
    boxes = []

    for out in outs:
        for detection in out:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]

            if confidence > 0.5:
                label = classes[class_id]
                if label in ['cup', 'bottle']:  # We only care about these two objects
                    center_x = int(detection[0] * width)
                    center_y = int(detection[1] * height)
                    w = int(detection[2] * width)
                    h = int(detection[3] * height)

                    x = int(center_x - w / 2)
                    y = int(center_y - h / 2)

                    boxes.append([x, y, w, h])
                    confidences.append(float(confidence))
                    class_ids.append(class_id)

    # Filter objects using Non-Maximum Suppression
    indexes = cv2.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)

    if len(boxes) == 0:  # No objects detected
        cnt3 = 0
        if cnt2 == 0:
            sock.sendall(b'LEFT\n')
            print(b'Finding\n')
        else:
            sock.sendall(b'STOP\n')
            cv2.putText(frame, "No Object Detected", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
    else:
        for i in range(len(boxes)):
            if i in indexes:
                x, y, w, h = boxes[i]
                object_center_x = x + w // 2

                # Object alignment logic (only check alignment if not aligned yet)
                if abs(object_center_x - frame_center_x) <= alignment_threshold and not aligned:
                    # Aligned with the object
                    cv2.putText(frame, "Aligned with Object", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                    if cnt3 != 1:
                      aligned = True  # Set aligned state to True

                    # Once aligned, the robot should move forward and not check alignment until the distance condition is met
                    if aligned:
                        # Check the distance before deciding further action
                        distance = getdistance()
                        if distance is not None and distance > distance_threshold:
                            print("Sending FORWARD command...")  # Debugging: Confirm command is being sent
                            cv2.putText(frame, f"Distance: {distance} cm", (50, 100), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
                            sock.sendall(b'FORWARD\n')
                            cnt2 = 1 
                            time.sleep(0.1)
                            cv2.putText(frame, "Moving Forward", (50, 150), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                            aligned = False
                        else:
                            # Reset alignment after reaching 10 cm
                            aligned = False  # Reset alignment once the robot gets close to the object (within 10 cm)
                            sock.sendall(b'STOP\n') 
                            cv2.putText(frame, "STOPPED", (50, 150), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
                            if aligned is False and distance is not None and cnt != 1:
                              if distance >= distance_threshold:
                                  sock.sendall(b'STOP\n')
                                  cv2.putText(frame, "Reached 20 cm", (50, 200), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                                  cnt2 = 0
                                  cnt3 = 1
                                  sock.sendall(b'Return\n')
                                  aligned = False
                                  print(cnt2)
                                  cnt = 1 
                                  print("sending return commands\n")
                                  break

                              elif distance < distance_threshold:
                                  sock.sendall(b'BACKWARD\n')
                                  cnt2 = 1
                                  time.sleep(0.1)  # Small delay to ensure precise control
                                  cv2.putText(frame, f"Adjusting Distance: {distance} cm", (50, 300), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 2)
        

                # Object misalignment handling (only move left or right if not aligned yet)
                elif ((abs(object_center_x - frame_center_x) > alignment_threshold) and cnt3 != 1):
                    if object_center_x < frame_center_x:
                        sock.sendall(b'LEFT\n')
                        cnt2 = 1 
                        print("Sending left command\n")
                        time.sleep(0.1)
                        cv2.putText(frame, "Moving Left", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
                    else:
                        sock.sendall(b'RIGHT\n')
                        cnt2 = 1 
                        print("Sending right command\n")
                        time.sleep(0.1)
                        cv2.putText(frame, "Moving Right", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

    # Show video feed with the overlay text
    cv2.imshow('Object Detection and Navigation', frame)

    # Exit on 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Cleanup
cap.release()
cv2.destroyAllWindows()
sock.close()
