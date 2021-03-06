#include <DHT.h>

const String WIFI_SSID = "HITRON-01F0";
const String WIFI_PWD = "NAQ2EACQBURD";
const bool DEBUG = true;   //show more logs
const int TIMEOUT = 3000; //communication timeout
const int WIFI_ERRORS_THRESHOLD = 10; //Wifi errors max before reboot
const int DATA_SENDING_INTERVAL = 2000; // Intervalo para enviar datos al servidor
const int STEP_TIME = 5000; //Loop delay
const int DEVICE_ID = 69; // Identificador de dispositivo para reporte de datos

const int SOIL_SENSOR_A = A0; // Sensor de suelo
const int TEMP_SENSOR_D = 7; // Sensor de temperatura
const int LIGHT_SENSOR_A = A1; // Sensor de luz
const int RX_PIN = 2; // Rx de modulo WiFi
const int TX_PIN = 3; // Tx de modulo WiFi

// Definimos el pin digital donde se conecta el sensor DHT11
#define DHTPIN TEMP_SENSOR_D
// Definimos el tipo de sensor
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(SOIL_SENSOR_A,INPUT);
  pinMode(LIGHT_SENSOR_A, INPUT);
  
  // Open serial communications and wait for port to open esp8266:
  Serial.begin(9600);
  Serial3.begin(115200);
  initWifi();
  dht.begin();
}


void initWifi() {
  sendToWifi("AT",TIMEOUT,DEBUG);
  delay(100);
  sendToWifi("AT+RST",TIMEOUT,DEBUG);
  delay(5000);
  sendToWifi("AT+CIPMUX=1",TIMEOUT,DEBUG);
  delay(100);
  sendToWifi("AT+CWMODE=1",TIMEOUT,DEBUG);
  delay(100);
  sendToWifi("AT+CIPMODE=0",TIMEOUT,DEBUG);
  delay(100);
  sendToWifi("AT+CWJAP=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",5000,DEBUG);
  delay(100);
  Serial.println("Wifi connection is running!");
}


float h_amb = 0;
float t_amb = 0;
int ac_time = 0;
int connection = 0;
int wifi_errors = 0;

void loop() {
  // Leemos la humedad relativa
  float temp_h_amb = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float temp_t_amb = dht.readTemperature();

  // Comprobamos si ha habido algún error en la lectura
  if (!isnan(temp_h_amb) && !isnan(temp_t_amb)) {
    h_amb = temp_h_amb;
    t_amb = temp_t_amb;
  }
  
  // Leemos humedad del suelo
  int h_soil = analogRead(SOIL_SENSOR_A);
  // Leemos la itensidad de luz
  int l_amb = analogRead(LIGHT_SENSOR_A);

  if (ac_time % DATA_SENDING_INTERVAL == 0) { //Envio datos cada 
    // Si se supera la cantidad de errores en conexion Wifi ejecuto rutina de re-boot
    if (wifi_errors > WIFI_ERRORS_THRESHOLD) {
      initWifi();
      wifi_errors = 0;
    }
    
    ac_time  = 0;
    Serial.print("HA:");
    Serial.println(h_amb);
    Serial.print("TA:");
    Serial.println(t_amb);
    Serial.print("HS:");
    Serial.println(h_soil);
    Serial.print("LA:");
    Serial.println(l_amb);

    String data = "?device_id="+String(DEVICE_ID);
    data += "&humidity="+String(h_amb);
    data += "&temperature="+String(t_amb);
    data += "&soil_humidity="+String(h_soil);
    data += "&light_intensity="+String(l_amb);
    
    sendToWifi("AT+CIPSTART="+String(connection)+",\"TCP\",\"www.jcdavidconde.com\",80",TIMEOUT,DEBUG);
    delay(100);
    String request = "GET /jcdavidconde.com/sensor.php";
    request += data;
    request += " HTTP/1.1\r\n";
    request += "Accept: */*\r\n";
    request += "User-Agent: ArduinoUno/ESP8266\r\n";
    request += "Host: www.jcdavidconde.com\r\n";
    request += "Connection: close\r\n\r\n";

    sendData(request);
    connection = (connection + 1) % 4;
  }
   
  delay(STEP_TIME);
  ac_time += STEP_TIME;
}


/*
* Name: sendData
* Description: Function used to send string to tcp client using cipsend
* Params: 
* Returns: void
*/
void sendData(String str) {
  String len="";
  len+=str.length()+2;
  sendToWifi("AT+CIPSEND="+String(connection)+","+len,TIMEOUT,DEBUG);
  delay(100);
  Serial.println(str);
  sendToWifi(str,TIMEOUT,DEBUG);
  sendToWifi("AT+CIPCLOSE="+String(connection),TIMEOUT,DEBUG);
}


/*
* Name: readSerial3Message
* Description: Function used to read data from ESP8266 Serial.
* Params: 
* Returns: The response from the esp8266 (if there is a reponse)
*/
String  readSerial3Message() {
  char value[100]; 
  int index_count =0;
  while(Serial3.available()>0) {
    value[index_count]=Serial3.read();
    index_count++;
    value[index_count] = '\0'; // Null terminate the string
  }
  String str(value);
  str.trim();
  return str;
}


/*
* Name: sendToWifi
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendToWifi(String command, const int timeout, const bool debug) {
  String response = "";
  Serial3.println(command); // send the read character to the esp8266
  long int time = millis();
  while( (time+timeout) > millis()) {
    while(Serial3.available()) {
    // The esp has data so display its output to the serial window 
    char c = Serial3.read(); // read the next character.
    response+=c;
    }  
  }
  if(debug) {
    if (response == "") {
      response = "ERROR: WiFi timeout exceded";
    }
    if (response.indexOf("ERR") != -1) {
      wifi_errors++;
    }
    Serial.println(response);
  }
  return response;
}
