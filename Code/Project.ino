#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "Bnaya";
const char* password = "";

WebServer server(80);

int humiditySum = 0;
int humidityCount = 0;



void handleRoot() {
  String html = "<html><head>";
  html += "<style>";
  html += "body {";
  html += "  font-family: Arial, sans-serif;";
  html += "  display: flex;";
  html += "  justify-content: center;";
  html += "  align-items: center;";
  html += "  height: 100vh;";
  html += "  background: url('https://i.pinimg.com/564x/3f/cb/38/3fcb3884bddd23e58545e1ce5b74cd6b.jpg') center/cover rgba(0,0,0,0.5) no-repeat;";
  html += "}";
  html += ".container {";
  html += "  text-align: center;";
  html += "}";
  html += "h1 {";
  html += "  color: #000000;";
  html += "  font-size: 7em;"; // Increased font size
  html += "  font-family: Calibri, sans-serif;";
  html += "}";
  html += "h2 {";
  html += "  color: #000000;";
  html += "  font-size: 4em;";
  html += "}";
  html += "select {";
  html += "  padding: 10px;";
  html += "  font-size: 3em;";
  html += "}";
  html += "input[type='submit'] {";
  html += "  background-color: #4CAF50;";
  html += "  color: white;";
  html += "  padding: 30px 60px;";
  html += "  border: none;";
  html += "  cursor: pointer;";
  html += "  font-size: 3em;";
  html += "}";
  html += "#average, #result {";
  html += "  font-size: 2.5em;";
  html += "}";
  html += "#startWatering {";
  html += "  padding: 40px 80px;";
  html += "  font-size: 2.5em;";
  html += "}";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1 style='font-family: \"Calibri\", cursive, sans-serif;'>Soil Thermometer</h1>"; // Updated h1 with custom font
  html += "<h2>Choose pot size:</h2>";
  html += "<form action='/measurements'>";
  html += "<select name='potSize'>";
  html += "<option value='small'>Small</option>";
  html += "<option value='medium'>Medium</option>";
  html += "<option value='large'>Large</option>";
  html += "</select>";
  html += "<br><br>";
  html += "<h2>Choose vegetation type:</h2>";
  html += "<select name='vegetationType'>";
  html += "<option value='crops/tropical'>Crops/Tropical</option>";
  html += "<option value='durable crops'>Durable Crops</option>";
  html += "<option value='succulents'>Succulents</option>";
  html += "<option value='cacti'>Cacti</option>";
  html += "</select>";
  html += "<br><br>";
  html += "<input type='submit' value='Take Measurements'>";
  html += "</form>";
  html += "<p id='average'></p>";
  html += "<p id='result'></p>";
  html += "<button id='startWatering' disabled>Start Watering</button>";
  html += "<script>";
  html += "document.querySelector('form').addEventListener('submit', function(e) {";
  html += "  e.preventDefault();";
  html += "  var formData = new FormData(e.target);";
  html += "  var potSize = formData.get('potSize');";
  html += "  var vegetationType = formData.get('vegetationType');";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/measurements?potSize=' + encodeURIComponent(potSize) + '&vegetationType=' + encodeURIComponent(vegetationType), true);";
  html += "  xhr.onload = function() {";
  html += "    if (xhr.status === 200) {";
  html += "      var response = JSON.parse(xhr.responseText);";
  html += "      document.getElementById('average').textContent = 'Average Humidity: ' + response.average;";
  html += "      document.getElementById('result').textContent = 'Result: ' + response.result;";
  html += "      document.getElementById('startWatering').disabled = response.result === 'Bad' ? false : true;";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "});";
  html += "document.getElementById('startWatering').addEventListener('click', function(e) {";
  html += "  e.preventDefault();";
  html += "  var potSize = document.querySelector('select[name=potSize]').value;";
  html += "  var wateringTime = 0;";
  html += "  if (potSize === 'small') {";
  html += "    wateringTime = 10;";
  html += "  } else if (potSize === 'medium') {";
  html += "    wateringTime = 15;";
  html += "  } else if (potSize === 'large') {";
  html += "    wateringTime = 20;";
  html += "  }";
  html += "  if (wateringTime > 0) {";
  html += "    var xhr = new XMLHttpRequest();";
  html += "    xhr.open('GET', '/startWatering?duration=' + wateringTime, true);";
  html += "    xhr.onload = function() {";
  html += "      if (xhr.status === 200) {";
  html += "        alert(xhr.responseText);";
  html += "      }";
  html += "    };";
  html += "    xhr.send();";
  html += "  }";
  html += "});";
  html += "</script>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}



void handleStartWatering() {
  String durationString = server.arg("duration");
  int duration = durationString.toInt();
  
  if (duration > 0) {
    digitalWrite(A4, HIGH); // Activate watering pin
    delay(duration * 1000); // Delay in milliseconds
    digitalWrite(A4, LOW); // Deactivate watering pin
    server.send(200, "text/plain", "Watering completed");
  } else {
    server.send(400, "text/plain", "Invalid watering duration");
  }
}



void handleMeasurements() {
  String potSize = server.arg("potSize");
  String vegetationType = server.arg("vegetationType");

  humiditySum = 0;
  humidityCount = 0;

  for (int i = 0; i < 10; i++) {
    float h = analogRead(A0);
    if (!isnan(h)) {
      humiditySum += h;
      humidityCount++;
    }
    delay(100); // Delay between measurements
  }

  float averageHumidity = humiditySum * 0.07 / humidityCount;
  String result;

  if (vegetationType == "crops/tropical") {
    if (averageHumidity >= 20) {
      result = "Good";
    } else {
      result = "Bad";
    }
  } else if (vegetationType == "durable crops") {
    if (averageHumidity >= 15) {
      result = "Good";
    } else {
      result = "Bad";
    }
  } else if (vegetationType == "succulents") {
    if (averageHumidity >= 10) {
      result = "Good";
    } else {
      result = "Bad";
    }
  } else if (vegetationType == "cacti") {
    if (averageHumidity >= 5) {
      result = "Good";
    } else {
      result = "Bad";
    }
  }

  DynamicJsonDocument jsonDoc(256);
  jsonDoc["average"] = averageHumidity;
  jsonDoc["result"] = result;

  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  server.send(200, "application/json", jsonResponse);
}


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(A4, OUTPUT);
  digitalWrite(A4, LOW);

  server.on("/", handleRoot);
  server.on("/measurements", handleMeasurements);
  server.on("/startWatering", handleStartWatering);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
