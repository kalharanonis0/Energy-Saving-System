#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define US1_TRIG 2
#define US1_ECHO 3
#define US2_TRIG 4
#define US2_ECHO 5
#define PIR_PIN  6
#define RELAY1_PIN 7   
#define RELAY2_PIN 8   
#define BUZZER_PIN 9   


#define BTN_MENU   10   
#define BTN_LEFT   11   
#define BTN_RIGHT  12   
#define BTN_OK     13   


int sensorSensitivity = 20;             
const int SENS_MIN = 5;                 
const int SENS_MAX = 100;               
const int SENS_STEP = 5;                
const unsigned long SEQ_TIMEOUT = 1500; 


unsigned long pirHoldTime = 0;            
const unsigned long PIR_HOLD_MIN = 0;     
const unsigned long PIR_HOLD_MAX = 10000; 
const unsigned long PIR_HOLD_STEP = 500;  
bool pirRawState = false;        
unsigned long lastMotionTime = 0;

int persons = 0;
bool lightOn = false;
unsigned long personsZeroTime = 0;
const unsigned long LIGHT_OFF_DELAY = 1000;

enum SeqState { SEQ_NONE, SEQ_S1_FIRST, SEQ_S2_FIRST };
SeqState seqState = SEQ_NONE;
unsigned long seqTime = 0;

bool s1Last = false, s2Last = false;


enum AppState { STATE_BOOT, STATE_HOME, STATE_MENU, STATE_PAGE, STATE_SETTING_DETAIL, STATE_CONFIRM_RESTART };
AppState currentState = STATE_BOOT;

const int NUM_ICONS = 4;
const char* iconNames[NUM_ICONS] = {"Home", "Info", "Settings", "Restart"};
int menuIndex = 0;

int settingIndex = 0;          
int openSettingIndex = -1;     
const int NUM_SETTINGS = 2;
const char* settingNames[NUM_SETTINGS] = {"Sensitivity", "PIR Sensor"};


void (*resetFunc)(void) = 0; 


bool lastOk = HIGH, lastMenu = HIGH, lastLeft = HIGH, lastRight = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150;


int spinnerFrame = 0;
unsigned long lastSpinnerUpdate = 0;
const unsigned long spinnerInterval = 120; 


#define ANIM_FRAME_DELAY (42)
#define ANIM_FRAME_WIDTH (32)
#define ANIM_FRAME_HEIGHT (32)
#define ANIM_FRAME_COUNT (sizeof(bootFrames) / sizeof(bootFrames[0]))

