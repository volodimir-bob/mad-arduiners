//#define BLYNK_PRINT Console
//#include <Console.h>
#include <Wire.h> 
//#include <Bridge.h>
#include <SimpleTimer.h>
//#include <BlynkSimpleYun.h>
#include <SPI.h>
#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <RFID.h> 
#include <DHT.h>

//
//server
#include <YunClient.h>
#include <PubSubClient.h>
#include <Bridge.h>
#define MQTT_SERVER "192.168.137.117"

#define MQTT_CLIENTID "paho53523262479"
#define ledPin 13
#define DHTPIN          5
#define relayPin 3
#define DHTTYPE         DHT11
#define PUBLISH_DELAY   30000
void callback(char* topic, byte* payload, unsigned int length);
YunClient yun;
PubSubClient mqtt(MQTT_SERVER, 1883, callback, yun);
long previousMillis;
char msgBuffer[20];
void callback(char* topic, byte* payload, unsigned int length);
// /server
bool auth = false;

bool pisk = true;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).

//??char auth[] = "82a1ce85272647fc9a2c3c7d6a16cca5";

SimpleTimer timer;
//************************I2C 1602 LCD Display
LiquidCrystal_I2C lcd(0x27,16,2);
//************************Servo/Keypad
Servo myservo;

/*
BLYNK_WRITE(V3)
{
  myservo.write(param.asInt());
}
*/

#define Password_Length 8 
//int signalPin = 12;
char Data[Password_Length]; 
char Master[Password_Length] = "123A456"; 
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {47, 45, 43, 41}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {39, 37, 35, 33}; //connect to the column pinouts of the keypad
 
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

//************************RFID
RFID rfid(48,49);   //sda:pin48,rst:pin49
bool state=true;

//***********************DHT11
#define DHTPIN 5          // What digital pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301
DHT dht(DHTPIN, DHTTYPE);
float h;  //humidity
float t;  //Temperature

//BlynkTimer timer;
// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.

//******************************Light sensor/Utrasonic sensor/ LED
#define echoPin 29
#define trigPin 27
int lightsensorPin = A0;    // select the input pin for the photoresistor
int light_val = 0;       // variable to store the value coming from the sensor 
float duration, distance;
int LED = 26;   // select the pin26 for the LED 

//****************************Widget LED
/*
WidgetLED led1(V0); //register to virtual pin 0
WidgetLED led2(V1); //register to virtual pin 1
WidgetLED led3(V2); //register to virtual pin 2
WidgetLED led4(V4); //register to virtual pin 4
*/
//************************************************************************smoke sensor mq2
//connect to A1
 int smoke_mq2 = A1;
int smoke = 0; 

//************************************************************************FLAME sensor
//connect to A2
int FlamePin = A2;  // This is for input pin
int Flame = 0;  // HIGH when FLAME Exposed

//************************************************************************BUZZER
int Buzzer = 31;//the pin of the active buzzer
//************************************************************************RGB1
const int redPin1 = 8; // R1 connected to digital pin 8 
const int greenPin1 = 9; // G1 to digital pin 9 
const int bluePin1 = 10; // B1 connected to digital pin 10 
//************************************************************************RGB2
const int redPin2 = 11; // R2 connected to digital pin 11 
const int greenPin2 = 12; // G2 to digital pin 12 
const int bluePin2 = 13; // B2 connected to digital pin 13 

//************************************************************************2 CHANNEL RELAYS
//the relays connect to
int IN1 = 4;
int IN2 = 3;

//************************************************************************Water sensor
//the signal pin connect to D7
int waterSensor = 7;
int waterValue = 0;

//************************************************************************PIR_HC_SR501

//Connect to D24
int pir_hc_sr501 = 24;
int LED1 = 13;                // choose the pin for the LED
//int relayInput = 3;             // choose the pin for the relay
int pirState = LOW;             // we start, assuming no motion detected
int pirval = 0;                    // variable for reading the pin status
bool pirStatus=false;


//rgb
void setColor1(int red, int green, int blue)
{
  analogWrite(redPin1, red);
  analogWrite(greenPin1, green);
  analogWrite(bluePin1, blue);  
}

