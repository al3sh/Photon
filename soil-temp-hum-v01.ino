// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_SSD1306.h>

// This #include statement was automatically added by the Particle IDE.
#include "OneWire.h"

// This #include statement was automatically added by the Particle IDE.
#include "spark-dallas-temperature.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire.h"

// This #include statement was automatically added by the Particle IDE.
#include "spark-dallas-temperature.h"

// This #include statement was automatically added by the Spark IDE.
#include "OneWire.h"

// This #include statement was automatically added by the Spark IDE.
#include "spark-dallas-temperature.h"

// -----------------
// Read temperature
// -----------------

// Data wire is plugged into port 0 on the Arduino
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(D2);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature dallas(&oneWire);

// Create a variable that will store the temperature value
double temperature = 0.0;
#include <math.h>

//
//Definicion de PINS 
#define soil  A4  //Declare a variable for the soil moisture sensor 
#define soilPower D6   //Variable for Soil moisture Power

int valSoil = 0;    //Variable de Humedad de la tierra
int soilQ = 0;      //Variable % Humedad de la tierra

//Pantalla LCD
 //Use I2C with OLED RESET pin on D4
 #define OLED_RESET D4
 Adafruit_SSD1306 oled(OLED_RESET);

 unsigned long previousMillis;
 unsigned long interval = 30000;

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

void setup()
{
// by default, we'll generate the high voltage from the 3.3v line internally! (neat!) //PAntalla LCD
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done  
  
  // Register a Particle Core variable here
  Particle.variable("temperature", &temperature, DOUBLE);

  // setup the library
  dallas.begin();

  //Serial.begin(9600);  //Eliminar!!!
  
  pinMode(soilPower, OUTPUT);       //Establece Pin D6 como OUTPUT
  digitalWrite(soilPower, LOW);      //Establece el Pin D6 como Apagado.

}//Fin Setup()

void loop()
{
//update_wifi();

  
//**********************************CONTROL DEL SENSOR DE TEMPERATURA DEL SUSTRATO***************************

  // Request temperature conversion (traditional)
  dallas.requestTemperatures();

  sin( 23423 );

  // get the temperature in Celcius
  float tempC = dallas.getTempCByIndex(0);
  // convert to double
  temperature = (double)tempC;
  
  //Lectura Correcta del Sensor de TEMP.
  // Use String to convert the float to a string - theirs probably a better way but this is easy.
  String temperatureString = String(temperature);
  // You might be able to fix this rather than allocate a new char[] every time.
  int length = temperatureString.length();
  char temperature[length];
  
  // Convert the string to char[]
  temperatureString.toCharArray(temperature, temperatureString.length()-4);
  
  

  //Particle.publish("Temp",String(tempC)); //added ssa
  //Particle.publish("Temp",String(temperature)); //added ssa

  

//**********************************CONTROL DEL SENSOR DE HUMEDAD DEL SUSTRATO*******************************
//
  
  //Particle.publish("Soil", String(valSoil));  //Publica el valor de la Humedad del Sustrato //JALANDO!!!
  
  //If your soil is too dry, turn on Red LED to notify you
  //This value will vary depending on your soil and plant
  if(readSoil() < 200)
  {
      // take control of the RGB LED
      RGB.control(true);
      RGB.color(255, 0, 0);//set RGB LED to Red
  }
  else
   {
      // resume normal operation
      RGB.control(false);
   }
   
    soilQ = map(valSoil, 0, 2550, 0, 100);
   
//**********************************CONTROL DE PANTALLA LCD*************************************************    
  
  // Do some simple loop math to draw rectangles as the bars
  // Draw one bar for each "bar" Duh...
  /*
  for (int b=0; b <= bars; b++) {
  oled.fillRect(59 + (b*5),33 - (b*5),3,b*5,WHITE);
    }
    */
    
  oled.clearDisplay();
  delay(20);
  
  //Checks WIFI strength
int wifi_rssi = WiFi.RSSI();
      int wifi_quality;

      wifi_quality = map(wifi_rssi, -1, -127, 100, 0);
      //wifi_quality = 0;   //Prueba No WIFI
      
      if (wifi_quality > 0) {
        oled.drawFastVLine( 100, 6, 2, WHITE); //Dibuja linea Vertical
        oled.drawFastVLine( 101, 6, 2, WHITE);
        oled.drawFastVLine( 102, 6, 2, WHITE);
      }
      if (wifi_quality >= 20) {
        oled.drawFastVLine( 104, 5, 3, WHITE);
        oled.drawFastVLine( 105, 5, 3, WHITE);
        oled.drawFastVLine( 106, 5, 3, WHITE);
      }
      if (wifi_quality >= 40) {
        oled.drawFastVLine( 108, 4, 4, WHITE);
        oled.drawFastVLine( 109, 4, 4, WHITE);
        oled.drawFastVLine(110, 4, 4, WHITE);
      }
      if (wifi_quality >= 60) {
        oled.drawFastVLine( 112, 3, 5, WHITE);
        oled.drawFastVLine( 113, 3, 5, WHITE);
        oled.drawFastVLine( 114, 3, 5, WHITE);
      }
      if (wifi_quality >= 80) {
        oled.drawFastVLine(115, 2, 6, WHITE);
        oled.drawFastVLine(116, 2, 6, WHITE);
        oled.drawFastVLine(117, 2, 6, WHITE);
      }
      
      oled.setCursor(0,0);
      oled.setTextSize(1);
      //oled.print(WiFi.SSID());
      oled.print(" Soil Info:");
    
    if (wifi_quality == 0) {
        oled.setTextSize(1);
        oled.setCursor(0,0);
        oled.print("no WiFi");
    }
  
  /*
  //Desplieda Informacion del Sustrato
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,12);
  oled.setTextSize(2);
  //oled.setTextColor(YELLOW);
  if(String(temperature) == "-127.0"){
      //DEspliega Error de Lectura de Sensor
      oled.print("Temp: Err.");
  } else { 
      //Normal
      oled.print("Temp: ");  oled.print(String(temperature)); //oled.print(" C");
      }
  oled.setCursor(0,30);
  oled.print("Hum: "); oled.print(String(soilQ));  oled.print("%");  oled.print(" =");
  oled.setCursor(60,48);
  oled.print(String(valSoil)); 
  */
  //Despliega inforamcion de la Senal del Photon
  oled.setTextColor(WHITE);
  oled.setCursor(0,12);
  oled.setTextSize(2);
  oled.print("SIGNAL: "); 
  oled.setCursor(44,32);
  oled.print(String(wifi_rssi));
  
  //oled.print(Time.hourFormat12()); oled.print(":"); oled.print(Time.minute()); oled.print(":"); oled.print(Time.second());
  oled.setTextColor(BLACK, WHITE); // 'inverted' text
  
  oled.display();
//****

delay(800);

}//Fin Loop()


//This is a function used to get the soil moisture content
//FUNCION PARA OBTENER LOS DATOS DEL SENSOR DE HUMENDAD DEL SUSTRATO
int readSoil()
{
    digitalWrite(soilPower, HIGH);//turn D6 "On"
    delay(10);//wait 10 milliseconds 
    valSoil = analogRead(soil);
    
    digitalWrite(soilPower, LOW);//turn D6 "Off"
    return valSoil;
}

