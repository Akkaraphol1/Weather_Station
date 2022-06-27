
#include <SFE_BMP180.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
SFE_BMP180 pressure;

#include <math.h>
#define WindSensorPin (2) 
volatile unsigned long Rotations; 
volatile unsigned long ContactBounceTime; 
float WindSpeed;

int VaneValue; 
int Direction;
int CalDirection;
int LastValue;
#define Offset 0;

#define WIFI_STA_NAME "GTT"
#define WIFI_STA_PASS "56/17Thai.EE"

String Data;
String GOOGLE_SCRIPT_ID = "AKfycbzbzm4DpGj0nSe16Y38s5Cc97hs5S-53L2L_P9KwtovSsh3CNI";

#define ALTITUDE 1655.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters

void setup()
{
  Serial.println("REBOOT");


  LastValue = 1;
  Serial.begin(9600);
Serial.println("Vane Value\tDirection\tHeading");

  while (!Serial) ;
 pressure.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
   Serial.println("BMP180 init success");

   pinMode(WindSensorPin, INPUT);
attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);

Serial.println("Wind Speed Test");
Serial.println("Rotations\tMPH");



  // Initialize the sensor (it is important to get calibration values stored on the device).

}

void loop()
{
  char status;
  double T,P,p0,a,H;
String strwd = "N";
float keepWindspeed; 
///////


Rotations = 0; // Set Rotations count to 0 ready for calculations

sei(); // Enables interrupts
delay (100); // Wait 3 seconds to average
cli(); // Disable interrupts

// convert to mp/h using the formula V=P(2.25/T)
// V = P(2.25/3) = P * 0.75

WindSpeed = Rotations * 0.75;


Serial.print(Rotations); Serial.print("\t\t");
Serial.println(WindSpeed);
keepWindspeed = WindSpeed;

  
VaneValue = analogRead(A0);
Direction = map(VaneValue, 0, 4095, 0, 360);


CalDirection = Direction + Offset;

if(CalDirection > 360)
CalDirection = CalDirection - 360;

if(CalDirection < 0)
CalDirection = CalDirection + 360;

// Only update the display if change greater than 2 degrees.
if(abs(CalDirection - LastValue) > 5)
{
Serial.print(VaneValue); Serial.print("\t\t");
Serial.print(CalDirection); Serial.print("\t\t");

if(CalDirection < 22){
strwd = "N";
Serial.println(strwd);}
else if (CalDirection < 67){
  strwd = "NE";
Serial.println(strwd);}
else if (CalDirection < 112){
   strwd = "E";
Serial.println(strwd);}
else if (CalDirection < 157){
  strwd = "SE";
Serial.println(strwd);}

else if (CalDirection < 212){
  strwd = "S";
Serial.println(strwd);}
else if (CalDirection < 247){
  strwd = "SW";
Serial.println(strwd);}
else if (CalDirection < 292){
  strwd = "W";
Serial.println(strwd);}
else if (CalDirection < 337){
  strwd = "NW";
Serial.println(strwd);}
else
  strwd = "E";
Serial.println(strwd);

LastValue = CalDirection;

}
 
  Serial.println("Message Received: ");
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
  


  status = pressure.startTemperature();
  if (status != 0)
  {
   
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      H = (T + (T/2));
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
   
      Serial.print("Humidity: ");
      Serial.print(H, 2);
      Serial.println("%");
     
      status = pressure.startPressure(3);
      if (status != 0)
      {
        
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          

          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
//Serial.println(Serial2.readString());

           send_all_value(T,H,P,keepWindspeed,strwd);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

  delay(3000);  // Pause for 30 seconds.
}


void send_all_value(float tem, float hum , float pres ,float keepWindspeed, String strwd)
{

  String param;

     param  = "Temperature="+String(tem);
     param  += "&Humidity="+String(hum);
     param  += "&Pressure="+String(pres);
     param  += "&WindSpeed="+String(keepWindspeed);
     param  += "&WindDirect="+String(strwd);

    Serial.println(param);
    write_to_google_sheet(param);
}

/************************************************************************************
 *  loop function starts
 **********************************************************************************/
void write_to_google_sheet(String params) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
   //Serial.print(url);
    Serial.println("Send data Scanner to Google Sheet");
    //---------------------------------------------------------------------
    //starts posting data to google sheet
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();  
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);     
    }
    //---------------------------------------------------------------------
    http.end();
}



// Converts compass direction to heading
/*void getHeading(int direction) {
if(direction < 22){
strwd = "N";
Serial.println("N");}
else if (direction < 67){
strwd = "NE";
Serial.println("NE");}
else if (direction < 112){
strwd = "E";
Serial.println("E");}
else if (direction < 157){
strwd = "SE";
Serial.println("SE");}
else if (direction < 212){
strwd = "S";
Serial.println("S");}
else if (direction < 247){
strwd = "SW";
Serial.println("SW");}
else if (direction < 292){
strwd = "W";
Serial.println("W");}
else if (direction < 337){
strwd = "NW";
Serial.println("NW");}
else
strwd = "N";
Serial.println("N");
delay(1000);
}
*/
void isr_rotation () {

if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
Rotations++;
ContactBounceTime = millis();
}}


//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