void setColor2(int red, int green, int blue)
{
  analogWrite(redPin2, red);
  analogWrite(greenPin2, green);
  analogWrite(bluePin2, blue);  
}
void Page1();
//keypad
void KEYPADSENSOR() {
   // Serial.println("KEYPAD START!");
 if (auth==false)
 {
//char customKey;
  customKey = customKeypad.getKey();
 // Serial.println("KEYPAD START!");
  lcd.setCursor(0,0);
  lcd.print("Enter Password:");
 // Serial.println("Enter Password:");
  if (customKey){
    Data[data_count] = customKey; 
    lcd.setCursor(data_count,1); 
    lcd.print(Data[data_count]); 
    Serial.println(Data[data_count]);
    data_count++; 
    }

  if(data_count == Password_Length-1){
    lcd.clear();

    if(!strcmp(Data, Master)){
      setColor1(0, 255, 0);  // set rgb1 green
      lcd.print("Correct");
      Serial.println("Password Correct!");
      //digitalWrite(signalPin, HIGH); 
      myservo.write(90);//goes to 90 degrees
      //lcd.print("1602 LCD IS READY");
      //delay(1000);//wait for a second
      delay(1000);
      myservo.write(0);//back to 0 degrees 
      setColor1(0, 0, 0);
      lcd.clear();
      Page1();
      lcd.setCursor(0, 0);
      auth=true;
  
//      MENU(int t, int h);
      }
    else{
      lcd.print("Incorrect");
      setColor1(255, 0, 0);  // set rgb1 red
      delay(1000);
      setColor1(0, 0, 0);
      lcd.clear();
      } 
    clearData();  
  }
 }
 else
 {customKey = customKeypad.getKey();
 if(customKey)MENU();}
}

void clearData(){
  while(data_count !=0)
  {
    Data[data_count--] = 0; 
  }
  return;
  lcd.clear();
}

//menu
char commands[8][12]={"Temperature", "Light", "Pir", "Flame", "Smoke", "Buzzer", "Exit"};
int pointer=0;
int level=0;
void Page1()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(commands[pointer]);
  lcd.setCursor(0,1);
  lcd.print(commands[pointer+1]);
}

void MENU()
{
  if(auth)
  {
  switch(level)
  {
    case 0:
    if(customKey=='2'&&pointer>0)pointer--;
    if(customKey=='8'&&pointer<6)pointer++;
    if(customKey=='A')level=pointer+1;
    if(!level)Page1();
    else MENU();
    break;
    case 1:   //DHT
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Humidity: ");
      lcd.print(h);
      lcd.setCursor(0,1);
      lcd.print("Temperat: ");
      lcd.print(t);
      if(customKey=='B'){
      Page1();
      level=0;
    }
    break;
    case 2:     //light
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("light_val = ");
      lcd.print(light_val);   //The serial will print the light value
      lcd.setCursor(0,1);
      lcd.print("Distance = ");
      lcd.print(distance);
      lcd.print(" cm");
    if(customKey=='B')
    {
      Page1();
      level=0;
    }
    break;
    case 3:
    lcd.clear();
    if(pirStatus)lcd.print("Motion detected!");
    else lcd.print("No motion");
    if(customKey=='B'){
      Page1();
      level=0;
    }
    if(customKey=='B')level=0;
    break;
    case 4:     //flame
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("flame_val = ");
      lcd.print(Flame);
    if(customKey=='B'){
      Page1();
      level=0;
    }
    break;
    case 5:   //smoke
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("smoke_val = ");
    lcd.print(smoke);
    if(customKey=='B'){
      Page1();
      level=0;
    }
    break;
    case 6://Buzzer
    lcd.clear();
    lcd.print("* to ON ");
    if(pisk)lcd.print("   <-");
    lcd.setCursor(0,1);
    lcd.print("# to OFF");
    if(!pisk)lcd.print("   <-");
    if(customKey=='*'){pisk=true;digitalWrite(Buzzer, HIGH);}
    else if(customKey=='#'){pisk=false;digitalWrite(Buzzer, LOW);}
    else if(customKey=='B'){
      Page1();
      level=0;
    }
    break;
    case 7: //exit
    auth=false;
    level=0;
    pointer=0;
    break;
    default:
    break;
  }
  customKey=' ';
}
/*
Serial.println("menu");
 lcd.clear();  
 lcd.setCursor(0,0);
 lcd.print("menu");
  if (data_count == 1)
 {
 lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(t);  //printing temperarture to the LCD display
 

  lcd.print("'C");
  
  lcd.setCursor(0,1);
  lcd.print("Humid: ");
  lcd.print(h);  //printing humidity to the LCD display
  lcd.print("%");
 } 
 else if(data_count == 2){
 lcd.clear();
  lcd.setCursor(0,0);
 lcd.print("FLAME: ");
 //lcd.print(Flame);
 lcd.setCursor(0,1);
 lcd.print("SMOKE: ");
 //  
// lcd.print(smoke);
 }
 else{
 lcd.setCursor(0,0);
 lcd.print("menu");
 }
*/
}
//rfid

