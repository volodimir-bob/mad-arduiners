// include the library code:
#include <LiquidCrystal.h>
//#include <SimpleDHT.h>

#include <Keypad.h>

//Software PWM A1-A5
#include <SoftPWM.h>
#include <SoftPWM_timer.h>

//Tone Player for Timer1
#include <TonePlayer.h>

const byte ROWS = 4;
const byte COLS = 4;


char keys[ROWS][COLS] = {
 //14  15  16  17  18  19  
  {36, 5, 56, 66},  //3
  {37, 47, 57, 67},  //4
  {38, 48, 58, 68},  //5
  {39, 49, 59, 69},  //6
  
 
};
byte rowPins[ROWS] = {33, 35, 37, 39};
byte colPins[COLS] = {41, 43, 45, 47};
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );






int music[137] = {
1318,1318,1318,1046,1318,1568,784,1046,784,659,880,987,932,880,784,1318,
1568,1750,1396,1568,1318,1046,1174,987,1046,784,659,880,987,932,880,784,
1318,1568,1750,1396,1568,1318,1046,1174,987,1568,1480,1396,1244,1318,830,880,
1046,880,1046,1174,1568,1480,1396,1244,1318,2093,2093,2093,1568,1480,1396,1244,
1318,830,880,1046,880,1046,1174,1244,1174,1046,1568,1480,1396,1244,1318,
830,880,1046,880,1046,1174,1568,1480,1396,1244,1318,2093,2093,2093,1568,1480,
1396,1244,1318,830,880,1046,880,1046,1174,1244,1174,1046,1046, 1046,1046,1046,
1174,1318,1046,880,784,1046,1046,1046,1046,1174,1318,1046,1046,1046,1046,1174,
1318,1046,880,784,1046,1046,1046,1046,1174,1318}; //Mario

TonePlayer tone1 (TCCR1A, TCCR1B, OCR1AH, OCR1AL, TCNT1H, TCNT1L);  // pin D9 (Uno), D11 (Mega)
#define SPEAKER 11
#include <Encoder.h>

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc(12, 13);
//   avoid using pins with LEDs attached
long oldPosition  = -999;

// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
//int pinDHT11 = 3;
//SimpleDHT11 dht11;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

const uint8_t scroll_bar[4][8] = {
    {B11111, B11111, B00000, B01010, B10101, B01010, B10101, B01010}, //0 
    {B10101, B01010, B00000, B11111, B11111, B00000, B10101, B01010}, //1 
    {B10101, B01010, B10101, B01010, B10101, B00000, B11111, B11111}, //2
    {B10101, B01010, B10101, B01010, B10101, B01010, B10101, B01010},  //3
  };
// encoder Switch
int pinSW = 60;  // Connected enc button to pin A6,  UNO: 20,  Mega: 60

//init menu level

byte h; //humidity
byte t; //temperature
byte poz=0; //set position
int year=2017; //init year 2000-xxxx
byte month=1; //init month 1-12
byte day=1; //init month 1-31
byte weekday=7; //init week day 7 - sunday
byte hour=8;
byte minute=30;
byte second=45;

byte sublev=0; //init sublevel
int sel_item=0; //Selected item
int first_item=0; //Selected menu first item
int last_item=0; //Selected menu last item
int qty=0; //number of menu items
byte menu_sym=0x7e; //menu symbol
int del=250; //delay
byte mcols=16; //lcd display number of cols
byte mrows=2; //lcd display number of rows
bool clr=false;
bool mstate=false;
byte brightness=255; //default brightness
byte volume=50; //default volume
long previousMillis = 0;  // здесь будет храниться время последнего изменения состояния задержки анимации
long interval = 2000;      // интервал начала скроллинга в мс и оно же время убирания функционального меню
long previousMillis2 = 0;  // здесь будет храниться время последнего изменения состояния задержки скролла
long interval2 = 200;      // интервал между буквами скроллинга в мс
bool infunc=false; //показатель что находимся внутри выполнения функции меню
byte m=0; //Current music note
bool lights[5]={
false,
false,
false,
false,
false
};
//set Alarms
typedef struct {
  byte ho;
  byte mi;
  byte se;
  bool al;
  } Alarms;
