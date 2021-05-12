// include libraries
#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3


// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS; 
#include <SD.h>                 // for SD card
#define SD_ChipSelectPin 10     // for SD card
#include <TMRpcm.h>             // Lib to play wav file
#include <Adafruit_MLX90614.h>  // for infrared thermometer
//-------------------------------------- oled
#include <LiquidCrystal.h>
LiquidCrystal lcd(5,4,17,16,15,14);
//--------------------------------------- oled
Adafruit_MLX90614 mlx = Adafruit_MLX90614();  //for infrared thermometer

TMRpcm tmrpcm;            // create an object for music player

double temp;  // to save temperature value
const int mute = 6;
const int trigPin = 7;  //ultrasonic
const int echoPin = 8;  //ultrasonic
const int pass = 2; 
const int alert = 0;
int maximumRange = 25; // Maximum range needed
int minimumRange = 15; // Minimum range needed
long duration;
int distance;
int step1_judge = 0;
const int chipSelect = 10;


void setup()
{
  pinMode(mute,OUTPUT);
  Serial.begin(9600);
  //lcd.init(); 
  //lcd.backlight();
  lcd.begin(16,2);
  mlx.begin(); 
  lcd.setCursor(2,0);
  lcd.print("DISINFECTION");
  lcd.setCursor(5,1); 
  lcd.print("BOOTH");
  delay (3000);
  lcd.clear();
  tmrpcm.speakerPin = 9; //pin 9 for output audio
  
  if(!SD.begin(chipSelect)) {
    
    //for Atmega 328 miso=12,mosi=11,sck=13
    // for Atmega 2560 miso=50,mosi=51,sck=52
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Initialization");
    lcd.setCursor(5,1);
    lcd.print("Fail");
    return;
  }
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Initialization");
  lcd.setCursor(5,1);
  lcd.print("Done");
  Serial.println("initialization done....");
    
   delay(3000);
   lcd.clear();
   digitalWrite(mute,HIGH);
  tmrpcm.play("m_weladd.wav"); //the sound file welcome will play each time the arduino powers up, or is reset
  //tmrpcm.play("m_wel.wav");
  tmrpcm.volume(2);
  lcd.setCursor(0,0);
  lcd.print(" Welcome!!!!!"); 
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input 
  pinMode(pass, OUTPUT); 
  pinMode(alert, OUTPUT);           
  mlx.begin();  //start infrared thermometer
  delay(52000);  //wait for welcome audio
  digitalWrite(mute,LOW);
  lcd.clear();
}

void loop(){
  //------------reading distance
  // Sets the trigPin on HIGH state for 10 micro seconds
 message_settings();
 lcd.clear();
 lcd.setCursor(1,0); 
 lcd.print("MOVE CLOSER");
 lcd.setCursor(0,1); 
 lcd.print("MOVE CLOSER");
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH, 23529); //23529us for timeout 4.0m

  // Calculating the distance
  distance= duration*0.034/2;

  //speak_out(temp);
  Serial.print("distance is ");
  Serial.println(distance);
  //if ((distance<40)&&(distance>0)) step1_judge++;
  //else step1_judge=0;
  
  //if (step1_judge>2){
    //step1_judge=0;
    if ((distance<55)&&(distance>30)){
      digitalWrite(mute,HIGH);
      lcd.clear();
     lcd.setCursor(1,0); 
     lcd.print("PUT YOUR HEAD");
     lcd.setCursor(0,1); 
     lcd.print("CLOSE TO SENSOR");
    tmrpcm.play("m_wel.wav");
    delay(10000); //wait for welcome voice complete
     lcd.clear();
    temp = mlx.readObjectTempC()+2.2;//---------------------reading temperature & show on LCD
    //temp = 37.4;  //for testing, comment this line for real reading
    lcd.clear();
    lcd.setCursor(0,0); 
  lcd.print("YOUR TEMPERATURE");
   lcd.setCursor(0,1); 
  lcd.print("N0W IS :");
  lcd.print(temp,1);
  lcd.print((char)223);
  lcd.print("C");
    tmrpcm.play("m_now.wav");
    delay(1380);
    
    if (temp<20){
      tmrpcm.play("m_b20.wav");  //speak out below 20 dgC
      delay(1700);               //wait for audio finish
      tmrpcm.play("m_nman.wav"); //speak out "you're not human"
      delay(2270);               //wait for audio finish
    }
    else{
      if (temp>50){
        tmrpcm.play("m_over50.wav"); //speak out over 50 dgC
        delay(1740);
        tmrpcm.play("m_nman.wav");   //speak out "you're not human"
        delay(2270);
      }
      else{
        speak_out(temp);  //speak out temperature (if it is from 20 to 50dgC)
        delay(1500);
        if((temp>36)&&(temp<37)){
          tmrpcm.play("m_normal.wav");  //speak out "normal temperature, keep healthy" if it is 36~37dgC
          delay(3268);
        }
        if(temp>37){
          tmrpcm.play("m_fever.wav"); //speak out "you got fever"
          delay(2728);
        }
        if((temp>34)&&(temp<37)){
          digitalWrite(pass,HIGH);
          delay(3000);
          digitalWrite(pass,LOW);
        }
         if(temp>37){
          digitalWrite(alert,HIGH);
          delay(10000);
          digitalWrite(alert,LOW);
          send_sms();
        }
      }
    }
  }
  digitalWrite(mute,LOW);
  delay(1000);
}