const byte PROGMEM bootFrames[][128] = {
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,3,0,0,224,7,0,1,112,14,128,2,56,28,64,4,28,56,32,8,14,112,16,16,7,224,8,32,3,192,4,32,1,128,4,16,2,64,8,8,4,32,16,4,8,16,32,2,16,8,64,1,32,4,128,0,192,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,3,0,0,224,7,0,1,112,14,128,2,56,28,64,4,28,56,32,8,14,112,16,16,7,224,8,32,3,192,4,32,1,128,4,16,2,64,8,8,4,32,16,4,8,16,32,2,16,8,64,1,32,4,128,0,192,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,3,0,0,224,7,0,1,112,14,128,2,56,28,64,4,28,56,32,8,14,112,16,16,7,224,8,32,3,192,4,32,1,128,4,16,2,64,8,8,4,32,16,4,8,16,32,2,16,8,64,1,32,4,128,0,192,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,2,0,0,224,6,0,1,112,15,0,2,56,29,128,4,28,56,192,8,14,112,96,16,7,224,48,32,3,192,24,32,1,128,12,16,3,128,8,8,4,192,16,4,8,96,32,2,16,48,64,1,32,24,128,0,192,13,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,2,0,0,224,2,0,1,112,4,0,2,56,28,0,4,28,60,0,8,14,114,0,16,7,225,0,32,3,192,128,32,3,128,64,16,7,0,32,8,6,0,16,4,14,0,32,2,17,0,64,1,32,128,128,0,192,65,0,0,0,34,0,0,0,28,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,2,0,0,224,2,0,1,112,4,0,2,56,8,0,4,28,16,0,8,14,112,0,16,7,240,0,32,3,200,0,32,3,132,0,16,7,2,0,8,14,1,0,4,28,0,128,2,24,0,192,1,56,0,128,0,196,1,0,0,2,2,0,0,1,4,0,0,0,136,0,0,0,112,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,2,0,0,224,2,0,1,112,4,0,2,56,8,0,4,28,16,0,8,14,32,0,16,7,64,0,32,3,192,0,32,3,224,0,16,7,48,0,8,14,24,0,4,28,12,0,2,56,6,0,1,112,3,0,0,224,1,0,0,48,2,0,0,24,4,0,0,12,8,0,0,6,16,0,0,3,32,0,0,1,192,0,0,0,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,2,0,0,224,2,0,1,112,4,0,2,56,8,0,4,28,16,0,8,14,32,0,16,7,64,0,32,3,128,0,32,3,128,0,16,7,64,0,8,14,32,0,4,28,16,0,2,56,8,0,1,112,4,0,0,224,2,0,0,192,2,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,2,0,0,224,2,0,1,112,4,0,2,56,8,0,4,28,16,0,8,14,32,0,16,7,64,0,32,3,128,0,32,3,128,0,16,7,64,0,8,14,32,0,4,28,16,0,2,56,8,0,1,112,4,0,0,224,2,0,0,192,2,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,192,2,0,0,224,2,0,1,112,4,0,2,56,8,0,4,28,16,0,8,14,32,0,16,7,64,0,32,3,128,0,32,3,128,0,16,7,64,0,8,14,32,0,4,28,16,0,2,56,8,0,1,112,4,0,0,224,2,0,0,192,2,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,128,0,0,1,192,0,0,3,32,0,0,6,16,0,0,12,8,0,0,24,4,0,0,112,2,0,0,224,1,0,1,112,3,0,2,56,6,0,4,28,12,0,8,14,24,0,16,7,48,0,32,3,224,0,32,3,192,0,16,7,192,0,8,14,32,0,4,28,16,0,2,56,8,0,1,112,4,0,0,224,2,0,0,192,2,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,96,0,0,0,240,0,0,1,136,0,0,3,4,0,0,6,2,0,0,204,1,0,1,56,0,128,2,24,0,192,4,28,0,128,8,14,1,0,16,7,2,0,32,3,132,0,32,3,200,0,16,7,240,0,8,14,112,0,4,28,16,0,2,56,8,0,1,112,4,0,0,224,2,0,0,192,2,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,0,0,0,28,0,0,0,34,0,0,192,65,0,1,32,128,128,2,17,0,64,4,14,0,32,8,6,0,16,16,7,0,48,32,3,128,96,32,3,192,192,16,7,225,128,8,14,115,0,4,28,62,0,2,56,28,0,1,112,4,0,0,224,2,0,0,192,2,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,192,13,0,1,32,24,128,2,16,48,64,4,8,96,32,8,4,192,16,16,3,128,8,32,3,128,12,32,3,192,24,16,7,224,48,8,14,112,96,4,28,56,192,2,56,29,128,1,112,15,0,0,224,7,0,0,192,2,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,3,0,1,32,4,128,2,16,8,64,4,8,16,32,8,4,32,16,16,2,64,8,32,1,128,4,32,3,192,4,16,7,224,8,8,14,112,16,4,28,56,32,2,56,28,64,1,112,14,128,0,224,7,0,0,192,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,3,0,1,32,4,128,2,16,8,64,4,8,16,32,8,4,32,16,16,2,64,8,32,1,128,4,32,3,192,4,16,7,224,8,8,14,112,16,4,28,56,32,2,56,28,64,1,112,14,128,0,224,7,0,0,192,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,3,0,1,32,4,128,2,16,8,64,4,8,16,32,8,4,32,16,16,2,64,8,32,1,128,4,32,3,192,4,16,7,224,8,8,14,112,16,4,28,56,32,2,56,28,64,1,112,14,128,0,224,7,0,0,192,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,96,0,0,0,176,3,0,1,24,4,128,2,12,8,64,4,6,16,32,8,3,32,16,16,1,192,8,48,1,192,4,24,3,192,4,12,7,224,8,6,14,112,16,3,28,56,32,1,184,28,64,0,240,14,128,0,96,7,0,0,64,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,56,0,0,0,68,0,0,0,130,3,0,1,1,4,128,2,0,136,64,4,0,112,32,12,0,96,16,4,0,224,8,2,1,192,4,1,3,192,4,0,135,224,8,0,78,112,16,0,60,56,32,0,56,28,64,0,32,14,128,0,64,7,0,0,64,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,14,0,0,0,17,0,0,0,32,128,0,0,64,64,0,0,128,35,0,1,0,28,128,3,0,24,64,1,0,56,32,0,128,112,16,0,64,224,8,0,33,192,4,0,19,192,4,0,15,224,8,0,14,112,16,0,8,56,32,0,16,28,64,0,32,14,128,0,64,7,0,0,64,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,0,0,0,3,128,0,0,4,192,0,0,8,96,0,0,16,48,0,0,32,24,0,0,64,12,0,0,128,7,0,0,192,14,128,0,96,28,64,0,48,56,32,0,24,112,16,0,12,224,8,0,7,192,4,0,3,192,4,0,2,224,8,0,4,112,16,0,8,56,32,0,16,28,64,0,32,14,128,0,64,7,0,0,64,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,64,3,0,0,64,7,0,0,32,14,128,0,16,28,64,0,8,56,32,0,4,112,16,0,2,224,8,0,1,192,4,0,1,192,4,0,2,224,8,0,4,112,16,0,8,56,32,0,16,28,64,0,32,14,128,0,64,7,0,0,64,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,64,3,0,0,64,7,0,0,32,14,128,0,16,28,64,0,8,56,32,0,4,112,16,0,2,224,8,0,1,192,4,0,1,192,4,0,2,224,8,0,4,112,16,0,8,56,32,0,16,28,64,0,32,14,128,0,64,7,0,0,64,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,64,3,0,0,64,7,0,0,32,14,128,0,16,28,64,0,8,56,32,0,4,112,16,0,2,224,8,0,1,192,4,0,1,192,4,0,2,224,8,0,4,112,16,0,8,56,32,0,16,28,64,0,32,14,128,0,64,7,0,0,64,3,0,0,32,4,0,0,16,8,0,0,8,16,0,0,4,32,0,0,2,64,0,0,1,128,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,64,3,0,0,64,7,0,0,32,14,128,0,16,28,64,0,8,56,32,0,4,112,16,0,3,224,8,0,3,192,4,0,7,192,4,0,12,224,8,0,24,112,16,0,48,56,32,0,96,28,64,0,192,14,128,0,128,7,0,0,64,14,0,0,32,24,0,0,16,48,0,0,8,96,0,0,4,192,0,0,3,128,0,0,1,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,64,3,0,0,64,7,0,0,32,14,128,0,16,28,64,0,8,56,32,0,14,112,16,0,15,224,8,0,19,192,4,0,33,192,4,0,64,224,8,0,128,112,16,1,0,56,32,3,0,28,64,1,0,28,128,0,128,51,0,0,64,96,0,0,32,192,0,0,17,128,0,0,15,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,64,3,0,0,64,7,0,0,48,14,128,0,56,28,64,0,124,56,32,0,206,112,16,1,135,224,8,3,3,192,4,6,1,192,4,12,0,224,8,12,0,112,16,4,0,112,32,2,0,136,64,1,1,4,128,0,130,3,0,0,68,0,0,0,56,0,0,0,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,128,0,0,2,64,0,0,4,32,0,0,8,16,0,0,16,8,0,0,32,4,0,0,64,3,0,0,96,7,0,0,240,14,128,1,184,28,64,3,28,56,32,6,14,112,16,12,7,224,8,24,3,192,4,48,1,192,4,16,1,192,8,8,3,32,16,4,6,16,32,2,12,8,64,1,24,4,128,0,176,3,0,0,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


const unsigned char PROGMEM people_icon_32[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,224,7,192,7,240,15,224,6,48,28,96,
  6,24,24,96,6,56,28,96,7,48,12,224,3,241,143,192,0,199,227,0,0,14,112,0,
  15,236,55,240,31,236,55,248,24,12,48,24,24,7,224,24,24,3,192,24,28,0,0,56,
  15,192,3,240,3,159,249,192,0,63,252,0,0,48,12,0,0,48,12,0,0,48,12,0,
  0,28,56,0,0,15,240,0,0,3,192,0,0,0,0,0,0,0,0,0,0,0,0,0
};

// ==================================================
// Ultrasonic distance reading
// ==================================================
long readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000); 
  if (duration == 0) return 999; 
  long distance = duration * 0.0343 / 2;
  return distance;
}