Alarms alarms[]={
{9,0,0,true},
{12,0,0,false},
{14,45,0,false},
{18,0,0,true},
{22,0,0,false},
{22,30,0,false},
{23,0,0,false}
};
byte light_lev[5]={
255,
255,
255,
255,
255
};
byte light_pins[5]={
  55, //A1 UNO: 15 MEGA: 55 WHITE/BUZZER
  56, //A2 UNO: 16 MEGA: 56 RED
  57, //A3 UNO: 17 MEGA: 57 YELLOW
  58, //A4 UNO: 18 MEGA: 58 GREEN
  59, //A5 UNO: 19 MEGA: 59 BLUE
  };
//debug
bool dbg[5]={
false, //show analog button level & encoder info
true, //enable soft A1-A5 pwm
true, //enable buzzer
true, //enable light sound
true, //enable alarm
};



void lcd_clr() {
//clear screen function
lcd.clear();
lcd.setCursor(0,0);
lcd.print("CLEAR SCREEN");
delay(1000);
lcd_menu();
}

//set Lmenu
typedef struct {
  unsigned char* itemname;
  byte level;
  byte sublevel;
  bool checked;
  bool selected;
  void (*function)();
  } Lmenu;

Lmenu lmenu[]= {
    { "Press UP and DOWN then press SELECT   ",   0,  0,  false,  false, zero },
    { "Temp / Hum",       0,  0,  false,  false, humtemp },
    { "LCD          >",   1,  0,  false,  false, go },
    { "LIGHT        >",   2,  0,  false,  false, go },
    { "SET TIME     >",   3,  0,  false,  false, go },
    { "SYSTEM       >",   4,  0,  false,  false, go },
    { "Volume",           0,  0,  false,  false, set_volume },
    { "TOP",              0,  0,  false,  false, endlev },

    { "LCD MENU",          0,  1,  false,  false, prevlevel },
    { "Brightness",   0,  1,  false,  false, set_brightness },
    { "Brighter",     0,  1,  false,  false, brighter },
    { "Darker",       0,  1,  false,  false, darker },
    { "Lcd ON/OFF",   0,  1,  false,  false, onoff },
    { "BACK",         0,  1,  false,  false, rootlev },

    { "EPILEPTIC WARNING!",    0,  2,  false,  false, prevlevel },
    { "Demo Random",     0,  2,  false,  false, randomlight },
    { "Demo Lights",     0,  2,  false,  false, lightdemo1 },
    { "Demo Strobe",     0,  2,  false,  false, lightdemo2 },
    { "Demo Strobe-2",     0,  2,  false,  false, lightdemo3 },
    { "SOFT PWM",   1,  2,  false,  false, settings },
    { "WHITE ON/OFF",    0,  2,  false,  false, lightonoff  },
    { "RED ON/OFF",    1,  2,  false,  false, lightonoff  },
    { "YELLOW ON/OFF",    2,  2,  false,  false, lightonoff  },
    { "GREEN ON/OFF",    3,  2,  false,  false, lightonoff  },
    { "BLUE ON/OFF",    4,  2,  false,  false, lightonoff  },
    { "WHITE LEVEL",    0,  2,  false,  false, lightlev  },
    { "RED LEVEL",    1,  2,  false,  false, lightlev  },
    { "YELLOW LEVEL",    2,  2,  false,  false, lightlev  },
    { "GREEN LEVEL",    3,  2,  false,  false, lightlev  },
    { "BLUE LEVEL",    4,  2,  false,  false, lightlev  },
    { "BACK",       0,  2,  false,  false, rootlev },
    
    { "SET TIME MENU",   0,  3,  false,  false, prevlevel}, //Menu 3
    { "Time set",       0,  3,  false,  false, timeset }, 
    { "Alarm1 set",       0,  3,  false,  false, alarmset },
    { "Alarm2 set",       1,  3,  false,  false, alarmset },
    { "Alarm3 set",       2,  3,  false,  false, alarmset },
    { "Alarm4 set",       3,  3,  false,  false, alarmset },
    { "Alarm5 set",       4,  3,  false,  false, alarmset },
    { "Alarm6 set",       5,  3,  false,  false, alarmset },
    { "Alarm7 set",       6,  3,  false,  false, alarmset },
    { "ALARM ON/OFF",   4,  3,  false,  false, settings },   
    { "Year",       0,  3,  false,  false, set_year },
    { "Month",      0,  3,  false,  false, set_month },
    { "Day",        0,  3,  false,  false, set_day },
    { "Weekday",    0,  3,  false,  false, set_weekday },
    { "BACK",       0,  3,  false,  false, rootlev },

    { "SYSTEM MENU",      0,  4,  false,  false, prevlevel}, //Menu 4
    { "BTN CODE",   0,  4,  false,  false, settings },
    { "PWM ON/OFF",   1,  4,  false,  false, settings },
    { "BUZZER ON/OFF",   2,  4,  false,  false, settings },
    { "LIGHT SOUND ON/OFF",   3,  4,  false,  false, settings },
    { "BACK",       0,  4,  false,  false, rootlev }
};


