#include  <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Bomba de agua
int WATER_PUMP_D = 13;

//Sensor de suelo
int SOIL_SENSOR_D = 5;
int SOIL_SENSOR_A = A3;

// Sensor de temperatura
int TEMP_SENSOR_D = 3;

// Sensor de agua (Ultrasonico)
int WATER_SENSOR_ECHO_PIN = 9;
int WATER_SENSOR_TRIG_PIN = 8;


// Profundidad de recipiente de agua en centimetros
int WATER_RECIP_H = 14;

// Constante de delay de loop
int STEP_MS = 500;

// Acumulador de loop
int ac_ms_G = 0;

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

  // Inicializo pines de ultrasonido
  pinMode(WATER_SENSOR_TRIG_PIN, OUTPUT);
  pinMode(WATER_SENSOR_ECHO_PIN, INPUT);

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
  ac_ms_G += STEP_MS;
  if (ac_ms_G % 15000 == 0) {
    displayAgua();
    ac_ms_G = 0;
  } else if (ac_ms_G % 10000 == 0) {
    displaySuelo();
  } else if (ac_ms_G % 5000 == 0) {
    displayAmbiente();
  }
}

void prenderBomba() {
  // Leemos humedad del suelo
  int soil_humidity_val = digitalRead(SOIL_SENSOR_D);
  if (soil_humidity_val == LOW || calcularNivelAgua() <= 30) {
    // Bomba apagada
    digitalWrite(WATER_PUMP_D, LOW);
  } else {
    // Prendo la bomba solo si hay al menos si el nivel de agua es 30%
    // Bomba prendida
    digitalWrite(WATER_PUMP_D, HIGH);
  }
}

double calcularNivelAgua() {
    // Limpio el pin del trigger
  digitalWrite(WATER_SENSOR_TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Seteo el pin de trigger en HIGH por 10 us
  digitalWrite(WATER_SENSOR_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(WATER_SENSOR_TRIG_PIN, LOW);
  // Leo el pin de echo, retorna la la duracion del trayecto de la onda de sonido en ms
  long duration = pulseIn(WATER_SENSOR_ECHO_PIN, HIGH);
  
  // Calculamos la distancia
  double distance = duration*0.034/2;
  Serial.print("Distance: ");
  Serial.println(distance);
  
  return ((WATER_RECIP_H - distance) / (double) WATER_RECIP_H) * 100;
}


void displayAgua() {
  int water_level = (int) calcularNivelAgua();
  String water_level_txt;
  if (water_level < 25) {
    water_level_txt = "Bajo";
  } else if (water_level < 75) {
    water_level_txt = "Medio";
  } else {
    water_level_txt = "Alto";
  }

  // Prints the distance on the Serial Monitor
  Serial.print("Water: ");
  Serial.println(water_level);

  lcd.clear();
  lcd.home();
  lcd.print("Nivel Agua ");
  lcd.print(water_level_txt);

  lcd.setCursor(0, 1);
  lcd.print("Indice: ");
  lcd.print(water_level);
  lcd.print("%");
}

void displaySuelo() {
  // Leemos humedad del suelo
  int h_s = analogRead(SOIL_SENSOR_A);
  int soil_humidity_val = digitalRead(SOIL_SENSOR_D);

  Serial.print("Soil: ");
  Serial.println(h_s);
  
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
  Serial.println("^C");

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
