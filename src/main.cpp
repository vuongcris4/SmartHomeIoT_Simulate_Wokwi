#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>

// Cấu hình WiFi
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// Cấu hình Firebase
#define FIREBASE_HOST "https://smarthomeiot-f2a2a-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyCYVzO92NAQBy4LGDjJJECC_ve_QTybA38"

// Cấu hình DHT
#define DHTPIN 14       // Chân của cảm biến DHT nối vào GPIO 14
#define DHTTYPE DHT22   // Chọn loại cảm biến, DHT11 hoặc DHT22

// Chân của cảm biến ánh sáng
#define LIGHT_SENSOR_PIN 33  // Chân analog của cảm biến ánh sáng nối vào GPIO 33

// Chân của các LED (thay thế cho relay)
#define LED_AIRCON_PIN 17   // LED cho điều hòa
#define LED_FAN_PIN 4     // LED cho quạt
#define LED_LIGHT_PIN 16   // LED cho đèn

// Khởi tạo các đối tượng
DHT dht(DHTPIN, DHTTYPE);
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void setup() {
  Serial.begin(115200);

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  // Cấu hình Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  
  // Khởi tạo Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Bắt đầu cảm biến DHT
  dht.begin();

  // Cấu hình các chân LED là OUTPUT
  pinMode(LED_AIRCON_PIN, OUTPUT);
  pinMode(LED_FAN_PIN, OUTPUT);
  pinMode(LED_LIGHT_PIN, OUTPUT);

  // Tắt tất cả LED ban đầu
  digitalWrite(LED_AIRCON_PIN, LOW);
  digitalWrite(LED_FAN_PIN, LOW);
  digitalWrite(LED_LIGHT_PIN, LOW);
}

void loop() {
  // Đọc nhiệt độ từ cảm biến DHT
  float temperature = dht.readTemperature();

  // Kiểm tra nếu đọc thành công
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Đọc giá trị ánh sáng từ cảm biến ánh sáng
  int lightLevel = analogRead(LIGHT_SENSOR_PIN);
  // float lightVoltage = lightLevel * (3.3 / 4095.0); // Chuyển đổi sang điện áp nếu cần
  float lightVoltage = lightLevel - 4095;

  // In ra màn hình serial
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C ");
  Serial.print("Light level: ");
  Serial.print(lightLevel);
  Serial.print(" (Voltage: ");
  Serial.print(lightVoltage);
  Serial.println(" V)");

  // Gửi nhiệt độ lên Firebase
  if (Firebase.setFloat(firebaseData, "bedroom/sensors/temperature/value", temperature)) {
    Serial.println("Temperature sent successfully");
  } else {
    Serial.print("Failed to send temperature: ");
    Serial.println(firebaseData.errorReason());
  }

  // Gửi giá trị ánh sáng lên Firebase
  if (Firebase.setInt(firebaseData, "bedroom/sensors/light/value", lightLevel)) {
    Serial.println("Light level sent successfully");
  } else {
    Serial.print("Failed to send light level: ");
    Serial.println(firebaseData.errorReason());
  }

  // Đọc trạng thái của các thiết bị từ Firebase và điều khiển LED
  int airConStatus = 0, fanStatus = 0, lightStatus = 0;

  if (Firebase.getInt(firebaseData, "bedroom/devices/airConditioner/status")) {
    airConStatus = firebaseData.intData();
    digitalWrite(LED_AIRCON_PIN, airConStatus == 1 ? HIGH : LOW);
    Serial.print("Air Conditioner LED: ");
    Serial.println(airConStatus ? "ON" : "OFF");
  } else {
    Serial.print("Failed to get air conditioner status: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.getInt(firebaseData, "bedroom/devices/fan/status")) {
    fanStatus = firebaseData.intData();
    digitalWrite(LED_FAN_PIN, fanStatus == 1 ? HIGH : LOW);
    Serial.print("Fan LED: ");
    Serial.println(fanStatus ? "ON" : "OFF");
  } else {
    Serial.print("Failed to get fan status: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.getInt(firebaseData, "bedroom/devices/light/status")) {
    lightStatus = firebaseData.intData();
    digitalWrite(LED_LIGHT_PIN, lightStatus == 1 ? HIGH : LOW);
    Serial.print("Light LED: ");
    Serial.println(lightStatus ? "ON" : "OFF");
  } else {
    Serial.print("Failed to get light status: ");
    Serial.println(firebaseData.errorReason());
  }

  // Chờ một khoảng thời gian trước khi gửi lần tiếp theo
  delay(2000);
}
