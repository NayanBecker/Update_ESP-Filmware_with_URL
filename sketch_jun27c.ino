#include <WiFi.h>
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <esp_log.h>
//#include <soc/rtc_wdt.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Update.h>


//const char *ssid = "3DSystemconection";
//const char *password = "System@3DConection";
const char *ssid = "Casa Becker";
const char *password = "902210947";


const char *serverName = "181.215.134.227";
const int serverPort = 3000;
int sensorName = 23;
String sensorLocation = "Rua Florianopolis,95E. ";
const float Vref = 3.3;
const long utcOffsetInSeconds = -10800;
WiFiUDP ntpUDP;

const char* firmwareUrl = "https://raw.githubusercontent.com/NayanBecker/Update-Esp32-Firmware-by-OTA/main/sketch_jun27c.ino.bin";

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

int entradaTensao = 35;
int sensorTampa = 36;
int sensorTemp = 39;
int sensornivel1 = 32;
int sensornivel2 = 33;
int sensorTampa2 = 25;
int sensorTemp2 = 26;
int sensornivel1_2 = 27;
int sensornivel2_2 = 14;
int releNivel = 16;
int releBateria = 19;
int relePainel = 23;
int releBombaO = 17;
int releBombaR = 18;

int lastStateTampa = LOW;
int lastStateTampa2 = LOW;
int lastStateTemp = LOW;
int lastStateTemp2 = LOW;
int currentStateTampa;
int currentStateTampa2;
int currentStateTemp;
int currentStateTemp2;
const float latitude = -27.110030;
const float longitude = -52.611819;
unsigned long previousTimeLevel = 0;
const unsigned long intervalLevel = 25000;
unsigned long sendDataStartTime = 0;
const unsigned long sendDataDelay = 10000;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 120000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi. Resetting...");
    ESP.restart();
  }
  timeClient.begin();
  timeClient.update();
  watchdogRTC();
  
  pinMode(releNivel, OUTPUT);
  pinMode(releBateria, OUTPUT);
  pinMode(relePainel, OUTPUT);
  pinMode(releBombaO, OUTPUT);
  pinMode(releBombaR, OUTPUT);
}

void loop() {
  timeClient.update();
  checkForUpdateCommand();
  //rtc_wdt_feed();

  pinMode(sensorTampa, INPUT);
  int sensorTampaValue = digitalRead(sensorTampa);
  currentStateTampa = digitalRead(sensorTampa);
  String tampa;
  tampa = sensorTampaValue == HIGH ? "Aberto" : "Fechado";

  pinMode(sensorTampa2, INPUT);
  int sensorTampaValue2 = digitalRead(sensorTampa2);
  currentStateTampa2 = digitalRead(sensorTampa2);
  String tampa2;
  tampa2 = sensorTampaValue2 == HIGH ? "Aberto" : "Fechado";

  pinMode(sensorTemp, INPUT);
  int sensorTempValue = digitalRead(sensorTemp);
  currentStateTemp = digitalRead(sensorTemp);
  String temp;
  temp = sensorTempValue == HIGH ? "Alerta" : "Normal";

  pinMode(sensorTemp2, INPUT);
  int sensorTempValue2 = digitalRead(sensorTemp2);
  currentStateTemp2 = digitalRead(sensorTemp2);
  String temp2;
  temp2 = sensorTempValue2 == HIGH ? "Alerta" : "Normal";

  pinMode(sensornivel1, INPUT);
  pinMode(sensornivel2, INPUT);
  int sensorNivel1Value = digitalRead(sensornivel1);
  int sensorNivel2Value = digitalRead(sensornivel2);
  String n1;
  n1 = (sensorNivel1Value && sensorNivel2Value == HIGH) ? "75%" : (sensorNivel1Value == HIGH) ? "50%"
                                                                : (sensorNivel2Value == HIGH) ? "100%"
                                                                                              : "<25%";

  pinMode(sensornivel1_2, INPUT);
  pinMode(sensornivel2_2, INPUT);
  int sensorNivel1Value2 = digitalRead(sensornivel1_2);
  int sensorNivel2Value2 = digitalRead(sensornivel2_2);
  String n2;
  n2 = (sensorNivel1Value2 && sensorNivel2Value2 == HIGH) ? "75%" : (sensorNivel1Value2 == HIGH) ? "50%"
                                                                : (sensorNivel2Value2 == HIGH) ? "100%"
                                                                                              : "<25%";

  String tensaoEntrada = String(analogRead(entradaTensao));
  String tensao = tensaoEntrada;


  if (lastStateTampa == HIGH && currentStateTampa == LOW) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  } else if (lastStateTampa == LOW && currentStateTampa == HIGH) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  }
  lastStateTampa = currentStateTampa;

  if (lastStateTampa2 == HIGH && currentStateTampa2 == LOW) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  } else if (lastStateTampa2 == LOW && currentStateTampa2 == HIGH) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  }
  lastStateTampa2 = currentStateTampa2;

  if (lastStateTemp == HIGH && currentStateTemp == LOW) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  } else if (lastStateTemp == LOW && currentStateTemp == HIGH) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  }
  lastStateTemp = currentStateTemp;

  if (lastStateTemp2 == HIGH && currentStateTemp2 == LOW) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  } else if (lastStateTemp2 == LOW && currentStateTemp2 == HIGH) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  }
  lastStateTemp2 = currentStateTemp2;


  float tensaoF = tensao.toFloat();
  checkAndControlRelay(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
  checkAndControlBatery();
  checkAndControlPannel();
  checkAndControlWater(tensaoF);
}