void level_recount() {
int i;
int j=0;
for (i=0; i<(sizeof(lmenu)/sizeof(Lmenu)); i++) {
if (sublev==lmenu[i].sublevel) {
if(j==0) {
sel_item=i;
first_item=i;
}
last_item=i;
j++;
}
}
qty=j;
}

void setup() {
  int i=0;
  pinMode (SPEAKER, OUTPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, (uint8_t*)scroll_bar[0]); //0
  lcd.createChar(1, (uint8_t*)scroll_bar[1]); //1
  lcd.createChar(2, (uint8_t*)scroll_bar[2]); //2
  lcd.createChar(3, (uint8_t*)scroll_bar[3]); //3
  lcd.clear();
// examplpe: lcd.print("\0");
  //encoder Switch
   pinMode (pinSW,INPUT);
  //soft pwm
  if(dbg[1]==true) { 
  SoftPWMBegin();
  for (i=0; i<sizeof(lights); i++) {
  setled(light_pins[i],0);
  SoftPWMSetFadeTime(light_pins[i], 100, 550);
  setled(light_pins[i],255);
  delay(100);
  setled(light_pins[i],0);
  delay(100);
  }
  }
  setled(10,brightness); //pin 10 for DFRobot keypad shield 0-255 
  level_recount();
  lcd_menu();
}

void setled( byte pin, byte val) {
if(dbg[1]==true) {
SoftPWMSet(pin, val); 
} else {
analogWrite(pin, val); 
}
}

byte key() {
//right 5 - 0 
//up 3 - 50-200 mega: 98
//down 4 - 200-480 mega: 254
//left 2  - 480-650 mega: 407
//select 1  - 650-800 mega: 640
//rst - RESET

 if (kpd.getKeys())
    {
        
        for (int i=0; i<LIST_MAX; i++)
        {
            
            if ( kpd.key[i].stateChanged )
            {
                
                switch (kpd.key[i].kstate) {
                    case PRESSED:
                    return(kpd.key[i]);
                break;
                    case HOLD:

                break;
                    case RELEASED:
                    
                break;
                    case IDLE:
                    break;
                }
            }
        }
    }
}


}

