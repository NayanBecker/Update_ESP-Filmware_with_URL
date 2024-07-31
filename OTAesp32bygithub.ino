#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// Configurações de Wi-Fi
const char* ssid = "Nome_rede_Wifi";
const char* password = "Senha_Rede_Wifi";

// URL do firmware no GitHub (link direto para o binário)
const char* firmwareUrl = "https://raw.githubusercontent.com/NayanBecker/Update-Esp32-Firmware-by-OTA/main/sketch_jun27c.ino.bin";

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  checkForUpdates();
}

void loop() {

AtualizarTeste();


}


void AtualizarTeste(){
  Serial.print("Teste Atualização");
  
}

// Função para conectar ao Wi-Fi
void connectToWiFi() {
  Serial.println("Conectando ao Wifi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConectado ao Wifi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função para seguir redirecionamentos
String followRedirects(HTTPClient& http) {
  int httpCode = http.GET();
  int redirectCount = 0;
  const int maxRedirects = 5; // Número máximo de redirecionamentos para evitar loops infinitos

  while (httpCode == 302 && redirectCount < maxRedirects) {
    String newLocation = http.getLocation();
    Serial.printf("Redirecionando para: %s\n", newLocation.c_str());
    http.end(); // Fecha a conexão anterior
    http.begin(newLocation); // Abre a nova URL
    httpCode = http.GET(); // Faz a requisição GET novamente
    redirectCount++;
  }

  return http.getString(); // Retorna o conteúdo do firmware se o download for bem-sucedido
}

// Função para verificar e aplicar a atualização OTA
void checkForUpdates() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Verificando atualizacoes de firmware...");

    HTTPClient http;
    http.begin(firmwareUrl); // Inicia a conexão HTTP
    int httpCode = http.GET(); // Faz a requisição GET

    if (httpCode == HTTP_CODE_OK) {
      int len = http.getSize();
      WiFiClient* client = http.getStreamPtr();

      if (Update.begin(len)) {
        Serial.println("Baixando e instalando atualizacao de firmware...");

        size_t written = Update.writeStream(*client);
        if (written == len) {
          if (Update.end()) {
            if (Update.isFinished()) {
              Serial.println("Atualizacao bem-sucedida. Reiniciando...");
              ESP.restart();
            } else {
              Serial.println("Atualizacao nao concluida.");
            }
          } else {
            Serial.printf("Falha na atualizacao. Erro #: %d\n", Update.getError());
          }
        } else {
          Serial.println("Arquivo Invalido");
        }
      } else {
        Serial.println("Espaco insuficiente para iniciar a atualizacao");
      }
    } else if (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND || httpCode == HTTP_CODE_SEE_OTHER || httpCode == HTTP_CODE_TEMPORARY_REDIRECT || httpCode == HTTP_CODE_PERMANENT_REDIRECT) {
      String redirectedContent = followRedirects(http);
      if (redirectedContent.length() > 0) {
        Serial.println("Conteudo da atualizacao de firmware obtido apos o redirecionamento");
        // Aqui você pode usar a lógica para atualizar o firmware usando o conteúdo redirecionado
      } else {
        Serial.println("Falha ao obter atualizacao de firmware apos o redirecionamento");
      }
    } else {
      Serial.printf("Falha na solicitação HTTP com codigo %d\n", httpCode);
    }

    http.end(); // Fecha a conexão HTTP
  } else {
    Serial.println("Wifi não conectado");
  }
}