void speak_out(double temperature_result){
  //this sub-program will speak out temperature
  temperature_result = temperature_result*10;
  temperature_result = round(temperature_result);
  int temp0 = temperature_result;
  int temp1 = temp0/10; 
  int temp2 = temp1%10; 
  int temp3 = temp0%10;

  if(temp1<20){
    tmrpcm.play("m_below20.wav"); //below 20dgC
    delay(1631);
  }
  if(temp1>50){
    tmrpcm.play("m_over50.wav"); //greater 50dgC
    delay(1747);
  }
  if((temp1>=20)&&(temp1<=29)){
    tmrpcm.play("m_twenty.wav"); //twenty
    delay(600);
  }
  if((temp1>=30)&&(temp1<=39)){
    tmrpcm.play("m_thirty.wav"); //thirty
    delay(500);
  }
  if((temp1>=40)&&(temp1<=49)){
    tmrpcm.play("m_fourty.wav"); //forty
    delay(691);
  }
  if (temp2!=0) speak_num(temp2); //temperature value, y digit (in xy.z dgC)
  if((temp1>=20)&&(temp1<=50)){
    tmrpcm.play("m_point.wav"); //point
    delay(319);
    speak_num(temp3); //temperature value, z digit (in xy.z dgC)
  }
  tmrpcm.play("m_dgc.wav"); //degree C
  delay(853);
  Serial.println(temp0);
  Serial.println(temp1);
  Serial.println(temp2);
  Serial.println(temp3);
}

void speak_num(int number){
  //this sub-program will be called in sub-program "speak_out()"
    if(number==1){
      tmrpcm.play("m_one.wav"); //one
      delay(453);
    }
    if(number==2){
      tmrpcm.play("m_two.wav"); //two
      delay(499);
    }
    if(number==3){
      tmrpcm.play("m_three.wav"); //three
      delay(406);
    }
    if(number==4){
      tmrpcm.play("m_four.wav"); //four
      delay(401);
    }
    if(number==5){
      tmrpcm.play("m_five.wav"); //five
      delay(354);
    }
    if(number==6){
      tmrpcm.play("m_six.wav"); //six
      delay(401);
    }
    if(number==7){
      tmrpcm.play("m_seven.wav"); //seven
      delay(633);
    }
    if(number==8){
      tmrpcm.play("m_eight.wav"); //eight
      delay(360);
    }
    if(number==9){
      tmrpcm.play("m_nine.wav"); //nine
      delay(580);
    }
    if(number==0){
      tmrpcm.play("m_zero.wav"); //zero
      delay(610);
    }
}
void message_settings(){
  fonaSS.print("AT+CMGF=1\r");
  delay(100);
  fonaSS.print("AT+CMGR=1\r");
  delay(10); 
  fonaSS.println("AT+CNMI=1,2,0,0,0");
  if(fonaSS.available()>0)
  {
   textMessage = fonaSS.readString();
   delay(10);
  }
  if(textMessage.indexOf("kts2")>=0)
  {   
   Serial.println("Message Received");
   delay(1000);
    //save the phone number of the senders in a string (this works with italian region you must adapt to   yours) 
   CellNumtemp = textMessage.substring(textMessage.indexOf("+234"));
   delay(1000);
   CellNum = CellNumtemp.substring(0,14);
   delay(1000);
   CellNumtemp = "";
   textMessage = ""; 
   fonaSS.println("AT+CMGD=1\r");
   delay(100);
   fonaSS.println("AT+CMGD=2\r");
   delay(100);
  }
  }
  send_sms()
  {
  fonaSS.println("AT+CMGF=1\r");                   //Sets the GSM Module in Text Mode
  delay(800);
  fonaSS.println("AT+CMGS=\"" + CellNum + "\"\r"); //Mobile phone number to send message
  delay(1000);
  fonaSS.println("Alert!!! Present user has a very high temperature.");                        // This is the message to be sent.
  delay(1000);
  fonaSS.println((char)26);                       // ASCII code of CTRL+Z to finalized the sending of sms
  delay(1000);
  }