void ncomm (byte command) {
int j;
lcd.setCursor(0,0);
if(command==0) {
lcd.print("Volume");
return; 
} 
char* item=lmenu[sel_item].itemname;
//substr item name if long
for(j=0; j<strlen(item); j++) { 
if(j<mcols) {
lcd.print(item[j]);
}
}
return; 
}
void scomm (byte command, unsigned int val) {
lcd.setCursor(0,0);
switch (command) {
case 0: 
volume=val; 
break; 
case 1: 
brightness=val; 
bright(brightness);
break;
case 2: 
year=val; 
break; 
case 3: 
month=val; 
break; 
case 4: 
day=val; 
break; 
case 5: 
weekday=val; 
break; 
case 6: 
hour=val; 
break; 
case 7: 
minute=val; 
break; 
case 9:
int s=lmenu[sel_item].level; 
light_lev[s]=val;  
setled(light_pins[s], val); 
break;
}  
}
void randomlight() { 
previousMillis = millis();  
byte z=125;
byte s;
int i;
lcd.clear();
delay(300);
ncomm(9);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > z) {
previousMillis = millis(); 
if (dbg[3]==true) {
tone1.tone(random(100,2000));
}
s=key();
if(s==1)  {
tone1.noTone();  
infunc=false;
lcd_menu();
delay (500);
break;
} 
}
lcd.setCursor(0,1);

i=random(0,sizeof(light_pins));
SoftPWMSetFadeTime(light_pins[i], 2000, 1000);
setled(light_pins[i], 255);
lcd.print("-");
lcd.print(z);
lcd.print("+ ");
delay(s);
i=random(0,sizeof(light_pins));
setled(light_pins[i], 0);
delay(s);
switch(key()) {
case 1:
tone1.noTone();
for(i=0; i<sizeof(light_pins); i++) {
if(lights[i]==true) {
setled(light_pins[i], light_lev[i]); 
} else {
setled(light_pins[i], 0); 
}
}
SoftPWMSetFadeTime(light_pins[i], 100, 550);
infunc=false;
lcd_menu();
delay (500);
break;
case 2: case 4:
if(z>=1) { z--; }
break;
case 3: case 5:
if(z<255) { z++; }
break;
}
} 
}
void lightdemo1 () {
previousMillis = millis();  
byte s;
int i;
int j=0;
byte z=10;
bool e=true;
lcd.clear();
delay(300);
ncomm(9);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > z) {
previousMillis = millis(); 
s=key();
if(s==1)  {
tone1.noTone();
infunc=false;
lcd_menu();
delay (500);
break;
} 
}
lcd.setCursor(0,1);
lcd.print("-");
lcd.print(z);
lcd.print("+ ");
lcd.print(j);
lcd.print("   ");
if (dbg[3]==true) {
tone1.tone(4096-j*128);
}
for (i=0; i<sizeof(lights); i++) {
  SoftPWMSetFadeTime(light_pins[i], j*10, j*20);
  setled(light_pins[i],255);
  delay(j*10);
  setled(light_pins[i],0);
}
tone1.noTone();
delay (j*10/sizeof(lights));
if(j>=z) {e=false; }
if(j<=1) {e=true; } 
if(e==true) {j++;} else { j--;}

switch(key()) {
case 2: case 4:
if(z>=1) { z--; }
break;
case 3: case 5:
if(z<255) { z++; }
break;
case 1:
tone1.noTone();
for(i=0; i<sizeof(light_pins); i++) {
if(lights[i]==true) {
setled(light_pins[i], light_lev[i]); 
} else {
setled(light_pins[i], 0); 
}
}
SoftPWMSetFadeTime(light_pins[i], 100, 550);
infunc=false;
lcd_menu();
delay (500);
break;
}
}
}
void lightdemo2 () {
//strobe
previousMillis = millis();  
byte s;
byte z=40;
int i;
lcd.clear();
delay(300);
ncomm(9);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > z) {
m++;  if(m>=137) { m=0; }
if (dbg[3]==true) {
tone1.tone(music[m]);
}
previousMillis = millis(); 
s=key();
if(s==1)  {
tone1.noTone();
infunc=false;
lcd_menu();
delay (500);
break;
} 
}

