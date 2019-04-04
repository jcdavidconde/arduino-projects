#include  <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Bomba de agua
int WATER_PUMP_D = 13;

// Corriente Display LCD
int LCD_POS_PIN = 12;
int LCD_NEG_PIN = 11;

//Sensor de suelo
//int SOIL_SENSOR_POS_PIN = 8;
//int SOIL_SENSOR_NEG_PIN = A3;
int SOIL_SENSOR_D = 5;
int SOIL_SENSOR_A = A3;

// Sensor de temperatura
int TEMP_SENSOR_POS_PIN = 2;
int TEMP_SENSOR_NEG_PIN = 4;
int TEMP_SENSOR_D = 3;

// Sensor de agua
int WATER_SENSOR_POS_PIN = 9;
int WATER_SENSOR_NEG_PIN = 10;
int WATER_SENSOR_A = A2;

// Variables de tiempo
int STEP_MS = 500;
int ac_ms = 0;

// Definimos el pin digital donde se conecta el sensor DHT11
#define DHTPIN TEMP_SENSOR_D
// Definimos el tipo de sensor
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// Constructor de la librería de LCD 16x2
// Aqui se configuran los pines asignados a la pantalla del PCF8574
LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  // Pin de entrada de datos digital del sensor del suelo
  pinMode(SOIL_SENSOR_D, INPUT);
  
  // Pin de activación de bomba de agua
  pinMode(WATER_PUMP_D,OUTPUT);
  
  // Pin positivo de display LCD
  pinMode(LCD_POS_PIN,OUTPUT);
  digitalWrite(LCD_POS_PIN, HIGH);
  // Pin negativo de display LCD
  pinMode(LCD_NEG_PIN,OUTPUT);
  digitalWrite(LCD_NEG_PIN, LOW);
  
  // Pin positivo de sensor de temperatura
  pinMode(TEMP_SENSOR_POS_PIN,OUTPUT);
  digitalWrite(TEMP_SENSOR_POS_PIN, HIGH);
  // Pin negativo de sensor de temperatura
  pinMode(TEMP_SENSOR_NEG_PIN,OUTPUT);
  digitalWrite(TEMP_SENSOR_NEG_PIN, LOW);
  
  // Pin positivo de sensor de agua
  pinMode(WATER_SENSOR_POS_PIN,OUTPUT);
  digitalWrite(WATER_SENSOR_POS_PIN, HIGH);
  // Pin negativo de sensor de temperatura
  pinMode(WATER_SENSOR_NEG_PIN,OUTPUT);
  digitalWrite(WATER_SENSOR_NEG_PIN, LOW);

  // Inicializamos comunicación serie
  Serial.begin(9600);
  
  // Comenzamos el sensor DHT
  dht.begin();

  // Inicializo LCD
  lcd.init();
  //Encender la luz de fondo.
  lcd.backlight();
  //Ubico cursor en (0,0)
  lcd.home();
  lcd.print("POPOFFFFFF");
  lcd.setCursor(0,1);
  lcd.print("INITIALIZED");
}

void loop() {
  prenderBomba();
  delay(STEP_MS);
  ac_ms += STEP_MS;
  if (ac_ms % 15000 == 0) {
    displayAgua();
    ac_ms = 0;
  } else if (ac_ms % 10000 == 0) {
    displaySuelo();
  } else if (ac_ms % 5000 == 0) {
    displayAmbiente();
  }
}

void prenderBomba() {
  // Leemos humedad del suelo
  int soil_humidity_val = digitalRead(SOIL_SENSOR_D);
  if (soil_humidity_val == LOW) {
    // Bomba apagada
    digitalWrite(WATER_PUMP_D, LOW);
  } else {
    // Bomba prendida
    digitalWrite(WATER_PUMP_D, HIGH);
  }
}

void displayAgua() {
  // Leemos nivel del agua
  int w_s = analogRead(WATER_SENSOR_A);
  String water_level;

  if (w_s >= 400) {
    water_level = "Alto";
  } else if (w_s >= 100) {
    water_level = "Medio";
  } else {
    water_level = "Bajo";
  }

  Serial.print("Water: ");
  Serial.print(w_s);
  Serial.print("\n");

  lcd.clear();
  lcd.home();
  lcd.print("Nivel Agua ");
  lcd.print(water_level);

  lcd.setCursor(0, 1);
  lcd.print("Indice: ");
  lcd.print(w_s);
}

void displaySuelo() {
  // Leemos humedad del suelo
  int h_s = analogRead(SOIL_SENSOR_A);
  int soil_humidity_val = digitalRead(SOIL_SENSOR_D);

  Serial.print("Soil: ");
  Serial.print(h_s);
  Serial.print("\n");

  lcd.clear();
  lcd.home();
  lcd.print("Suelo ");
  lcd.print(soil_humidity_val == LOW ? "Humedo" : "Seco");

  lcd.setCursor(0, 1);
  lcd.print("Indice: ");
  lcd.print(h_s);
}

void displayAmbiente() {
  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t)) {
    Serial.println("Error del sensor DHT11");
    return;
  }
 
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);
 
  Serial.print("Hum: ");
  Serial.print(h);
  Serial.print("%\t");
  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print("^C\t");
  Serial.print("Heat Index: ");
  Serial.print(hic);
  Serial.print("^C\n");

  lcd.clear();
  lcd.home();
  lcd.print("Humedad: ");
  lcd.print(h);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print("^C");
}