void RFIDSENSOR() {
  if(auth)return;
    Serial.println("RFID START!");
    unsigned char type[MAX_LEN];
    //to find the card
    if (rfid.isCard( )) {
      Serial.println("Find the card!"); 
      // Show card type
      ShowCardType(type);
      //Read card sequence number
      if (rfid.readCardSerial()) {
      Serial.print("The card's number is  : ");
      Serial.print(rfid.serNum[0],HEX);
      Serial.print(rfid.serNum[1],HEX);
      Serial.print(rfid.serNum[2],HEX);
      Serial.print(rfid.serNum[3],HEX);
      Serial.print(rfid.serNum[4],HEX);
      Serial.println(" ");
      ShowUser(rfid.serNum);
      Page1();
      auth=true;
    }
    //Select card, return card capacity (lock card, prevent most read), remove the line to read the card continuously
    Serial.println(rfid.selectTag(rfid.serNum));
  }
  rfid.halt();
}
void ShowCardType( unsigned char* type)
{
  Serial.print("Card type: ");
  if(type[0]==0x04&&type[1]==0x00) 
    Serial.println("MFOne-S50");
  else if(type[0]==0x02&&type[1]==0x00)
    Serial.println("MFOne-S70");
  else if(type[0]==0x44&&type[1]==0x00)
    Serial.println("MF-UltraLight");
  else if(type[0]==0x08&&type[1]==0x00)
    Serial.println("MF-Pro");
  else if(type[0]==0x44&&type[1]==0x03)
    Serial.println("MF Desire");
  else
    Serial.println("Unknown");
}

void ShowUser( unsigned char* id)
{
  //32 DA 94 10
  //Overhere, you need to use your own card message!!!
  if( id[0]==0xC3 && id[1]==0x6C && id[2]==0x4C && id[3]==0xD3
 ) {
    //Serial.println("Hello Mary!");
    lcd.clear();
    lcd.print("Hello Mary!");
    setColor1(0, 255, 0);  // set rgb1 green
    myservo.write(90);//goes to 90 degrees
    // state=RelayStatus(state);
   delay(3000);
   setColor1(0, 0, 0);
   myservo.write(0);//back to 0 degrees 
  } 
  //75 C0 D0 6
  //Overhere, you need to use your own card message!!!
  else if(id[0]==0x76 && id[1]==0x0C && id[2]==0x98 && id[3]==0x1A) {
    Serial.println("Hello MicroHao!");
    myservo.write(90);//goes to 90 degrees
    setColor1(0, 255, 0);  // set rgb1 green
    Serial.println("Open th door");
    delay(3000);
    myservo.write(0);//back to 0 degrees 
    setColor1(0, 0, 0);
    Serial.println("Close the door");
    //state=RelayStatus(state);
  }
  else{
    Serial.println("Hello unkown guy!");
    setColor1(255, 0, 0);  // set rgb1 red
    delay(500);
    setColor1(0, 0, 0);
    delay(500);
    setColor1(255, 0, 0);  // set rgb1 red
    delay(500);
    setColor1(0, 0, 0);
  }
}

//dht

void DHTSENSOR ()
{
  Serial.println("DHT START!");
  h = dht.readHumidity();
  t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
   // lcd.setCursor(0, 0);
    lcd.print("Failed to read from DHT sensor!    : ");
   //   lcd.clear();
 delay(500);
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  mqtt.publish("Home/Kitchen/Kitchen_Temperature", dtostrf(t, 6, 2, msgBuffer));
  mqtt.publish("Home/Kitchen/Kitchen_Humidity", dtostrf(h, 6, 2, msgBuffer));
  // You can send any value at any time.
  // Please don't send more that 10 values per second.

 /* Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);*/
 
  delay(500);
}

//Light / Ultrasonic

void LIGHTSENSOR () {
  Serial.println("LIGHT SENSOR AND HC-SR04 START!");
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
 
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) * 0.0344;
  
light_val = analogRead(lightsensorPin);   // read the value from the sensor  
    Serial.print("light_val = ");
    Serial.println(light_val);   //The serial will print the light value
    Serial.print("Distance = ");
    Serial.print(distance);
    Serial.println(" cm");
  if(light_val>=400 && distance <= 10  )       // the point at which the state of LEDs change 
  {
    digitalWrite(LED, HIGH);
    delay(1000);
  }
  else
  {
    digitalWrite(LED, LOW);  //set LED off
  } 
}

