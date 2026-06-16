#include <QMC5883LCompass.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Objeto Brújula
QMC5883LCompass compass;

// Pines para los LEDs (Indicadores de giro)
const int LED_IZQ = 25; 
const int LED_DER = 26; 

// Identificadores Únicos (UUIDs) para el servicio BLE
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Para enviar el Azimut al celular
#define CHARACTERISTIC_UUID_RX "12345678-1234-5678-1234-56789abcdef0" // Para recibir 'I' o 'D' del celular

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;

// Variables de tiempo asíncrono
unsigned long ultimoEnvio = 0;
const int INTERVALO_ENVIO = 500;

// Callbacks para gestionar la conexión
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("¡Dispositivo conectado!");
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Dispositivo desconectado.");
      pServer->getAdvertising()->start(); // Reiniciar el anuncio si se desconecta
    }
};

// Callbacks para recibir datos desde la Web App
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue();
      if (value.length() > 0) {
        char comando = value[0]; // Leer la primera letra
        
        if (comando == 'I' || comando == 'i') {
          Serial.println("Comando: IZQUIERDA");
          digitalWrite(LED_IZQ, HIGH);
          delay(400); 
          digitalWrite(LED_IZQ, LOW);
        } else if (comando == 'D' || comando == 'd') {
          Serial.println("Comando: DERECHA");
          digitalWrite(LED_DER, HIGH);
          delay(400);
          digitalWrite(LED_DER, LOW);
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_IZQ, OUTPUT);
  pinMode(LED_DER, OUTPUT);

  compass.init();
  
  // Configuración del Servidor BLE
  BLEDevice::init("NaviNode_BLE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Característica TX (ESP32 -> Celular)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Característica RX (Celular -> ESP32)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Servidor BLE encendido. Esperando conexión...");
}

void loop() {
  if (deviceConnected && (millis() - ultimoEnvio > INTERVALO_ENVIO)) {
    ultimoEnvio = millis();
    
    compass.read();
    int azimut = compass.getAzimuth();
    if (azimut < 0) azimut = 360 + azimut;

    // Convertir el entero a String y enviarlo por notificación BLE
    String valorAzimut = String(azimut);
    pTxCharacteristic->setValue(valorAzimut.c_str());
    pTxCharacteristic->notify();
  }
}