#include <SoftwareSerial.h>
#include <DHT.h>

const String _SSID = "la_cueva_de_ugarte";
const String _PWD = "fr4nqu1t0";

// Sensor de suelo
const int SOIL_SENSOR_A = A0;
// Sensor de temperatura
const int TEMP_SENSOR_D = 7;
// Sensor de luz
const int LIGHT_SENSOR_A = A1;

const int RX_PIN = 2;
const int TX_PIN = 3;

// DEVICE_ID
const int DEVICE_ID = 12;

// Definimos el pin digital donde se conecta el sensor DHT11
#define DHTPIN TEMP_SENSOR_D
// Definimos el tipo de sensor
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial wifiSerial(RX_PIN, TX_PIN);      // RX, TX for ESP8266

bool DEBUG = true;   //show more logs
const int TIMEOUT = 3000; //communication timeout

void setup()
{
  pinMode(SOIL_SENSOR_A,INPUT);
  pinMode(LIGHT_SENSOR_A, INPUT);
  
  // Open serial communications and wait for port to open esp8266:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  wifiSerial.begin(115200);
  while (!wifiSerial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
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
  sendToWifi("AT+CWJAP=\""+_SSID+"\",\""+_PWD+"\"",5000,DEBUG);
  delay(100);
  Serial.println("Wifi connection is running!");  
}

float h_amb = 0;
float t_amb = 0;
int ac_time = 0;
const int step_time = 1000;
int connection = 0;

void loop()
{
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

  if (ac_time % 30000 == 0) {
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
   
  delay(step_time);
  ac_time += step_time;
}


/*
* Name: sendData
* Description: Function used to send string to tcp client using cipsend
* Params: 
* Returns: void
*/
void sendData(String str){
  String len="";
  len+=str.length()+2;
  sendToWifi("AT+CIPSEND="+String(connection)+","+len,TIMEOUT,DEBUG);
  delay(100);
  Serial.println(str);
  sendToWifi(str,TIMEOUT,DEBUG);
  sendToWifi("AT+CIPCLOSE="+String(connection),TIMEOUT,DEBUG);
}


/*
* Name: find
* Description: Function used to match two string
* Params: 
* Returns: true if match else false
*/
boolean find(String string, String value){
  return string.indexOf(value)>=0;
}


/*
* Name: readWifiSerialMessage
* Description: Function used to read data from ESP8266 Serial.
* Params: 
* Returns: The response from the esp8266 (if there is a reponse)
*/
String  readWifiSerialMessage(){
  char value[100]; 
  int index_count =0;
  while(wifiSerial.available()>0){
    value[index_count]=wifiSerial.read();
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
String sendToWifi(String command, const int timeout, boolean debug){
  String response = "";
  wifiSerial.println(command); // send the read character to the esp8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(wifiSerial.available())
    {
    // The esp has data so display its output to the serial window 
    char c = wifiSerial.read(); // read the next character.
    response+=c;
    }  
  }
  if(debug)
  {
    if (response == "") {
      response = "WiFi timeout exceded";
    }
    Serial.println(response);
  }
  return response;
}
