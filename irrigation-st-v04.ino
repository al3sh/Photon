/*PROGRAMA QUE CONTROLA SISTEMA DE RIEGO DE TRES VALCULAS.
LAS VALVULAS ESTAN CONECTADAS A RELEVADORES NORMALMENTE CERRADOS.

Control de Versiones:

-irrigation-st-01       Version Inicial. Control de valvulas via SmartThings (encender/apagar). No hay checkStatus en Smarthhings  
-irrigation-st-02       Se agrega la funcion en deviceType (Smartthings) checkStatus que verifica cual es el estado actual de cada valvula.
                        Particle comparte este Informacion atravez de "Particle.variable("checkStatus1", "off")".
-irrigation-st-03       Se agrega Sensor de Temperatura Digital DS18b20 para monitorear la temp. del sustrato.        
-irrigation-st-04       Se agregan...


*/

// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_SSD1306.h>

// This #include statement was automatically added by the Particle IDE.
#include "spark-dallas-temperature.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire.h"


STARTUP(WiFi.selectAntenna(ANT_AUTO));
//STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

//******************************************INICIALIZACION DE SENSORES [TEMP Y HUM]****************************************

// Data wire is plugged into port 0 on the Arduino
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(D5);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature dallas(&oneWire);

// Create a variable that will store the temperature value
double temperature = 0.0;
#include <math.h>

//
//Definicion de PINS 
#define soil  A4  //Declare a variable for the soil moisture sensor 
#define soilPower D7   //Variable for Soil moisture Power

int valSoil = 0;    //Variable de Humedad de la tierra
int soilQ = 0;      //Variable % Humedad de la tierra

//***********************************************INICIALIZACION DE PANTALLA LCD*********************************************
 //Use I2C with OLED RESET pin on D4
 #define OLED_RESET D6
 Adafruit_SSD1306 oled(OLED_RESET);

 unsigned long previousMillis;
 unsigned long interval = 30000;



//***********************************************INICIALIZACION DE CONTROL DE REIGO******************************************

int ledControl(String command);

// Asignacion de Pines
int aspersor1 = D2; 
int aspersor2 = D3;
int aspersor3 = D4;
//int aspersor4 = D4;       //RElay 4 no sera usado por el mometo

//Variables para monitorear la ultia vez que riego cada valvula.
//Fecha en Formato UNIX
int riego_ant_aspersor1;
int riego_ant_aspersor2;
int riego_ant_aspersor3;
//int riego_ant_aspersor4;

//Variables para controlar la ultima vez que se detubo el Riego [CANCELACION O CICLO COMPLETO]
int stop_aspersor1;
int stop_aspersor2;
int stop_aspersor3;
//int stop_aspersor4;

//Los SEGUNDOS necesarios para que un nuevo evento de Riego cuente como NUEVO RIEGO y se active el mismo..
int delay_riego = 1; //1 = Modo Test; Modo Operacion = 61.