bool isTriggered(int trigPin, int echoPin) {
  long d = readDistanceCM(trigPin, echoPin);
  return (d > 0 && d < sensorSensitivity);
}


void updateSensors() {
  bool s1now = isTriggered(US1_TRIG, US1_ECHO);
  bool s2now = isTriggered(US2_TRIG, US2_ECHO);

  // rising edge detection
  if (s1now && !s1Last) {
    if (seqState == SEQ_NONE) {
      seqState = SEQ_S1_FIRST;
      seqTime = millis();
    } else if (seqState == SEQ_S2_FIRST) {
      
      persons--;
      if (persons < 0) persons = 0;
      seqState = SEQ_NONE;
      personsZeroTime = millis(); 
    }
  }

  if (s2now && !s2Last) {
    if (seqState == SEQ_NONE) {
      seqState = SEQ_S2_FIRST;
      seqTime = millis();
    } else if (seqState == SEQ_S1_FIRST) {
      
      persons++;
      seqState = SEQ_NONE;
    }
  }

  
  if (seqState != SEQ_NONE && (millis() - seqTime > SEQ_TIMEOUT)) {
    seqState = SEQ_NONE;
  }

  s1Last = s1now;
  s2Last = s2now;

  
  pirRawState = (digitalRead(PIR_PIN) == HIGH);
  if (pirRawState) {
    lastMotionTime = millis();
  }
  
  bool motion = pirRawState || (millis() - lastMotionTime < pirHoldTime);

  if (motion && persons > 0) {
    lightOn = true;
    personsZeroTime = 0; 
  } else if (persons == 0) {
    
    if (personsZeroTime == 0) {
      personsZeroTime = millis(); 
    }
    
    if (millis() - personsZeroTime > LIGHT_OFF_DELAY) {
      lightOn = false; 
    }
    
  }
  

  digitalWrite(RELAY1_PIN, lightOn ? HIGH : LOW);

 
  digitalWrite(RELAY2_PIN, lightOn ? HIGH : LOW);
}


