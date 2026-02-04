#include <WiFi.h>

#include <WebServer.h>



const char* ssid = "Breach";

const char* password = "12345678";



WebServer server(80);



// Finger input pins (pull-up)

const int fingerPins[] = {13, 12, 14, 27};

const int weights[]   = {1, 2, 4, 8};



// Gesture timing & state

unsigned long pressStartTime = 0;

int lastDetectedID = 0;

int confirmedID = 0;

bool gestureLocked = false;



const unsigned long holdTimeThreshold = 1000; // 1 second



// ---------------- HTML PAGE ----------------

String getHTML() {

  String html = "<!DOCTYPE html><html><head>";

  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";

  html += "<style>";

  html += "body{font-family:'Segoe UI',sans-serif;text-align:center;background:#000;color:#fff;";

  html += "display:flex;flex-direction:column;justify-content:center;height:100vh;margin:0}";

  html += ".sentence{font-size:90px;font-weight:bold;color:#00d1b2;padding:20px;text-transform:uppercase}";

  html += ".progress-container{width:70%;height:15px;background:#222;margin:40px auto;";

  html += "border-radius:20px;overflow:hidden;border:1px solid #444}";

  html += ".progress-bar{width:0%;height:100%;background:linear-gradient(90deg,#00d1b2,#00ffcc);";

  html += "transition:width 0.1s}";

  html += ".status{font-size:18px;color:#777;letter-spacing:2px;text-transform:uppercase}";

  html += "</style></head><body>";



  html += "<div class='status' id='stat'>System Ready</div>";

  html += "<div class='sentence' id='textDisp'>WAITING</div>";

  html += "<div class='progress-container'><div class='progress-bar' id='bar'></div></div>";



  html += "<script>";

  html += "let progress=0,lastID=0;";

  html += "const map={1:'HELLO',2:'THANKS',3:'WATER',4:'YES',5:'HUNGRY',6:'HELP',7:'DOCTOR',";

  html += "8:'NO',9:'TIRED',10:'SLEEP',11:'LIGHT',12:'HOT',13:'MEDICINE',14:'OPEN DOOR',15:'EMERGENCY'};";



  html += "function speak(t){speechSynthesis.cancel();";

  html += "let u=new SpeechSynthesisUtterance(t);u.rate=1.0;speechSynthesis.speak(u);}";



  html += "setInterval(()=>{";

  html += "fetch('/getWeight').then(r=>r.text()).then(id=>{";

  html += "id=parseInt(id);let bar=document.getElementById('bar');";

  html += "let txt=document.getElementById('textDisp');";

  html += "let st=document.getElementById('stat');";



  html += "if(id>0){st.innerHTML='Detecting Gesture';";

  html += "if(progress<100)progress+=10;bar.style.width=progress+'%';";

  html += "if(progress>=100 && id!==lastID){";

  html += "let t=map[id]||'UNKNOWN';txt.innerHTML=t;txt.style.color='#fff';";

  html += "speak(t);lastID=id;}}";

  html += "else{progress=0;bar.style.width='0%';";

  html += "txt.innerHTML='WAITING';txt.style.color='#00d1b2';";

  html += "st.innerHTML='System Idle';lastID=0;}";

  html += "});},100);";

  html += "</script></body></html>";



  return html;

}



// ---------------- SERVER HANDLER ----------------

void handleWeight() {

  server.send(200, "text/plain", String(confirmedID));

}



// ---------------- SETUP ----------------

void setup() {

  Serial.begin(115200);

  delay(1000);



  for (int i = 0; i < 4; i++) {

    pinMode(fingerPins[i], INPUT_PULLUP);

  }



  Serial.println();

  Serial.println("Connecting to WiFi...");

  Serial.print("SSID: ");

  Serial.println(ssid);



  WiFi.begin(ssid, password);



  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

    attempts++;

    if (attempts > 20) {

      Serial.println("\nWiFi connection FAILED");

      return;

    }

  }



  Serial.println("\nWiFi Connected");

  Serial.print("ESP32 IP Address: ");

  Serial.println(WiFi.localIP());



  server.on("/", []() {

    server.send(200, "text/html", getHTML());

  });



  server.on("/getWeight", handleWeight);



  server.begin();

  Serial.println("Web Server Started");

}



// ---------------- LOOP ----------------

void loop() {

  server.handleClient();



  int currentID = 0;

  for (int i = 0; i < 4; i++) {

    if (digitalRead(fingerPins[i]) == LOW) {

      currentID += weights[i];

    }

  }



  // No fingers pressed → reset

  if (currentID == 0) {

    pressStartTime = 0;

    lastDetectedID = 0;

    confirmedID = 0;

    gestureLocked = false;

    return;

  }



  // New gesture detected

  if (currentID != lastDetectedID) {

    pressStartTime = millis();

    lastDetectedID = currentID;

    gestureLocked = false;

    return;

  }



  // Stable hold

  if (!gestureLocked && millis() - pressStartTime >= holdTimeThreshold) {

    confirmedID = currentID;

    gestureLocked = true;

    Serial.print("Gesture Confirmed: ");

    Serial.println(confirmedID);

  }

}
