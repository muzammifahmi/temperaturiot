#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>

const char *ssid = "OPPO A31"; //your wifi name
const char *password = "00000000"; passowrd wifi

WebServer server(80);
DHT dht(26, DHT11);

String temperatureData = ""; // To store temperature data
String humidityData = "";   // To store humidity data

unsigned long previousMillis = 0; // Store last time reading was updated
const long interval = 5000;       // Interval to read data (5 seconds)

int countComma(const String &data) {
  int count = 0;
  for (int i = 0; i < data.length(); i++) {
    if (data[i] == ',') count++;
  }
  return count + 1; // Return number of elements
}

void handleRoot() {
  char msg[5000];

  snprintf(msg, 5000,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.0-alpha1/dist/css/bootstrap.min.css' rel='stylesheet'>\
    <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>\
    <title>ESP32 DHT Server</title>\
  </head>\
  <body class='bg-light text-center'>\
    <div class='container py-5'>\
      <h2 class='mb-4 text-primary'>Monitoring Suhu dan Kelembapan Ruangan</h2>\
      <div class='row'>\
        <div class='col-md-6 offset-md-3'>\
          <div class='card shadow'>\
            <div class='card-body'>\
              <h4 class='card-title'>Temperature</h4>\
              <p class='card-text text-danger'><i class='fas fa-thermometer-half'></i> %.2f &deg;C</p>\
              <h4 class='card-title'>Humidity</h4>\
              <p class='card-text text-info'><i class='fas fa-tint'></i> %.2f &percnt;</p>\
            </div>\
          </div>\
        </div>\
      </div>\
      <canvas id='chart' width='400' height='200'></canvas>\
    </div>\
    <script>\
      const ctx = document.getElementById('chart').getContext('2d');\
      new Chart(ctx, {\
        type: 'bar',\
        data: {\
          labels: Array.from({length: %d}, (_, i) => i * 5 + ' s'),\
          datasets: [\
            {\
              label: 'Temperature (°C)',\
              data: [%s],\
              backgroundColor: 'rgba(255, 99, 132, 0.2)',\
              borderColor: 'rgba(255, 99, 132, 1)',\
              borderWidth: 1\
            },\
            {\
              label: 'Humidity (%)',\
              data: [%s],\
              backgroundColor: 'rgba(54, 162, 235, 0.2)',\
              borderColor: 'rgba(54, 162, 235, 1)',\
              borderWidth: 1\
            }\
          ]\
        },\
        options: {\
          responsive: true,\
          scales: {\
            x: {\
              title: {\
                display: true,\
                text: 'Time (s)'\
              }\
            },\
            y: {\
              title: {\
                display: true,\
                text: 'Value'\
              },\
              beginAtZero: true\
            }\
          }\
        }\
      });\
    </script>\
  </body>\
</html>",
           readDHTTemperature(), readDHTHumidity(),
           countComma(temperatureData), temperatureData.c_str(), humidityData.c_str());

  server.send(200, "text/html", msg);
}

void setup(void) {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read temperature and humidity
    float temp = readDHTTemperature();
    float hum = readDHTHumidity();

    // Append data only if the reading is valid
    if (temp != -1 && hum != -1) {
      if (temperatureData.length() == 0) {
        temperatureData += String(temp);
        humidityData += String(hum);
      } else {
        temperatureData += "," + String(temp);
        humidityData += "," + String(hum);
      }

      // Limit data points to 12 (1 minute at 5-second intervals)
      if (countComma(temperatureData) > 12) {
        temperatureData = temperatureData.substring(temperatureData.indexOf(',') + 1);
        humidityData = humidityData.substring(humidityData.indexOf(',') + 1);
      }
    }
  }
}

float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  return t;
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  return h;
}
