#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ESP32Servo.h>
#include <iostream>
#include <sstream>
#define LEFT_MOTOR_FORWARD 2
#define LEFT_MOTOR_BACKWARD 4
#define RIGHT_MOTOR_FORWARD 5
#define RIGHT_MOTOR_BACKWARD 18
#define trigPin 12
#define echoPin 13

#define R 0
#define L 1
#define enA 19
#define enB 21
int Speed = 150;
int control = 0; 
long duration, distance;

WiFiServer wifiserver(8080);

void moveForward() {
  ledcWrite(R, 170);
  ledcWrite(L, 170);
  digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
  digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
  digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
  Delay(); // Short pulse for controlled movement
  stopMotors();
}

void Delay() {
  delay(150);
}

void moveBackward() {
  ledcWrite(R, 170);
  ledcWrite(L, 170);
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
  delay(50);
  stopMotors();
}

void moveBackward2() {
  ledcWrite(R, 170);
  ledcWrite(L, 170);
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
  Delay();
  stopMotors();
}

void moveLeft() {
  ledcWrite(R, Speed);
  ledcWrite(L, Speed);
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
  Delay(); // Short pulse for controlled movement
  stopMotors();
}

void moveLeft2() {
  ledcWrite(R, 170);
  ledcWrite(L, 170);
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
}

void moveRight() {
  ledcWrite(R, Speed);
  ledcWrite(L, Speed);
  digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
  digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
  Delay(); // Short pulse for controlled movement
  stopMotors();
}

void stopMotors() {
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
}

struct ServoPins
{
  Servo servo;
  int servoPin;
  String servoName;
  int initialPosition;  
};
std::vector<ServoPins> servoPins = 
{
  { Servo(), 27 , "Base", 0},
  { Servo(), 26 , "Shoulder", 0},
  { Servo(), 25 , "Elbow", 0},
  { Servo(), 33 , "Gripper", 0},
};

struct RecordedStep
{
  int servoIndex;
  int value;
  int delayInStep;
};
std::vector<RecordedStep> recordedSteps;

bool recordSteps = false;
bool playRecordedSteps = false;

unsigned long previousTimeInMilli = millis();

// Set your own Wi-Fi credentials before uploading the firmware.
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