int j=0;
int e=random(0,sizeof(lights));
lcd.setCursor(0,1);
lcd.print("-");
lcd.print(z);
lcd.print("+  ");
lcd.print(music[m]);
lcd.print("  ");
analogWrite(light_pins[e],255);
delay(z);
analogWrite(light_pins[i],0);
delay(z*2);
switch(key()) {
case 2: case 4:
if(z>=1) { z--; }
break;
case 3: case 5:
if(z<255) { z++; }
break;
case 1:
if (dbg[3]==true) {
tone1.noTone();
}
for(i=0; i<sizeof(light_pins); i++) {
if(lights[i]==true) {
analogWrite(light_pins[i], light_lev[i]); 
} else {
analogWrite(light_pins[i], 0); 
}
}
infunc=false;
lcd_menu();
delay (500);
break;
}
}
}

void lightdemo3 () {
previousMillis = millis();  
byte s;
int i;
int j=0;
byte z=20;
bool e=true;
lcd.clear();
delay(300);
ncomm(9);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > 250) {
previousMillis = millis(); 
s=key();
if(s==1)  {
tone1.noTone();
infunc=false;
lcd_menu();
delay (500);
break;
} 
}
lcd.setCursor(0,1);
lcd.print("-");
lcd.print(z);
lcd.print("+ ");
lcd.print(j);
lcd.print("   ");
for (i=0; i<sizeof(lights); i++) {
  analogWrite(light_pins[i],255);
  delay(j*10);
  analogWrite(light_pins[i],0);
}
delay (j*10/sizeof(lights));
if(j>=z) {e=false; }
if(j<=1) {e=true; } 
if(e==true) {j++;} else { j--;}

switch(key()) {
case 2: case 4:
if(z>=1) { z--; }
break;
case 3: case 5:
if(z<255) { z++; }
break;
case 1:
tone1.noTone();
for(i=0; i<sizeof(light_pins); i++) {
if(lights[i]==true) {
analogWrite(light_pins[i], light_lev[i]); 
} else {
analogWrite(light_pins[i], 0); 
}
}
infunc=false;
lcd_menu();
delay (500);
break;
}
}
}


void show_humtemp(byte command, byte h, byte t) { 
byte s;
lcd.clear();
delay (500);

previousMillis = millis(); 
ncomm(command);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > interval) {
previousMillis = millis(); 
s=key();
if(s==1)  {
tone1.noTone();  
infunc=false;
lcd_menu();
delay (500);
break;
} 
}
lcd.setCursor(0,1);
//if (dht11.read(pinDHT11, &t, &h, NULL)) {
lcd.print("--");
lcd.print("*C ");
lcd.print("--");
lcd.print("%");
return;
//} 
lcd.print(t);
lcd.print("*C   ");
lcd.print(h);
lcd.print("%");
delay(200);
switch(key()) {
case 1:
infunc=false;
lcd_menu();
delay (500);
break;
}
}
}

byte set_bool(byte command, bool val, int i) {
previousMillis = millis();  
byte s;
lcd.clear();
ncomm(command);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > interval) {
s=key();
if((s==0)||(s==1))  {
tone1.noTone();  
infunc=false;
lcd_menu();
delay (500);
break;
} 
} 

