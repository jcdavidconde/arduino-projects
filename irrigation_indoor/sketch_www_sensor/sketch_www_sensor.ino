#include  <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// Auth de la red WiFi
const String W_SSID = "la_cueva_de_ugarte";
const String W_PWD = "fr4nqui1to";

// Definimos pines de WiFi
const byte RX_PIN = 3;
const byte TX_PIN = 2;

// Sensor de suelo
const int SOIL_SENSOR_A = A3;

// Sensor de temperatura
const int TEMP_SENSOR_D = 4;

// Constante de delay de loop
const int STEP_MS = 500;

// Acumulador de loop
const int ac_ms_G = 0;

// Definimos el pin digital donde se conecta el sensor DHT11
#define DHTPIN TEMP_SENSOR_D
// Definimos el tipo de sensor
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// Inicializo controlador WiFi
SoftwareSerial SerialESP8266 (RX_PIN, TX_PIN);

void setup() {
  // Inicializamos comunicación serie
  Serial.begin(9600);
  // Inicializamos comunicación WiFi
  SerialESP8266.begin(115200); 
  int wifi_result = 0;
  while (wifi_result == 0) {
    wifi_result = initWiFi();
  }
  // Comenzamos el sensor DHT
  dht.begin();
}

int initWiFi() {
  //Verificamos si el ESP8266 responde
  SerialESP8266.println("AT");
  if(SerialESP8266.find("OK")) {
    Serial.println("Respuesta AT correcto");
  } else {
    Serial.println("Error en ESP8266");
    return 0;
  }
  
  //-----Configuración de red-------//Podemos comentar si el ESP ya está configurado

  //ESP8266 en modo estación (nos conectaremos a una red existente)
  SerialESP8266.println("AT+CWMODE=1");
  if(SerialESP8266.find("OK")) {
    Serial.println("ESP8266 en modo Estacion");
  } else {
    Serial.println("ESP8266 no se pudo determinar el modo");
    return 0;
  }
    
  //Nos conectamos a una red wifi 
  SerialESP8266.println("AT+CWJAP=\""+W_SSID+"\",\""+W_PWD+"\"");
  Serial.println("Conectandose a la red ...");
  SerialESP8266.setTimeout(10000); //Aumentar si demora la conexion
  if(SerialESP8266.find("OK")) {
    Serial.println("WIFI conectado");
  } else {
    Serial.println("Error al conectarse en la red");
    return 0;
  }
  SerialESP8266.setTimeout(2000);
  //Deshabilitamos las conexiones multiples
  SerialESP8266.println("AT+CIPMUX=0");
  if(SerialESP8266.find("OK")) {
    Serial.println("Multiconexiones deshabilitadas");
  } else {
    Serial.println("No se pudieron deshabilitar multiconexiones");
    return 0;
  }
  //------fin de configuracion-------------------
  return 1;
}

void loop() {
  delay(STEP_MS);
  
  // Leemos la humedad relativa
  float h_amb = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t_amb = dht.readTemperature();
  // Leemos humedad del suelo
  int h_soil = analogRead(SOIL_SENSOR_A);
}