void watchdogRTC() {
  //rtc_wdt_protect_off();  //Disable RTC WDT write protection
  //Set stage 0 to trigger a system reset after 1000ms
  //rtc_wdt_set_stage(RTC_WDT_STAGE0, RTC_WDT_STAGE_ACTION_RESET_RTC);
  //rtc_wdt_set_time(RTC_WDT_STAGE0, 360000);
  //rtc_wdt_enable();      //Start the RTC WDT timer
  //rtc_wdt_protect_on();  //Enable RTC WDT write protection
}

void sendData(int sensorName, String sensorLocation, String tampa, String temp, String n1, String tampa2, String temp2, String n2, String tensao, float latitude, float longitude) {
  if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;

    if (client.connect(serverName, serverPort)) {
      Serial.println("Connected to server");

      String jsonData = "{\"idTrash\": " + String(sensorName) + ", \"address\": \"" + sensorLocation + "\", \"coverOrganic\": \"" + tampa + "\", \"temperatureOrganic\": \"" + temp + "\", \"levelOrganic\": \"" + n1 + "\", \"coverRecyclable\": \"" + tampa2 + "\", \"temperatureRecyclable\": \"" + temp2 + "\", \"levelRecyclable\": \"" + n2 + "\", \"tension\": \"" + tensao + "\", \"latitude\": " + String(latitude, 6) + ", \"longitude\": " + String(longitude, 6) + "}";
      Serial.println("Sending JSON data:");
      Serial.println(jsonData);

      client.print(String("POST ") + "/esp-backend/data" + " HTTP/1.1\r\n" + "Host: " + serverName + ":" + serverPort + "\r\n" + "Content-Type: application/json\r\n" + "Content-Length: " + jsonData.length() + "\r\n\r\n" + jsonData);
      Serial.println("POST request sent");
      Serial.println("Response from server:");
      while (client.connected()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
      client.stop();

    } else {
      Serial.print("Connection to server failed.");
    }
  } else {
    Serial.println("WiFi Disconnected");
    ESP.restart();
  }
}


void checkAndControlRelay(int sensorName, String sensorLocation, String tampa, String temp, String n1, String tampa2, String temp2, String n2, String tensao, float latitude, float longitude) {
  String formattedTime = timeClient.getFormattedTime();
  int hours = formattedTime.substring(0, 2).toInt();
  int minutes = formattedTime.substring(3, 5).toInt();
  int seconds = formattedTime.substring(6, 8).toInt();

  if (minutes == 0 && hours % 2 != 0 && seconds == 0) {
    digitalWrite(releNivel, HIGH);
    Serial.print("Ativou o rele");
  }
  if (minutes == 1 && hours % 2 != 0 && seconds == 0) {
    sendData(sensorName, sensorLocation, tampa, temp, n1, tampa2, temp2, n2, tensao, latitude, longitude);
    Serial.print("Enviou a requisição");
  }
   if (minutes == 2 && hours % 2 != 0 && seconds == 0) {
    digitalWrite(releNivel, LOW);
    Serial.print("Desativou o rele");
  }
}