lcd.setCursor(0,1);
if(val==true) {
 if(command==9) { 
  setled(light_pins[i], light_lev[i]);
 }
lcd.print("\3\3");
lcd.print((char)255);
lcd.print((char)255);
lcd.print(" ON  ");
} else {
if(command==9) { 
setled(light_pins[i], 0); 
}
lcd.print((char)255);
lcd.print((char)255); 
lcd.print("\3\3");
lcd.print(" OFF ");
}
switch(key()) {
case 1:
infunc=false;
lcd_menu();
delay (500);
break;
 case 2: case 4: 
 previousMillis = millis();  
 val=false;
 scomm(command,val);
 break;
 case 3: case 5:
 previousMillis = millis();  
 val=true;
 scomm(command,val);
 break;
}
}
return val;
}
void alarmset() {
clockset(1);  
}
void timeset() {
clockset(0);  
}
void clockset(byte command) {
infunc=true;
lcd.clear();
byte s;
byte i;
byte hhh;
byte mmm;
byte sss;
bool alal;
byte maxpoz=3;
int j=lmenu[sel_item].level; 
switch (command) {
  case 1: //alarm
  hhh=alarms[j].ho;
  mmm=alarms[j].mi;
  sss=alarms[j].se; 
  alal=alarms[j].al;  
  break;
  case 0: //time
  maxpoz=2;
  hhh=hour;
  mmm=minute;
  sss=second;
  break;
}
poz=0;
ncomm(1);
delay(500);
while(infunc==true) {
lcd.setCursor(0,1);
for(i=0; i<100; i++) {
s=key();
if(s==1) { 
if(poz==maxpoz) {
tone1.noTone();
delay (500);
infunc=false;
lcd_menu();
return;  
} else {
poz++;
break;   
}
}
if(s==2) { if(poz>0) {poz--; break; } }
if(s==5) { if(poz<maxpoz) {poz++; break; } }
if(poz==0) {
if(s==3) { if(hhh<23) { hhh++; } else {hhh=0; } tone1.tone(2000);delay(2);tone1.noTone();  break; } 
if(s==4) { if(hhh>0) { hhh--; } else { hhh=23; } tone1.tone(2000);delay(2);tone1.noTone();  break;}      
}
if(poz==1) {
if(s==3) { if(mmm<59) { mmm++; } else { mmm=0;} tone1.tone(2000);delay(2);tone1.noTone();  break; } 
if(s==4) { if(mmm>0) { mmm--; }  else {mmm=59;} tone1.tone(2000);delay(2);tone1.noTone();  break; }   
}
if(poz==2) {
if(s==3) { if(sss<59) { sss++; } else { sss=0;} tone1.tone(2000);delay(2);tone1.noTone();  break; } 
if(s==4) { if(sss>0) { sss--; }  else {sss=59;} tone1.tone(2000);delay(2);tone1.noTone();  break; }   
}
if(poz==3) {
if((s==3)||(s==4)) { 
if(alal==true) { 
  alal=false; 
  } else { 
    alal=true;
    }
    tone1.tone(2000);
    delay(2);
    tone1.noTone();
    break;  
}
}
}
if(hhh<10) {
lcd.print("0");
}
lcd.print(hhh);
if(poz==0) {lcd.print(char(0x7f));} else { lcd.print(":"); }
if(mmm<10) {
lcd.print("0");
}
lcd.print(mmm);
if(poz==1) {lcd.print(char(0x7f));} else { lcd.print(":"); }
if(sss<10) {
lcd.print("0");
}
lcd.print(sss);
if(poz==2) {lcd.print(char(0x7f));} else { lcd.print(" "); } 
if(maxpoz==3) { 

if(alal==true) {
lcd.print(" ON");
} else {
lcd.print("OFF");  
}
if(poz==3) {lcd.print(char(0x7f));}
}
switch (command) {
  case 1: //alarm
  alarms[j].ho=hhh;
  alarms[j].mi=mmm;
  alarms[j].se=sss;
  alarms[j].al=alal;  
  break;
  case 0: //time
  hour=hhh;
  minute=mmm;
  second=sss;
  break;
}
}
}