AsyncWebServer server(80);
AsyncWebSocket wsRobotArmInput("/RobotArmInput");

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
    /* Your existing styles */
    input[type=button] {
      background-color: red;
      color: white;
      border-radius: 30px;
      width: 100%;
      height: 40px;
      font-size: 20px;
      text-align: center;
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
      -webkit-user-select: none; /* Safari */
      -khtml-user-select: none; /* Konqueror HTML */
      -moz-user-select: none; /* Firefox */
      -ms-user-select: none; /* Internet Explorer/Edge */
      user-select: none; /* Non-prefixed version, currently supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 20px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }

    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }
    </style>
  </head>
  <body class="noselect" align="center" style="background-color:white">
    <h1 style="color: teal;text-align:center;">Uchihas</h1>
    <h2 style="color: teal;text-align:center;">Robot Arm Control</h2>

    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr/><tr/>
      <tr/><tr/>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Gripper:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Gripper" oninput='sendButtonInput("Gripper",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Elbow:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Elbow" oninput='sendButtonInput("Elbow",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/>      
      <tr>
        <td style="text-align:left;font-size:25px"><b>Shoulder:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Shoulder" oninput='sendButtonInput("Shoulder",value)'>
          </div>
        </td>
      </tr>  
      <tr/><tr/>      
      <tr>
        <td style="text-align:left;font-size:25px"><b>Base:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Base" oninput='sendButtonInput("Base",value)'>
          </div>
        </td>
      </tr> 
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b>Record:</b></td>
        <td><input type="button" id="Record" value="Record" ontouchend='onclickButton(this)'></td>
        <td></td>
      </tr>
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b>Play:</b></td>
        <td><input type="button" id="Play" value="Play" ontouchend='onclickButton(this)'></td>
        <td></td>
      </tr>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Move Forward:</b></td>
        <td><input type="button" id="MoveForward" value="Hold to Move Forward" 
                   onmousedown="startFunction(this)" 
                   onmouseup="stopFunction(this)" 
                   ontouchstart="startFunction(this)" 
                   ontouchend="stopFunction(this)"></td>
      </tr>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Move Left:</b></td>
        <td><input type="button" id="MoveLeft" value="Hold to Move Forward" 
                   onmousedown="startFunction(this)" 
                   onmouseup="stopFunction(this)" 
                   ontouchstart="startFunction(this)" 
                   ontouchend="stopFunction(this)"></td>
      </tr>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Move Right:</b></td>
        <td><input type="button" id="MoveRight" value="Hold to Move Forward" 
                   onmousedown="startFunction(this)" 
                   onmouseup="stopFunction(this)" 
                   ontouchstart="startFunction(this)" 
                   ontouchend="stopFunction(this)"></td>
      </tr>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Move BackWard:</b></td>
        <td><input type="button" id="MoveBackWard" value="Hold to Move Forward" 
                   onmousedown="startFunction(this)" 
                   onmouseup="stopFunction(this)" 
                   ontouchstart="startFunction(this)" 
                   ontouchend="stopFunction(this)"></td>
      </tr>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Stop:</b></td>
        <td><input type="button" id="Stop" value="Hold to Move Forward" 
                   onmousedown="startFunction(this)" 
                   onmouseup="stopFunction(this)" 
                   ontouchstart="startFunction(this)" 
                   ontouchend="stopFunction(this)"></td>
      </tr>
    </table>

    <script>
      var webSocketRobotArmInputUrl = "ws:\/\/" + window.location.hostname + "/RobotArmInput";      
      var websocketRobotArmInput;

      function initRobotArmInputWebSocket() {
        websocketRobotArmInput = new WebSocket(webSocketRobotArmInputUrl);
        websocketRobotArmInput.onopen = function(event) {};
        websocketRobotArmInput.onclose = function(event) { setTimeout(initRobotArmInputWebSocket, 2000); };
        websocketRobotArmInput.onmessage = function(event) {
          var keyValue = event.data.split(",");
          var button = document.getElementById(keyValue[0]);
          button.value = keyValue[1];
          if (button.id == "Record" || button.id == "Play") {
            button.style.backgroundColor = (button.value == "ON" ? "green" : "red");
            enableDisableButtonsSliders(button);
          }
        };
      }

      function sendButtonInput(key, value) {
        var data = key + "," + value;
        websocketRobotArmInput.send(data);
      }

      var interval;
      var keyToSend = "MoveForward"; // Set the key value for MoveForward button

      function startFunction(button) {
        // Start sending data to Arduino while the button is pressed
        interval = setInterval(function() {
          console.log("Button is being pressed");
          sendButtonInput(button.id, 1);  // Send 1 when the button is pressed
        }, 200); // Sends every 100 milliseconds
      }

      function stopFunction(button) {
        // Stop sending data when the button is released
        clearInterval(interval);
        console.log("Button released");
        sendButtonInput(button.id, 0);  // Send 0 when the button is released
      }

      function onclickButton(button) {
        button.value = (button.value == "ON") ? "OFF" : "ON";        
        button.style.backgroundColor = (button.value == "ON" ? "green" : "red");          
        var value = (button.value == "ON") ? 1 : 0 ;
        sendButtonInput(button.id, value);
        enableDisableButtonsSliders(button);
      }

      function enableDisableButtonsSliders(button) {
        if(button.id == "Play") {
          var disabled = "auto";
          if (button.value == "ON") {
            disabled = "none";            
          }
          document.getElementById("Gripper").style.pointerEvents = disabled;
          document.getElementById("Elbow").style.pointerEvents = disabled;          
          document.getElementById("Shoulder").style.pointerEvents = disabled;          
          document.getElementById("Base").style.pointerEvents = disabled; 
          document.getElementById("Record").style.pointerEvents = disabled;
        }
        if(button.id == "Record") {
          var disabled = "auto";
          if (button.value == "ON") {
            disabled = "none";            
          }
          document.getElementById("Play").style.pointerEvents = disabled;
        }        
      }

      window.onload = initRobotArmInputWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault();
      });      
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";
void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

void onRobotArmInputWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                       
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      sendCurrentRobotArmState();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str()); 
        int valueInt = atoi(value.c_str()); 
        
        if (key == "Record")
        {
          recordSteps = valueInt;
          if (recordSteps)
          {
            recordedSteps.clear();
            previousTimeInMilli = millis();
          }
        }
        else if(key == "MoveForward"){
          moveForward();
        }
        else if(key == "MoveLeft"){
          moveLeft();
        }
        else if(key == "MoveRight"){
          moveRight();
        }
        else if(key == "MoveBackWard"){
          moveBackward2();
        }
        else if(key == "Stop"){
          stopMotors();
        }
        else if (key == "Play")
        {
          playRecordedSteps = valueInt;
        }
        else if (key == "Base")
        {
          writeServoValues(0, valueInt);            
        } 
        else if (key == "Shoulder")
        {
          writeServoValues(1, valueInt);           
        } 
        else if (key == "Elbow")
        {
          writeServoValues(2, valueInt);           
        }         
        else if (key == "Gripper")
        {
          writeServoValues(3, valueInt);     
        }   
             

      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void sendCurrentRobotArmState()
{
  for (int i = 0; i < servoPins.size(); i++)
  {
    wsRobotArmInput.textAll(servoPins[i].servoName + "," + servoPins[i].servo.read());
  }
  wsRobotArmInput.textAll(String("Record,") + (recordSteps ? "ON" : "OFF"));
  wsRobotArmInput.textAll(String("Play,") + (playRecordedSteps ? "ON" : "OFF"));  
}

void writeServoValues(int servoIndex, int value)
{
  if (recordSteps)
  {
    RecordedStep recordedStep;       
    if (recordedSteps.size() == 0) // We will first record initial position of all servos. 
    {
      for (int i = 0; i < servoPins.size(); i++)
      {
        recordedStep.servoIndex = i; 
        recordedStep.value = servoPins[i].servo.read(); 
        recordedStep.delayInStep = 0;
        recordedSteps.push_back(recordedStep);         
      }      
    }
    unsigned long currentTime = millis();
    recordedStep.servoIndex = servoIndex; 
    recordedStep.value = value; 
    recordedStep.delayInStep = currentTime - previousTimeInMilli;
    recordedSteps.push_back(recordedStep);  
    previousTimeInMilli = currentTime;         
  }
  servoPins[servoIndex].servo.write(value);   
}

void playRecordedRobotArmSteps()
{
  if (recordedSteps.size() == 0)
  {
    return;
  }
  //This is to move servo to initial position slowly. First 4 steps are initial position    
  for (int i = 0; i < 4 && playRecordedSteps; i++)
  {
    RecordedStep &recordedStep = recordedSteps[i];
    int currentServoPosition = servoPins[recordedStep.servoIndex].servo.read();
    while (currentServoPosition != recordedStep.value && playRecordedSteps)  
    {
      currentServoPosition = (currentServoPosition > recordedStep.value ? currentServoPosition - 1 : currentServoPosition + 1); 
      servoPins[recordedStep.servoIndex].servo.write(currentServoPosition);
      wsRobotArmInput.textAll(servoPins[recordedStep.servoIndex].servoName + "," + currentServoPosition);
      delay(50);
    }
  }
  delay(2000); // Delay before starting the actual steps.
  
  for (int i = 4; i < recordedSteps.size() && playRecordedSteps ; i++)
  {
    RecordedStep &recordedStep = recordedSteps[i];
    delay(recordedStep.delayInStep);
    servoPins[recordedStep.servoIndex].servo.write(recordedStep.value);
    wsRobotArmInput.textAll(servoPins[recordedStep.servoIndex].servoName + "," + recordedStep.value);
  }
}

void setUpPinModes()
{
  for (int i = 0; i < servoPins.size(); i++)
  {
    servoPins[i].servo.attach(servoPins[i].servoPin);
    servoPins[i].servo.write(servoPins[i].initialPosition);    
  }
}


void setup(void) {
  Serial.begin(115200);

  if(control == 1){
    Serial.println(control);
    setUpPinModes();
    pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(LEFT_MOTOR_BACKWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_BACKWARD, OUTPUT);
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);

    // Setup PWM channels
    ledcSetup(R, 5000, 8);  // Channel 0 for Motor A, 5 kHz frequency, 8-bit resolution
    ledcAttachPin(enA, R);
    ledcSetup(L, 5000, 8);  // Channel 0 for Motor B, 5 kHz frequency, 8-bit resolution
    ledcAttachPin(enB, L);

    WiFi.begin(ssid, password);
    Serial.println(WiFi.localIP());
    Serial.print("AP IP address: ");
    Serial.println(WiFi.localIP());


    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
      
    wsRobotArmInput.onEvent(onRobotArmInputWebSocketEvent);
    server.addHandler(&wsRobotArmInput);

    server.begin();
    Serial.println("HTTP server started");
  
  }
  else if (control == 0){
    Serial.begin(115200);
    pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(LEFT_MOTOR_BACKWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_BACKWARD, OUTPUT);
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
  // Setup PWM channels
    ledcSetup(R, 5000, 8);  // Channel 0 for Motor A, 5 kHz frequency, 8-bit resolution
    ledcAttachPin(enA, L);
    ledcSetup(L, 5000, 8);  // Channel 0 for Motor B, 5 kHz frequency, 8-bit resolution
    ledcAttachPin(enB, R);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Wi-Fi...");
    } 
    Serial.println("Connected to Wi-Fi");
    Serial.println(WiFi.localIP());

  // Start server
    wifiserver.begin();
  }
}

void loop(){
  if(control == 0) {
    WiFiClient client = wifiserver.available();
    if (client) {
      while (client.connected()) {
        if (client.available()) {
          String command = client.readStringUntil('\n');
          command.trim();
          if(command == "FORWARD"){
            moveForward();
          } else if (command == "LEFT") {
            moveLeft();
          } else if (command == "RIGHT") {
            moveRight();
          } else if (command == "STOP") {
            stopMotors();
          } else if (command == "BACKWARD") {
            moveBackward();
          } else if (command == "DISTANCE") {
            digitalWrite(trigPin, LOW);
            delayMicroseconds(2);
            digitalWrite(trigPin, HIGH);
            delayMicroseconds(10);
            digitalWrite(trigPin, LOW);
  
            duration = pulseIn(echoPin, HIGH);
            distance = (duration / 2) * 0.0343;
  
            client.print("DIST:");
            client.println(distance);
          } else if (command == "Return") {
            moveLeft2();
            delay(3000);
          } else if (command == "MODE") {
            control = 1;
            setup();
            break;
          }
        }
      }
      client.stop();
  }
 }
 else if(control == 1){
  wsRobotArmInput.cleanupClients();
  if (playRecordedSteps)
  { 
    playRecordedRobotArmSteps();
  }
 }
}