void beep() {
  tone(BUZZER_PIN, 3000, 300); 
}


void drawSpinner(int cx, int cy, int radius, int frame, int totalFrames) {
  for (int i = 0; i < totalFrames; i++) {
    float angle = (2 * PI * i / totalFrames);
    int dotX = cx + radius * cos(angle);
    int dotY = cy + radius * sin(angle);

    int diff = (i - frame + totalFrames) % totalFrames;
    int dotSize = (diff == 0) ? 2 : (diff == 1 || diff == totalFrames - 1) ? 1 : 0;

    if (dotSize > 0) {
      display.fillCircle(dotX, dotY, dotSize, SSD1306_WHITE);
    }
  }
}


void drawBootScreen() {
  unsigned long animStart = millis();
  int bootFrame = 0;

  while (millis() - animStart < 2000) {
    display.clearDisplay();
    display.drawBitmap(48, 16, bootFrames[bootFrame], ANIM_FRAME_WIDTH, ANIM_FRAME_HEIGHT, SSD1306_WHITE);

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(28, 52);
    display.print(F("Loading..."));

    display.display();
    beep();

    bootFrame = (bootFrame + 1) % ANIM_FRAME_COUNT;
    delay(ANIM_FRAME_DELAY);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 28);
  display.print(F("System Started"));
  display.display();
  delay(1000);
}