byte set_var(byte command, int val, int maxlevel, int minlevel, int steps) {
previousMillis = millis();  
byte s;
lcd.clear();
ncomm(command);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > interval) {
s=key();
if((s==0)||(s==1))  {
if( val<=minlevel) { val=minlevel; } 
if( val>=maxlevel) { val=maxlevel; } 
scomm(command,val);
tone1.noTone();  
infunc=false;
lcd_menu();
delay (500);
break;
} 
} 
lcd.setCursor(0,1);
for (s=0;s<10;s++) {
if (floor(val/(floor(maxlevel)/10))>s) { lcd.print((char)255); } else { lcd.print("\3"); }
}
if(val<10) {lcd.print(" ");}
if(val<100) {lcd.print(" ");}
if(val<1000) {lcd.print(" ");}
if(val<10000) {lcd.print(" ");}
if(val<100000) {lcd.print(" ");}
lcd.print(val);
switch(key()) {
case 1:
infunc=false;
lcd_menu();
delay (500);
break;
 case 2: case 4:
 previousMillis = millis();   
 if( val>minlevel) { val-=steps; } 
 if( val<=minlevel) { val=minlevel; } 
 scomm(command,val);
 break;
 case 3: case 5: 
 previousMillis = millis();    
 if( val<maxlevel) { val+=steps;}
 scomm(command,val);
 break;
}
}
return val;
}
void set_volume() {
set_var(0, volume, 100, 0, 5);
}
void set_brightness() {
set_var(1, brightness, 255, 0, 5);
}
void set_year() {
set_var(2, year, 2100, 2000, 1);
}
void set_month() {
set_var(3, month, 12, 1, 1);
}
void set_day() {
set_var(4, day, 31, 1, 1);
}
void set_weekday() {
set_var(5, weekday, 7, 1, 1);
}
void set_hour() {
set_var(6, hour, 23, 0, 1);
}
void set_minute() {
set_var(7, minute, 59, 0, 1);
}

//debug func
void settings() {
int s=lmenu[sel_item].level;
dbg[s]=set_bool(8, dbg[s], 0);
}
void lightlev() {
int s=lmenu[sel_item].level;  
set_var(9, light_lev[s], 255, 0, 1);
}
void light() {
int s=lmenu[sel_item].level;
lights[s]=set_bool(9, lights[s], s);
}

void humtemp() {
show_humtemp(10, h, t);
}
void lightonoff() {
int s=lmenu[sel_item].level;
bool st=lights[s];
if(st==true){ 
lights[s]=false; 
setled(light_pins[s],0);
} else { 
lights[s]=true;
setled(light_pins[s],light_lev[s]);
}
delay(500);
}
void onoff() {
if(brightness==0){ brightness=255; } else { brightness=0; }
setled(10,brightness);
delay(500);
}

void bright(byte val) {
setled(10,val);
}
void brighter() {
if(brightness<255) {brightness+=1;}
bright(brightness);
delay(10);
}
void darker() {
if(brightness>=1) {brightness-=1;}
bright(brightness);
delay(10); 
}
void go() {
if (lmenu[sel_item].level>0) {
lcd.clear();
infunc=true;
sublev=lmenu[sel_item].level;
} 
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;  
}
void zero() {
return;
}
void endlev() {
lcd.clear();
infunc=true;
level_recount();
sel_item=first_item;
delay(200);
infunc=false;
lcd_menu();
return;
}
void nextlevel() {
lcd.clear();
infunc=true;
sublev++;
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}
void timelevel() {
lcd.clear();
infunc=true;
sublev=3;
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}
void prevlevel() {
lcd.clear();
infunc=true;
if(sublev>=1) { sublev--; }
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}

void rootlev() {
lcd.clear();
infunc=true;
sublev=0;
sel_item=0;
level_recount();
delay(200); 
infunc=false;
lcd_menu();
return;
}

void loop() {
//show menu
int i=0;
for(i=0;i<sizeof(light_pins);i++) {
pinMode(light_pins[i],OUTPUT);  
}

if (millis() - previousMillis > interval) {
previousMillis = millis();  
mstate=true;
if(infunc==false) {
str_animate();
} 
}
if(clr==false) {
switch(key()) {
  
case 0: //Not press state
clr==false;
break;

case 1: //Select
if(infunc==false) {
run_menu();
} else {
infunc=false;
lcd_menu();
delay(1000);
}
break;

case 2: //Left
if(infunc==false) {
if(volume>5) { volume-=5; }
set_var(0, volume ,100,0, 5);
}
break;

case 3: //Up
if(infunc==false) {
sel_item--;
if(sel_item<first_item) { 
sel_item=first_item; }
clr=true;
}
break;

case 4: //Down
if(infunc==false) { 
if(sel_item<last_item) {  
sel_item++; 
}
clr=true;
}
break;

case 5: //Right
if(infunc==false) {
if(volume<100) { volume+=5; }
set_var(0, volume ,100, 0, 5);
}
break;
}

} else {
if(infunc!=true) {
if(key()==0) {
lcd_menu();
}
}
}
}