//*********************************************  PIR HC-SR501
void PIRSENSOR () {
    Serial.println("PIR SENSOR START!");
    pirval = digitalRead(pir_hc_sr501);  // read input value
    if (pirval == HIGH) {            // check if the input is HIGH
    //digitalWrite(LED, HIGH);  // turn LED ON
//    led2.on(); // set LED on
    if (pirState == LOW) {
    // we have just turned on
    pirStatus=true;
    Serial.println("Motion detected!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    mqtt.publish("Home/Kitchen/Kitchen_Motion", "1");
   // mqtt.publish("Home/Bathroom/Bathroom_Motion", true);
    setColor2(255, 255, 255);  // set rgb1 
   
    // We only want to print on the output change, not state
    pirState = HIGH;
    }
  } else {
//    led2.off();  //set LED off
     setColor2(0, 0, 0);
    if (pirState == HIGH){
    // we have just turned of
    pirStatus=false;
    Serial.println("Motion ended!");
    mqtt.publish("Home/Kitchen/Kitchen_Motion", "0");
//    mqtt.publish("Home/Bathroom/Bathroom_Motion", true);
    // We only want to print on the output change, not state
    pirState = LOW;
    }
  }
}

//*********************************************  Flame Sensor
void FLAMESENSOR () {

    Serial.println("FLAME SENSOR START!");
    Flame = analogRead(FlamePin);
    Serial.print("flame_val = ");
    Serial.println(Flame);   //The serial will print the fire value
    mqtt.publish("Home/Bedroom/Bedroom_Flame", itoa(Flame,msgBuffer,10));
  if(Flame>=1050)       // the point at which the state of LEDs change 
  {
    setColor2(255, 0, 0); // turn the RGB LED red 
    delay(500); 
    setColor2(0, 0, 0);
    delay(500); 
    setColor2(255, 0, 0); // turn the RGB LED red 
    delay(500); 
    setColor2(0, 0, 0);
    delay(500); 
    digitalWrite(Buzzer, LOW);
    delay(10000);
    
//    led3.on();

  }
  else
  {
    Serial.println("No Fire!");
    digitalWrite(Buzzer, HIGH);
//    led3.off();

  } 
    delay(2000);
}
//*********************************************  MQ2 Smoke sensor
void SMOKESENSOR () {
    Serial.println("SMOKE SENSOR START!");
    smoke = analogRead(smoke_mq2);
    mqtt.publish("Home/Bedroom/Bedroom_Smoke", itoa(smoke,msgBuffer,10));
    Serial.print("smoke_val = ");
    Serial.println(smoke);   //The serial will print the smoke value
  if(smoke>=540)       // the point at which the state of LEDs change 
  {
    
    setColor2(255, 0, 0); // turn the RGB LED red 
    delay(2000); 
    setColor2(0, 0, 0);
    delay(2000); 
    setColor2(255, 0, 0); // turn the RGB LED red 
    delay(2000); 
    setColor2(0, 0, 0);
    digitalWrite(Buzzer, LOW);
    delay(20000);
    //mqtt.publish("Home/Bedroom/Bedroom_Smoke", "true");
//    led1.on();
  }
  else
  {
    if (pisk == true)
    {
    Serial.println("No smoke!");
    //mqtt.publish("Home/Bedroom/Bedroom_Smoke", "false");
    digitalWrite(Buzzer, HIGH);
    }
//    led1.off();
  } 
  //  delay(500);
}

//**********************void setup
void setup()
{
  Serial.begin(9600);
 // Blynk.begin(auth); 
  Bridge.begin();
  //Console.begin();
  SPI.begin();
  dht.begin();  
  
  myservo.attach(2);
  myservo.write(0);//back to 0 degrees 
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear();
  rfid.init();
 // pinMode(led, OUTPUT);  
  //pinMode(relay,OUTPUT);
 // digitalWrite(relay,HIGH);
  // Setup a function to be called every second
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED, OUTPUT);      // declare LED as output
  pinMode(pir_hc_sr501, INPUT);     // declare sensor as input
  pinMode(Buzzer, OUTPUT);      // declare Buzzer as output
  pinMode(redPin1, OUTPUT);
  pinMode(greenPin1, OUTPUT);
  pinMode(bluePin1, OUTPUT);  
//Bridge.begin();
  if (mqtt.connect(MQTT_CLIENTID)) {
    mqtt.setCallback(callback);
    Serial.println(F("DHT sensor initialized"));
    Serial.println();
    Serial.println(F("Ready to send data"));
    previousMillis = millis();
  }
  timer.setInterval(100, KEYPADSENSOR);
  timer.setInterval(1000, MENU);
  timer.setInterval(1000, RFIDSENSOR);
  timer.setInterval(10000, DHTSENSOR);
  timer.setInterval(10000, FLAMESENSOR);
  timer.setInterval(10000, SMOKESENSOR);
  timer.setInterval(10000, LIGHTSENSOR);
  timer.setInterval(3000, PIRSENSOR);
  while (!Serial);{}
    
}

//**********************void loop
void loop() 
{
  // this is where the "polling" occurs
  timer.run();
 // Blynk.run();
  digitalWrite(Buzzer, HIGH);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");//MQTT_BROKER
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println(payload[0]);

 
  if (strncmp((const char*)payload, "ON", 2) == 0) {
    digitalWrite(ledPin, HIGH);    //
    digitalWrite(relayPin, HIGH);
  }
  if (strncmp((const char*)payload, "OFF", 3) == 0) {
    digitalWrite(relayPin, LOW);     //
    digitalWrite(ledPin, LOW);
  }
  
}
