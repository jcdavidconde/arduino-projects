#include  <Wire.h>
#include <DS1302.h>
#include <EEPROM.h>
#include <time.h> 

// Bomba de agua
const int WATER_PUMP_D = 8;

//Sensor de suelo
const int SOIL_SENSOR_D = 7;
const int SOIL_SENSOR_A = A2;

// Constante de delay de loop
const int STEP_MS = 1000;

// Constantes de Reloj
const int RTC_RST_PIN = 4;
const int RTC_DAT_PIN = 3;
const int RTC_CLK_PIN = 2;

/* Create buffers */
char buf[50];
int eeAddress = 0;   //Location we want the data to be put.

// Maximo intervalo entre riegos (segundos)
const double LAST_IRRIGATION_MAX_INTERVAL = 120; //20*3600;

// Tiempo de riego (milisegundos)
const int PUMP_ON_TIME = 30*1000;

DS1302 rtc(RTC_RST_PIN, RTC_DAT_PIN, RTC_CLK_PIN);

void setup() {
  // Pin de entrada de datos digital del sensor del suelo
  pinMode(SOIL_SENSOR_D, INPUT);
  
  // Pin de activación de bomba de agua
  pinMode(WATER_PUMP_D,OUTPUT);

  // Inicializamos comunicación serie
  Serial.begin(9600);

  rtc.halt(false);
  rtc.writeProtect(false);

  //rtc.setDOW(SUNDAY);        // Imposta il giorno della settimana a SUNDAY
  //rtc.setTime(17, 33, 0);     // Imposta l'ora come 11:32:00 (Formato 24hr)
  //rtc.setDate(14, 4, 2019);   // Imposta la data cone 12 febbraio 2017
}

void loop() {
  verificarRiego();
  displaySuelo();
  delay(STEP_MS);
}

void verificarRiego() {
  // Leemos humedad del suelo
  int soil_humidity_val = digitalRead(SOIL_SENSOR_D);
  
  struct tm last_irrigation_t; //Variable to store custom object read from EEPROM.
  EEPROM.get(eeAddress, last_irrigation_t);
  printTime(last_irrigation_t, "Last irrigation");
  
  struct tm current_t = convertTime(rtc.time());
  printTime(current_t, "Current time");
  
  double t_since_last_irrigation = difftime(mktime(&current_t),mktime(&last_irrigation_t));
  EEPROM.get(eeAddress, last_irrigation_t);

  if (soil_humidity_val == HIGH &&
     (last_irrigation_t.tm_year < 2011 || LAST_IRRIGATION_MAX_INTERVAL < t_since_last_irrigation)) {
    // Prendo si paso un tiempo antes del ultimo riego
    regar();
  }
}

void displaySuelo() {
  // Leemos humedad del suelo
  int h_s = analogRead(SOIL_SENSOR_A);
  int soil_humidity_val = digitalRead(SOIL_SENSOR_D);

  Serial.print("Soil: ");
  Serial.println(h_s);
}

void regar() {
  struct tm irrigation_t = convertTime(rtc.time());
  EEPROM.put(eeAddress, irrigation_t);
  
  // Bomba prendida
  digitalWrite(WATER_PUMP_D, HIGH);

  delay(PUMP_ON_TIME);

  // Bomba apagada
  digitalWrite(WATER_PUMP_D, LOW);
}

struct tm convertTime(Time t) {
  struct tm converted_time = {0};
  
  converted_time.tm_hour = (int) t.hr;
  converted_time.tm_min = (int) t.min;
  converted_time.tm_sec = (int) t.sec;
  converted_time.tm_year = (int) t.yr;
  converted_time.tm_mon = (int) t.mon;
  converted_time.tm_mday = (int) t.date;

  return converted_time;
}

void printTime(struct tm t, String label) {
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
    t.tm_year, t.tm_mon, t.tm_mday,
    t.tm_hour, t.tm_min, t.tm_sec);

  Serial.print(label);
  Serial.print(": ");
  Serial.println(buf);
}