void lcd_menu() {
mstate=false;
infunc=false;
int i;
int j;
int row=0;
clr=false;
for (i=first_item; i<=last_item; i++) {
if ((i>=sel_item)&&(row<mrows)&&(sublev==lmenu[i].sublevel)) {
lcd.setCursor(0, row);
if(i==sel_item) {
lcd.print ((char)menu_sym); 
} else {
lcd.print(" "); 
}
//sprintf(nav," %d/%d",(i+1),qty);
int procent=ceil((sel_item-first_item)*100/qty);
char* item=lmenu[i].itemname;

//substr item name if long
if(strlen(item)>(mcols-2)) {mstate=true;}
for(j=0; j<strlen(item); j++) { 
if(j<(mcols-2)) {
lcd.print(item[j]);
}
}
//print spacer to the end
for(j=(strlen(item)+1); j<(mcols-1); j++) { 
lcd.print(" ");
}

//create right navbar
if ((i==(qty-1))||(row==(mrows-1)))  { 
//низ
if(procent>=55) {
if(procent>=70) {
if(procent>=90) {
lcd.write((uint8_t)2);    
} else {
lcd.write((uint8_t)1);  
}   
} else {
lcd.write((uint8_t)0); 
}
} else {
lcd.write((uint8_t)3);   
}
} else {
if ((i==first_item)||(i==sel_item)){
//верх
if(procent<=55) {
if(procent<=35) {
if(procent<=20) {
lcd.write((uint8_t)0);   
} else{
lcd.write((uint8_t)1);   
} 
} else {
lcd.write((uint8_t)2); 
}
} else {
lcd.write((uint8_t)3);   
}
} else {
if (sel_item==qty){
//last element on the screen
lcd.write((uint8_t)2);  
} else {
//other ||
lcd.write((uint8_t)3);
}

}
}
row++;
}
}

//clear last stroke
if(row<=(mrows-1)) {
for(j=row; j<mcols; j++) {
lcd.setCursor(0, j);  
for(i=0; i<=mcols; i++) {
lcd.print(" ");
}
}
}
}
void run_menu() {
 if (dbg[2]==true) {
tone1.tone(220);  // 220 Hz
  delay (50);
  tone1.noTone();

  tone1.tone(440);
  delay (50);
  tone1.noTone();

  tone1.tone(880);
  delay (50);
 }
  tone1.noTone();
lmenu[sel_item].function();
}


void str_animate() {
int j;
int i;
if (infunc==false) {
int val=analogRead(0);
long newPosition = myEnc.read();
char* item=lmenu[sel_item].itemname;

if((mstate==true)&&(strlen(item)>(mcols-2))) {
//animation
for(i=0; i<strlen(item); i++) { 
lcd.setCursor(1, 0); 
//delay
while (millis() - previousMillis2 < interval2) {  
val=analogRead(0);
newPosition = myEnc.read();
if((val<1000)||(newPosition!=oldPosition)||(key()==1)) {mstate==false; return;}
} //EOF delay
previousMillis2 = millis();
int s=0;
for(j=i; j<(i+strlen(item)+mcols-2); j++) {
if(s>(mcols-3)) { break; }
if(j<strlen(item)) {
lcd.print(item[j]);
} else {
lcd.print(item[j-strlen(item)]);  
}
s++;
}
}

} else {
 //no amimation
lcd.setCursor(1, 0);
for(j=0; j<strlen(item); j++) { 
if(j<(mcols-2)) {
lcd.print(item[j]);
}
}
mstate=false;
return;
}
} else {
if (millis() - previousMillis > interval) {
previousMillis = millis();  
delay(interval);
infunc=false;
lcd_menu();
} 
}
}