#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 5
#include<LiquidCrystal.h>
LiquidCrystal lcd(13,12,11,10,8,7);

#include <SoftwareSerial.h>
SoftwareSerial espSerial (2,3);
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
float temp= 0.00;
float turb= 0.00;
float pH= 0.00;
float temperature = 0.00;
int buf[10];
int avgvalue=0;
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
int turbidityPin = A0,phpin=A1;
float volt;
float ntu;
long writingTimer = 15; 
long startTime = 0;
long waitTime = 0;
String apiKey = "";     // replace with your channel's thingspeak WRITE API key

String ssid="";    // Wifi network SSID
String password ="";  // Wifi network password

boolean DEBUG=true;

void showResponse(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (espSerial.available()){
        c=espSerial.read();
        if (DEBUG) Serial.print(c);
      }
    }
                   
}
boolean thingSpeakWrite(float value1, float value2,float value3){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";
  espSerial.println(cmd);
  if (DEBUG) Serial.println(cmd);
  if(espSerial.find("Error")){
    if (DEBUG) Serial.println("AT+CIPSTART error");
    return false;
  }
  String getStr = "GET /update?api_key=";   // prepare GET string
  getStr += apiKey;
  
  getStr +="&field1=";
  getStr += String(value1);
  getStr +="&field2=";
  getStr += String(value2);
  getStr +="&field3=";
  getStr += String(value3);
  // ...
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);
  if (DEBUG)  Serial.println(cmd);
  
  delay(100);
  if(espSerial.find(">")){
    espSerial.print(getStr);
    if (DEBUG)  Serial.print(getStr);
  }
  else{
    espSerial.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}
void setup() {
  // put your setup code here, to run once:
DEBUG=true;           // enable debug serial
  Serial.begin(9600); 
           // Start DHT sensor
  
  espSerial.begin(9600);  // enable software serial
                          // Your esp8266 module's speed is probably at 115200. 
                          // For this reason the first time set the speed to 115200 or to your esp8266 configured speed 
                          // and upload. Then change to 9600 and upload again
  
  //espSerial.println("AT+RST");         // Enable this line to reset the module;
  //showResponse(1000);

  //espSerial.println("AT+UART_CUR=9600,8,1,0,0");    // Enable this line to set esp8266 serial speed to 9600 bps
  //showResponse(1000);
  
  // Start up the library
  sensors.begin();
  lcd.begin(20,4);
  lcd.setCursor(4,0);
lcd.print("Water Quality");
lcd.setCursor(2,1);
lcd.print("Monitoring System");
delay(3500);
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Initializing...");
espSerial.println("AT+CWMODE=1");   // set esp8266 as client
  showResponse(1000);

  espSerial.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  // set your home router SSID and password
  showResponse(5000);

   if (DEBUG)  Serial.println("Setup completed");
   lcd.clear();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  temp = check_temperature();
  turb = check_turbidity();
  pH = check_pH();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.setCursor(6,0);
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Turb: ");
  lcd.setCursor(6,1);
  lcd.print(turb);
  lcd.setCursor(0,2);
  lcd.print("pH:");
  lcd.setCursor(6,2);
  lcd.print(pH);
  if (turb<500 && pH < 7.1)
  {
   lcd.setCursor(0,3);
   lcd.print("Water Is Safe");
   delay(2000); 
  }
  else if (turb>500&& pH < 7.1)
  {
   lcd.setCursor(0,3);
   lcd.print("Water Is Unsafe");
   delay(2000);  
  }
  waitTime = millis()-startTime;   
  if (waitTime > (writingTimer*1000)) 
  {
    thingSpeakWrite(temp,turb,pH);
    startTime = millis();   
  }
  thingSpeakWrite(temp,turb,pH);  
  //delay(10000);
}
float check_temperature()
{
  sensors.requestTemperatures(); 
  temperature = (sensors.getTempCByIndex(0));
  temperature = round (temperature);
  return temperature; 
}
float check_turbidity()
{
   volt = 0;
    for(int i=0; i<800; i++)
    {
        volt += ((float)analogRead(turbidityPin)/1023)*5;
    }
    volt = volt/800;
    volt = round_to_dp(volt,1);
    volt = volt + 1.4;
    if(volt < 2.5){
      ntu = 3000;
    }else{
   ntu = -1120.4*square(volt)+5742.3*volt-4353.8; 
   ntu= ntu+0.1;
    }
 ntu = round (ntu);
 return ntu;
}
float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

float check_pH()
{
for (int i=0;i<10;i++)
{
  buf[i]= analogRead(phpin);
  delay(10);
}
for(int i=0;i<9;i++)
{
for(int j=i+1;j<10;j++)
{
if (buf[i]>buf[j])
{
temp=buf[i];
buf[i]=buf[j];
buf[j]=temp;  
}  
}  
}
avgvalue =0;
for(int i=2;i<8;i++)
{
 avgvalue +=buf[i];
 float pHVol=(float)avgvalue*3.72/1023/6;
 Serial.println(pHVol);
 float phValue = -6.10 * pHVol + 23.592;// this involves a little bit of calculus
 Serial.print("Sensor = ");
 Serial.println(phValue);
 delay(1000); 
 return phValue;
}  
}