//SETUP()
void setup()
{
// by default, we'll generate the high voltage from the 3.3v line internally! (neat!) //PAntalla LCD
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done  
  
// setup the library
  dallas.begin();

//  
//Declaracion de Funciones que recibiran instucciones de a la NUBE <---- SMARTTHINGS
  Particle.function("sprinkler1", valvula1);    //Var Ej ="ledstate"
  Particle.function("sprinkler2", valvula2);
  Particle.function("sprinkler3", valvula3);
// Particle.function("sprinkler4", valvula4);

//Fecha en Formato Humano que sera compartida a SmartThings
    Particle.variable("antRiegoV1", Time.timeStr(riego_ant_aspersor1));
    Particle.variable("antRiegoV2", Time.timeStr(riego_ant_aspersor2));
    Particle.variable("antRiegoV3", Time.timeStr(riego_ant_aspersor3)); 

//Variable que sera leida por SMARTTHINGS para identificar en que estado estan las Vavulas. //Valor inicial OFF
  Particle.variable("checkStatus1", "off");
  Particle.variable("checkStatus2", "off");
  Particle.variable("checkStatus3", "off");
  //Particle.variable("checkStatus4", "off");    

//Valores actuales de los SENSORES que se enviaran a SmartThings
  Particle.variable("temp", &temperature, DOUBLE);
  //Particle.variable("hum1", &soilQ, DOUBLE);

  
//Declaracion de los PINES de Salida 
//(Control Valvulas) y el estado Inicial siempre sera apagadas.
  pinMode(aspersor1, OUTPUT);
  digitalWrite(aspersor1, LOW);
  pinMode(aspersor2, OUTPUT);
  digitalWrite(aspersor2,  LOW);
  pinMode(aspersor3, OUTPUT);
  digitalWrite(aspersor3, LOW);
  //pinMode(aspersor4, OUTPUT);
  //digitalWrite(aspersor4, LOW);  

//(PWR Senosr Humedad) y el estado Inicial siempre sera apagadas.
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
      //RGB.control(true);
      //RGB.color(255, 0, 0);//set RGB LED to Red
  }
  else
   {
      // resume normal operation
      RGB.control(false);
   }
   
    //Particle.publish("Soil",String(valSoil)); //added ssa
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
      oled.setTextColor(WHITE);
      oled.setCursor(80,0);
      oled.print(String(wifi_rssi));
    
    if (wifi_quality == 0) {
        oled.setTextSize(1);
        oled.setCursor(0,0);
        oled.print("no WiFi");
    }
  
  
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
  oled.print("Hum: "); oled.print(String(soilQ));  oled.print("%");  
  oled.setTextSize(1);
  oled.setCursor(60,48);
  oled.print(" = ");
  oled.print(String(valSoil)); 
  
  /*
  //Despliega inforamcion de la Senal del Photon
  oled.setTextColor(WHITE);
  oled.setCursor(0,12);
  oled.setTextSize(2);
  oled.print("SIGNAL: "); 
  oled.setCursor(44,32);
  oled.print(String(wifi_rssi));
  */
  
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

//FUNCION DE CONTROL DE LAS VALVULAS DE RIEGO. [VIA RELAYS SWITCH]
int valvula1(String command)
{
  if (command == "1") {
      int now = Time.now();
            if (now - riego_ant_aspersor1 > delay_riego) {
                riego_ant_aspersor1 = now;          //Actulaiza el Inicializacion del ULTIMO riego
                 digitalWrite(aspersor1, HIGH);   // Enciende aspersor1
                 Particle.variable("checkStatus1", "on");   //Variable que Publica en la nube el Estado de la Valvula [ON/OFF] -- Sera leida por SmartThings
                 Particle.variable("ult-valv1", riego_ant_aspersor1);
            }
    return 1;
  } else {               
    digitalWrite(aspersor1, LOW);    // Apaga aspersor1
    stop_aspersor1 = Time.now();
    Particle.variable("checkStatus1", "off");
    return 0;
  }
}

int valvula2(String command)
{
  if (command == "1") {
      int now = Time.now();
            if (now - riego_ant_aspersor2 > delay_riego) {
                riego_ant_aspersor2 = now;          //Actulaiza el Inicializacion del ULTIMO riego
                 digitalWrite(aspersor2, HIGH);   // Enciende aspersor2
                 Particle.variable("checkStatus2", "on");
            }
    return 1;
  } else {               
    digitalWrite(aspersor2, LOW);    // Apaga aspersor2
    stop_aspersor2 = Time.now();
    Particle.variable("checkStatus2", "off");
    return 0;
  }
}

int valvula3(String command)
{
  if (command == "1") {
      int now = Time.now();
            if (now - riego_ant_aspersor3 > delay_riego) {
                riego_ant_aspersor3 = now;          //Actulaiza el Inicializacion del ULTIMO riego
                 digitalWrite(aspersor3, HIGH);   // Enciende aspersor3
                 Particle.variable("checkStatus3", "on");
                 //Particle.publish("variableR",command);
                 //delay(command2*60000);
            }
    return 1;
  } else {               
    digitalWrite(aspersor3, LOW);    // Apaga aspersor3
    stop_aspersor3 = Time.now();
    Particle.variable("checkStatus3", "off");
   // Particle.publish("variableR",command);
    return 0;
  }
}