void checkAndControlBatery() {
  String formattedTime = timeClient.getFormattedTime();
  int hours = formattedTime.substring(0, 2).toInt();
  int minutes = formattedTime.substring(3, 5).toInt();
  int seconds = formattedTime.substring(6, 8).toInt();

  if (hours == 8 && minutes == 0 && seconds == 0) {
    digitalWrite(releBateria, HIGH);
  }

  if (hours == 8 && minutes == 1 && seconds == 0) {
    digitalWrite(releBateria, LOW);
  }
}

void checkAndControlPannel() {
  String formattedTime = timeClient.getFormattedTime();
  int hours = formattedTime.substring(0, 2).toInt();
  int minutes = formattedTime.substring(3, 5).toInt();
  int seconds = formattedTime.substring(6, 8).toInt();

  if (hours == 8 && minutes == 0 && seconds == 0) {
    digitalWrite(relePainel, HIGH);
  }
  if (hours == 8 && minutes == 10 && seconds == 0) {
    digitalWrite(relePainel, LOW);
  }
}

void checkAndControlWater(float tensao) {
    String formattedTime = timeClient.getFormattedTime();
  int hours = formattedTime.substring(0, 2).toInt();
  int minutes = formattedTime.substring(3, 5).toInt();
  int seconds = formattedTime.substring(6, 8).toInt();

  if (hours == 12 && minutes == 0 && seconds == 0 && tensao > 11500) {
    digitalWrite(releBombaO, HIGH);
    digitalWrite(releBombaR, HIGH);
  }

  if (hours == 12 && minutes == 30 && seconds == 0) {
    digitalWrite(releBombaO, LOW);
    digitalWrite(releBombaR, LOW);
  }
}

String followRedirects(HTTPClient& http) {
  int httpCode = http.GET();
  int redirectCount = 0;
  const int maxRedirects = 5; 

  while (httpCode == 302 && redirectCount < maxRedirects) {
    String newLocation = http.getLocation();
    Serial.printf("Redirecionando para: %s\n", newLocation.c_str());
    http.end();
    http.begin(newLocation);
    httpCode = http.GET();
    redirectCount++;
  }

  return http.getString();
}

void checkForUpdateCommand() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');

    if (command.equals("update")) {
      checkForUpdates();
    } else {
      Serial.println("comado Invalido. teste 'update' ");
    }
  }
}


void checkForUpdates() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Verificando atualizações");

    HTTPClient http;
    http.begin(firmwareUrl); // Inicia a conexao HTTP
    int httpCode = http.GET(); // Faz a requisiçao GET

    if (httpCode == HTTP_CODE_OK) {
      int len = http.getSize();
      WiFiClient* client = http.getStreamPtr();

      if (Update.begin(len)) {
        Serial.println("Baixando e Instalando Atualizações");

        size_t written = Update.writeStream(*client);
        if (written == len) {
          if (Update.end()) {
            if (Update.isFinished()) {
              Serial.println("Atualização bem-sucedida!, Reiniciando....");
              ESP.restart();
            } else {
              Serial.println("Falha ao Reiniciar");
            }
          } else {
            Serial.printf("Falha na atualização: %d\n", Update.getError());
          }
        } else {
          Serial.println("Falha. bytes do Arquivo não conhecidos");
        }
      } else {
        Serial.println("Espaço insuficiente");
      }
    } else if (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND || httpCode == HTTP_CODE_SEE_OTHER || httpCode == HTTP_CODE_TEMPORARY_REDIRECT || httpCode == HTTP_CODE_PERMANENT_REDIRECT) {
      String redirectedContent = followRedirects(http);
      if (redirectedContent.length() > 0) {
        Serial.println("Conteúdo da atualização Invalido");
      
      } else {
        Serial.println("Falha ao obter atualização");
      }
    } else {
      Serial.printf("Falha na solicitação HTTP com código %d\n", httpCode);
    }

    http.end();
  } else {
    Serial.println("Wi-Fi não conectado");
  }
}