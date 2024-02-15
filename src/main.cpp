#include <WiFi.h>
#include <TelnetStream.h>
#include <HTTPClient.h>
#include <Update.h>

#define DEBUG
#ifdef DEBUG
  #define _SP(x)  Serial.print(x)
  #define _SPL(x)  Serial.println(x)
#else
  #define _SP(x)
  #define _SPL(x)
#endif

const char* mySSID = "MGTS_GPON_U9nG";
const char* myPASSWORD = "JwYZ4UHJ";

uint32_t g_full_length = 0;
uint32_t g_curr_length = 0;

void updateFlash(uint8_t* data, size_t len)
{
    // Запись массива в Flash
    Update.write(data, len);
    // Обновление счётчика записсаного размера
    g_curr_length += len;

    // Вывод на дисплей точек на дисплей
    static int count = 0;
    count++;
    if (count % 1024 == 0){_SP(".");};

    // ранний выход из функции, если записаны не все данные
    if (g_curr_length != g_full_length)
        return;

    // завершение обновления
    Update.end(true);
}

void updateFirmware(HTTPClient& client) {
    // хранение части полученных данных
    uint8_t buff[128]{};
    // получение информации о размере файла
    g_full_length = client.getSize();
    // размер для декрементации в цикле
    int len = g_full_length;
    // начинаем обновление
    Update.begin(UPDATE_SIZE_UNKNOWN);
    // получаем указатель на поток данных
    WiFiClient* stream = client.getStreamPtr();

    while (client.connected() && (len > 0 || len == -1)) {
        // пока данные есть - записываем поэтапно в Flash память
        size_t size = stream->available();
        if (size) {
            int bytes_read = stream->readBytes(buff, ((size > sizeof(buff))?sizeof(buff):size));
            updateFlash(buff, bytes_read);
            if (len > 0)
                len -= bytes_read;
        }
        delay(1);
    }
}

void update(String url){
    // запрос на сервер
    HTTPClient client;
    client.begin(url);

    // если запрос удачный - обновляем раздел новым кодом
    int httpcode = client.GET();
    if (httpcode == HTTP_CODE_OK) {
        // disp.clear();
        // disp.print("FW UPDATE..");
        _SPL("FW UPDATE..");
        updateFirmware(client);
    }
    else {
        // disp.clear();
        // disp.printf("HTTP: %d", httpcode);
        Serial.printf("HTTP: %d", httpcode);
        return;
    }

    client.end();

    // перезагрузка
    ESP.restart();
}

void taskLive(void *TaskParameters_t){
  while(1){
    TelnetStream.println(millis());
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void setup(){
  Serial.begin(115200);
  Serial.setTimeout(10);

  _SPL("connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(mySSID, myPASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    _SP(".");
    delay(500);
  }
  _SP("\nIP: ");
  _SPL(WiFi.localIP());
  TelnetStream.begin();

  xTaskCreate(taskLive, "Live", 2048, NULL, 1, NULL);
}

void loop(){
  if(Serial.available()){
    String url = Serial.readString();
    _SPL(url);
    update(url);
  }
}

// #define ESP32_RTOS

// #include "OTA.h"
// // #include <credentials.h>

// uint32_t entry;

// const char* mySSID = "MGTS_GPON_U9nG";
// const char* myPASSWORD = "JwYZ4UHJ";
// const char* hostname = "JwYZ4UHJ";

// void setup() {
//   Serial.begin(115200);
//   Serial.println("Booting");

//   setupOTA(hostname, mySSID, myPASSWORD);

//   // Your setup code
// }

// void loop() {
//   // entry = micros();
// // #ifdef defined(ESP32_RTOS) && defined(ESP32)
// // #else // If you do not use FreeRTOS, you have to regulary call the handle method.
//   // ArduinoOTA.handle();
// // #endif
//   // TelnetStream.println(micros()-entry);
//   TelnetStream.println(millis());
//   delay(1000);
//   // Your code here
// }


//elegant OTA
// #if defined(ESP8266)
//   #include <ESP8266WiFi.h>
//   #include <WiFiClient.h>
//   #include <ESP8266WebServer.h>
// #elif defined(ESP32)
//   #include <WiFi.h>
//   #include <WiFiClient.h>
//   #include <WebServer.h>
// #elif defined(TARGET_RP2040)
//   #include <WiFi.h>
//   #include <WebServer.h>
// #endif

// #include <ElegantOTA.h>

// const char* ssid = "MGTS_GPON_U9nG";
// const char* password = "JwYZ4UHJ";

// #if defined(ESP8266)
//   ESP8266WebServer server(80);
// #elif defined(ESP32)
//   WebServer server(80);
// #elif defined(TARGET_RP2040)
//   WebServer server(80);
// #endif

// unsigned long ota_progress_millis = 0;

// void onOTAStart() {
//   // Log when OTA has started
//   Serial.println("OTA update started!");
//   // <Add your own code here>
// }

// void onOTAProgress(size_t current, size_t final) {
//   // Log every 1 second
//   if (millis() - ota_progress_millis > 1000) {
//     ota_progress_millis = millis();
//     Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
//   }
// }

// void onOTAEnd(bool success) {
//   // Log when OTA has finished
//   if (success) {
//     Serial.println("OTA update finished successfully!");
//   } else {
//     Serial.println("There was an error during OTA update!");
//   }
//   // <Add your own code here>
// }

// void setup(void) {
//   Serial.begin(115200);
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   Serial.println("");

//   // Wait for connection
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.print("Connected to ");
//   Serial.println(ssid);
//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());

//   server.on("/", []() {
//     server.send(200, "text/plain", "Hi! This is ElegantOTA Demo.");
//   });

//   ElegantOTA.begin(&server);    // Start ElegantOTA
//   // ElegantOTA callbacks
//   ElegantOTA.onStart(onOTAStart);
//   ElegantOTA.onProgress(onOTAProgress);
//   ElegantOTA.onEnd(onOTAEnd);

//   server.begin();
//   Serial.println("HTTP server started");
// }

// void loop(void) {
//   server.handleClient();
//   ElegantOTA.loop();
// }