void drawHomeScreen() {
  display.clearDisplay();

  int iconSize = 32;
  int iconX = (SCREEN_WIDTH - iconSize) / 2;
  int iconY = 6;

  int cx = iconX + iconSize / 2;
  int cy = iconY + iconSize / 2;
  drawSpinner(cx, cy, 22, spinnerFrame, 8);

  display.drawBitmap(iconX, iconY, people_icon_32, 32, 32, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  char buf[20];
  sprintf(buf, "Persons: %d", persons);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 44);
  display.print(buf);

  display.setCursor(2, 56);
  display.print(F("Light: "));
  display.print(lightOn ? F("ON") : F("OFF"));

  display.setCursor(90, 56);
  display.print(F("[MENU]"));

  display.display();
}


void drawArrows() {
  int midY = SCREEN_HEIGHT / 2;
  display.fillTriangle(14, midY - 8, 14, midY + 8, 4, midY, SSD1306_WHITE);
  display.fillTriangle(SCREEN_WIDTH - 14, midY - 8, SCREEN_WIDTH - 14, midY + 8, SCREEN_WIDTH - 4, midY, SSD1306_WHITE);
}

void drawHomeIcon(int x, int y, int s) {
  display.fillTriangle(x + s/2, y, x, y + s/2, x + s, y + s/2, SSD1306_WHITE);
  display.fillRect(x + s/8, y + s/2, (s*6)/8, s/2, SSD1306_WHITE);
  display.fillRect(x + (3*s)/8, y + (3*s)/4, s/4, s/4, SSD1306_BLACK);
}

void drawInfoIcon(int x, int y, int s) {
  display.drawCircle(x + s/2, y + s/2, s/2 - 1, SSD1306_WHITE);
  display.drawCircle(x + s/2, y + s/2, s/2 - 2, SSD1306_WHITE);
  display.fillCircle(x + s/2, y + s/2 - s/4, s/12 + 1, SSD1306_WHITE);
  display.fillRect(x + s/2 - s/12, y + s/2 - s/16, s/6, s/3, SSD1306_WHITE);
}

void drawSettingsIcon(int x, int y, int s) {
  int cx = x + s / 2;
  int cy = y + s / 2;
  int outerR = s / 2 - 2;
  int innerR = s / 4;
  int toothLen = s / 6;
  int toothW = s / 8;

  for (int i = 0; i < 8; i++) {
    float angle = i * 45.0 * PI / 180.0;
    int tx = cx + (outerR + toothLen / 2) * cos(angle);
    int ty = cy + (outerR + toothLen / 2) * sin(angle);
    display.fillCircle(tx, ty, toothW / 2, SSD1306_WHITE);
  }
  display.fillCircle(cx, cy, outerR, SSD1306_WHITE);
  display.fillCircle(cx, cy, innerR, SSD1306_BLACK);
}

void drawRestartIcon(int x, int y, int s) {
  int cx = x + s / 2;
  int cy = y + s / 2;
  int r = s / 2 - 3;

  
  int startDeg = 30;
  int endDeg   = 320;
  int prevX = 0, prevY = 0;
  bool first = true;
  for (int deg = startDeg; deg <= endDeg; deg += 6) {
    float rad = deg * PI / 180.0;
    int px = cx + r * cos(rad);
    int py = cy + r * sin(rad);
    if (!first) {
      display.drawLine(prevX, prevY, px, py, SSD1306_WHITE);
    }
    prevX = px; prevY = py;
    first = false;
  }

  
  float endRad = endDeg * PI / 180.0;
  int tipX = cx + r * cos(endRad);
  int tipY = cy + r * sin(endRad);
  display.fillTriangle(tipX, tipY - 4, tipX - 5, tipY + 2, tipX + 4, tipY + 4, SSD1306_WHITE);
}

void drawIconByIndex(int index, int x, int y, int s) {
  switch (index) {
    case 0: drawHomeIcon(x, y, s); break;
    case 1: drawInfoIcon(x, y, s); break;
    case 2: drawSettingsIcon(x, y, s); break;
    case 3: drawRestartIcon(x, y, s); break;
  }
}

void drawMenuScreen() {
  display.clearDisplay();
  drawArrows();

  int iconSize = 32;
  int x = (SCREEN_WIDTH - iconSize) / 2;
  int y = (SCREEN_HEIGHT - iconSize) / 2 - 4;
  drawIconByIndex(menuIndex, x, y, iconSize);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(iconNames[menuIndex], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT - 10);
  display.print(iconNames[menuIndex]);

  display.display();
}

void drawPageScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(5, 5);
  display.print(iconNames[menuIndex]);
  display.print(F(" Page"));
  display.drawLine(0, 15, SCREEN_WIDTH, 15, SSD1306_WHITE);

  if (menuIndex == 1) {
    display.setCursor(5, 25);
    display.print(F("\tSmart Lecture Hall\n\nEnergy Saving System"));
  } else if (menuIndex == 2) {
    for (int i = 0; i < NUM_SETTINGS; i++) {
      int lineY = 22 + i * 12;
      bool selected = (i == settingIndex);

      if (selected) {
        display.fillRect(0, lineY - 1, SCREEN_WIDTH, 11, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }

      display.setCursor(5, lineY);
      if (i == 0) {
        char buf[24];
        sprintf(buf, "Sensitivity: %dcm", sensorSensitivity);
        display.print(buf);
      } else {
        display.print(settingNames[i]);
      }
    }
    display.setTextColor(SSD1306_WHITE);
  }

  display.setCursor(5, SCREEN_HEIGHT - 10);
  display.print(F("[MENU]=Back"));
  display.display();
}


void drawSettingDetailScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(5, 5);
  display.print(settingNames[openSettingIndex]);
  display.drawLine(0, 15, SCREEN_WIDTH, 15, SSD1306_WHITE);

  if (openSettingIndex == 0) {
    
    char buf[16];
    sprintf(buf, "%d cm", sensorSensitivity);

    display.setTextSize(2);
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 20);
    display.print(buf);

    display.setTextSize(1);
    display.setCursor(12, 40);
    display.print(F("< Left / Right >"));
  } else if (openSettingIndex == 1) {
    
    display.setTextSize(1);
    display.setCursor(5, 20);
    display.print(F("Hold: "));
    display.print(pirHoldTime);
    display.print(F(" ms"));

    display.setCursor(5, 32);
    display.print(F("Motion: "));
    display.print(pirRawState ? F("YES") : F("NO"));

    display.setCursor(5, 44);
    display.print(F("< Left / Right >"));
  }

  display.setCursor(5, SCREEN_HEIGHT - 10);
  display.print(F("[MENU]=Back"));
  display.display();
}


void drawConfirmRestartScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(5, 5);
  display.print(F("Restart System"));
  display.drawLine(0, 15, SCREEN_WIDTH, 15, SSD1306_WHITE);

  display.setTextSize(1);
  int16_t x1, y1; uint16_t w, h;
  const char* msg = "Are you sure?";
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 28);
  display.print(msg);

  display.setCursor(5, SCREEN_HEIGHT - 10);
  display.print(F("[OK]=Yes"));

  display.setCursor(75, SCREEN_HEIGHT - 10);
  display.print(F("[MENU]=No"));

  display.display();
}


