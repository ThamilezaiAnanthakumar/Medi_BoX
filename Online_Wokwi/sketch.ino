// Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <time.h>
#include <Arduino.h>


// Defnitions
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 0
#define UTC_OFFSET_DST 0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET-1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64,

#define BUZZER 18
#define LED_1 15
#define LED_2 2
#define CANCEL 34
#define UP 35
#define DOWN 32
#define OK 33
#define DHT 12

// object Declarations
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

// Variables
int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

bool alarm_enabled = false;
int n_alarms = 2;
int alarm_hours[] = {0, 0};
int alarm_minutes[] = {1, 10};
bool alarm_triggered[] = {false, false};

unsigned long timeNow = 0;
unsigned long timeLast = 0;

int current_mode = 0;
int max_modes = 4;
String options[] = {"1 - Set Time", "2 - Set Alarm 1", "3 - Set Alarm 2", "4 - Remove Alarm"};

void print_line(String text, int text_size, int row, int column){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column,row);
  display.println(text);

  display.display();
}

void check_temp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  bool all_good = true;
  if (data.temperature > 35){
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP HIGH", 1, 40, 0);
  }
  else if(data.temperature < 25){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("TEMP LOW", 1, 40, 0);
  }
  if(data.humidity > 85){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("HUMD HIGH", 1, 50, 0);
  }
  else if(data.humidity < 35){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("HUMP LOW", 1, 50, 0);
  }
  if (all_good){
    digitalWrite(LED_2, LOW);
  }
}



// function to automatically update the current time
void update_time(void) {
struct tm timeinfo;
getLocalTime(&timeinfo);

char day_str[8];
char hour_str[8];
char min_str[8];
char sec_str[8];
strftime(day_str,8, "%d", &timeinfo);
strftime(sec_str,8, "%S", &timeinfo);
strftime(hour_str,8, "%H", &timeinfo);
strftime(min_str,8, "%M", &timeinfo);

hours = atoi(hour_str);
minutes = atoi(min_str);
days = atoi(day_str);
seconds = atoi(sec_str);
}

int wait_for_button_press() {
while (true) {
if (digitalRead(UP) == LOW) {
delay(200);
return UP;

}

else if (digitalRead(DOWN) == LOW) {
delay(200);
return DOWN;

}

else if (digitalRead(CANCEL) == LOW) {
delay(200);
return CANCEL;
}

else if (digitalRead(OK) == LOW) {
delay(200);
return OK;

}

update_time();

}

}

void set_time(){
  int temp_hour =hours;
  while (true){
    display.clearDisplay();
    print_line("Enter hour:" + String(temp_hour),0,0,2);

    int pressed = wait_for_button_press();
    if (pressed == UP){
      delay(200);
      temp_hour +=1;
      temp_hour= temp_hour % 24;
    }
    else if (pressed == DOWN){
        delay(200);
        temp_hour -= 1;
        if (temp_hour<0){
           temp_hour=23;
        }
    }
    else if (pressed == OK){
        delay(200);
        hours = temp_hour;
        break;
    }
    else if (pressed == CANCEL){
        delay(200);
        break;
    }
  }
  int temp_minute = minutes;
  while (true){
    display.clearDisplay();
    print_line("Enter minute:" + String(temp_minute), 0, 0,2);

    int pressed = wait_for_button_press();
    if (pressed == UP){
      delay(200);
      temp_minute +=1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == DOWN){
      delay(200);
      temp_minute -= 1;
      if (temp_minute<0){
           temp_minute=59;
      }
    }
    else if (pressed == OK){
      delay(200);
      minutes = temp_minute;
      break;
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Time is set", 0, 0, 2);
  delay(1000);
}

void set_alarm(int alarm){
  int temp_hour = alarm_hours[alarm];
  while (true){
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_hour +=1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == DOWN){
      delay(200);
      temp_hour -=1;
      if (temp_hour < 0) {
        temp_hour =23;
      }
    }
    else if (pressed == OK){
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  int temp_minute = alarm_minutes[alarm];
  while (true){
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute),0,0,2);
    int pressed = wait_for_button_press();
    if (pressed == UP){
      delay(200);
      temp_minute +=1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == DOWN){
      delay(200);
      temp_minute -= 1;
      if (temp_minute<0){
           temp_minute=59;
      }
    }
    else if (pressed == OK){
      delay(200);
      minutes = temp_minute;
      break;
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Time is set", 0, 0, 2);
  delay(1000);
}

void run_mode(int mode){
  if (mode ==0){
    set_time();
  }
  else if (mode == 1 || mode ==2){
    set_alarm(mode -1);
  }
  else if (mode == 3){
    alarm_enabled = false;
  }
}

void go_to_menu(void){
  while (digitalRead(CANCEL)==HIGH){
    display.clearDisplay();
    print_line(options[current_mode],2,0,0);

    int pressed= wait_for_button_press();

    if (pressed==UP){
      current_mode +=1;
      current_mode %= max_modes;
      delay(200);
    }
    else if (pressed == DOWN){
      current_mode -= 1;
      if (current_mode < 0){
        current_mode = max_modes -1;
      }
      delay(200);
    }
    else if (pressed == OK){
      Serial.println(current_mode);
      delay(200);
      run_mode(current_mode);
    }
  }
}








// function to wait for button press in the menu
void print_time_now(void){
  print_line(String(days),2,0,0);
  print_line(":",2,0,30);
  print_line(String(hours),2,0,30);
  print_line(":",2,0,30);
  print_line(String(minutes),2,0,60);
  print_line(":",2,0,30);
  print_line(String(seconds),2,0,90);
}

void update_time_with_check_alarm(){
  display.clearDisplay();
  update_time();
  print_time_now();
  
  if (alarm_enabled){
    for (int i=0; i<n_alarms; i++){
      if (alarm_triggered[i] ==false && hours == alarm_hours[i] && minutes==alarm_minutes[i]){
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}



void ring_alarm(){
  display.clearDisplay();
  print_line("Medicine Time",2,0,0);
  
  digitalWrite(LED_1, HIGH);

  while (digitalRead(CANCEL)==HIGH){
    for (int i=0; i< n_notes; i++){
      if (digitalRead(CANCEL)==LOW){
        delay(200);
        break;
      }
    }
  }
}
void setup() {
Serial.begin(9600);

pinMode(BUZZER, OUTPUT);
pinMode(LED_1, OUTPUT);
pinMode(LED_2, OUTPUT);
pinMode(CANCEL, INPUT);
pinMode(UP, INPUT);
pinMode(DOWN, INPUT);
pinMode(OK, INPUT);

dhtSensor.setup(DHT, DHTesp :: DHT22);

// put your setup code here, to run once:
if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
Serial.println(F("SSD1306 allocation failed"));
}
for ( ;; ); // Don't proceed, loop forever

display.display();
delay(2000); // Pause for 2 seconds

// Clear the buffer
display.clearDisplay();
print_line("Welcome to Medibox", 2, 0, 0); // (String, text_size, cursor_row, cursor_column)
delay(3000);

WiFi.begin("Wokwi-GUEST", "", 6);
while (WiFi.status() != WL_CONNECTED) {
delay(250);
}
display.clearDisplay();
print_line("Connecting to WiFi", 2, 0, 0); // (String, text_size, cursor_row, cursor_column)

display.clearDisplay();
print_line("Connected to WiFi", 2, 0, 0);

configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
}


void loop(){
// put your main code here, to run repeatedly:
update_time_with_check_alarm();
if (digitalRead(CANCEL) == LOW) {
delay(1000);
Serial.println("Menu");
go_to_menu();
}
check_temp();
}