void setup() {
  Serial.begin(9600);

  pinMode(US1_TRIG, OUTPUT);
  pinMode(US1_ECHO, INPUT);
  pinMode(US2_TRIG, OUTPUT);
  pinMode(US2_ECHO, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  pinMode(BTN_MENU, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  drawBootScreen();          
  currentState = STATE_HOME; 
  drawHomeScreen();
}


void loop() {
  
  updateSensors();

  
  bool menuState  = digitalRead(BTN_MENU);
  bool leftState  = digitalRead(BTN_LEFT);
  bool rightState = digitalRead(BTN_RIGHT);
  bool okState    = digitalRead(BTN_OK);
  unsigned long now = millis();

  if (now - lastDebounceTime > debounceDelay) {

    
    if (menuState == LOW && lastMenu == HIGH) {
      beep();
      if (currentState == STATE_HOME) {
        currentState = STATE_MENU;
        drawMenuScreen();
      } else if (currentState == STATE_MENU) {
        currentState = STATE_HOME;
        drawHomeScreen();
      } else if (currentState == STATE_PAGE) {
        currentState = STATE_MENU;
        drawMenuScreen();
      } else if (currentState == STATE_SETTING_DETAIL) {
        
        currentState = STATE_PAGE;
        drawPageScreen();
      } else if (currentState == STATE_CONFIRM_RESTART) {
        
        currentState = STATE_MENU;
        drawMenuScreen();
      }
      lastDebounceTime = now;
    }

    // LEFT
    if (leftState == LOW && lastLeft == HIGH) {
      beep();
      if (currentState == STATE_MENU) {
        menuIndex--;
        if (menuIndex < 0) menuIndex = NUM_ICONS - 1;
        drawMenuScreen();
      } else if (currentState == STATE_PAGE && menuIndex == 2) {
        settingIndex--;
        if (settingIndex < 0) settingIndex = NUM_SETTINGS - 1;
        drawPageScreen();
      } else if (currentState == STATE_SETTING_DETAIL && openSettingIndex == 0) {
        sensorSensitivity -= SENS_STEP;
        if (sensorSensitivity < SENS_MIN) sensorSensitivity = SENS_MIN;
        drawSettingDetailScreen();
      } else if (currentState == STATE_SETTING_DETAIL && openSettingIndex == 1) {
        if (pirHoldTime >= PIR_HOLD_MIN + PIR_HOLD_STEP) pirHoldTime -= PIR_HOLD_STEP;
        else pirHoldTime = PIR_HOLD_MIN;
        drawSettingDetailScreen();
      }
      lastDebounceTime = now;
    }

    // RIGHT
    if (rightState == LOW && lastRight == HIGH) {
      beep();
      if (currentState == STATE_MENU) {
        menuIndex++;
        if (menuIndex >= NUM_ICONS) menuIndex = 0;
        drawMenuScreen();
      } else if (currentState == STATE_PAGE && menuIndex == 2) {
        settingIndex++;
        if (settingIndex >= NUM_SETTINGS) settingIndex = 0;
        drawPageScreen();
      } else if (currentState == STATE_SETTING_DETAIL && openSettingIndex == 0) {
        sensorSensitivity += SENS_STEP;
        if (sensorSensitivity > SENS_MAX) sensorSensitivity = SENS_MAX;
        drawSettingDetailScreen();
      } else if (currentState == STATE_SETTING_DETAIL && openSettingIndex == 1) {
        pirHoldTime += PIR_HOLD_STEP;
        if (pirHoldTime > PIR_HOLD_MAX) pirHoldTime = PIR_HOLD_MAX;
        drawSettingDetailScreen();
      }
      lastDebounceTime = now;
    }

    // OK
    if (okState == LOW && lastOk == HIGH) {
      beep();
      if (currentState == STATE_MENU) {
        if (menuIndex == 0) {
          
          currentState = STATE_HOME;
          drawHomeScreen();
        } else if (menuIndex == 3) {
          
          currentState = STATE_CONFIRM_RESTART;
          drawConfirmRestartScreen();
        } else {
          currentState = STATE_PAGE;
          settingIndex = 0;
          drawPageScreen();
        }
      } else if (currentState == STATE_PAGE && menuIndex == 2) {
        
        openSettingIndex = settingIndex;
        currentState = STATE_SETTING_DETAIL;
        drawSettingDetailScreen();
      } else if (currentState == STATE_CONFIRM_RESTART) {
        
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(20, 28);
        display.print(F("Restarting..."));
        display.display();
        delay(500);
        resetFunc(); 
      }
      lastDebounceTime = now;
    }
  }

  lastMenu = menuState;
  lastLeft = leftState;
  lastRight = rightState;
  lastOk = okState;

 
  if (currentState == STATE_HOME) {
    if (now - lastSpinnerUpdate > spinnerInterval) {
      spinnerFrame = (spinnerFrame + 1) % 8;
      lastSpinnerUpdate = now;
      drawHomeScreen();
    }
  }

  
  static unsigned long lastPirDisplayUpdate = 0;
  if (currentState == STATE_SETTING_DETAIL && openSettingIndex == 1) {
    if (now - lastPirDisplayUpdate > 200) {
      lastPirDisplayUpdate = now;
      drawSettingDetailScreen();
    }
  }
}