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
//#define UTC_OFFSET 19800
#define UTC_OFFSET_DST 0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //< See datasheet for Address; 0x3D for 128x64,

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
int UTC_OFFSET = 0;
int UTC_hours=0;
int UTC_minutes=0;

bool enable_alarm[] = {false, false};
int n_alarms = 2;
int alarm_hours[] = {0, 0};
int alarm_minutes[] = {1, 10};
bool alarm_triggered[] = {false, false};

unsigned long timeNow = 0;
unsigned long timeLast = 0;

int current_mode = 0;
int max_modes = 4;
String options[] = {"1 - show Alarm", "2 - Set Alarm ", "3 - Set UTC", "4 - Remove Alarm"};

void print_line(String text, int text_size, int row, int column){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column,row);
  display.println(text);

  display.display();
}

void show_alarm(){
  if(enable_alarm[0] == true){
    display.clearDisplay();
    print_line("Alarm 1 : ",1,0,0);
    print_line(String(alarm_hours[0]),1,8,0);
    print_line(" : ",1,8,16);
    print_line(String(alarm_minutes[0]),1,8,20);
    delay(3000);
  }
  if(enable_alarm[1] == true){
    print_line("Alarm 2 : ",1,16,0);
    print_line(String(alarm_hours[1]),1,24,0);
    print_line(" : ",1,24,24);
    print_line(String(alarm_minutes[1]),1,24,24);
    delay(3000);
    
  }
} 


void check_temp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  bool all_good = true;
  if (data.temperature > 32){
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP HIGH", 1, 40, 0);
  }
  else if(data.temperature < 24){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("TEMP LOW", 1, 40, 0);
  }
  if(data.humidity > 80){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("HUMD HIGH", 1, 50, 0);
  }
  else if(data.humidity < 65){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("HUMP LOW", 1, 50, 0);
  }
  if (all_good){
    digitalWrite(LED_2, LOW);
  }
}



// function to automatically update the current time
void update_time() {
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

}

}

void delete_alarm(){
  display.clearDisplay();
  print_line("UP : del Alarm 2",1,0,0);
  print_line("DOWN : del Alarm 1",1,8,0);
  delay(3000);
  int pressed = wait_for_button_press();
  delay(200);
  if(pressed == UP){
    enable_alarm[1] = false;
    display.clearDisplay();
    print_line("Alarm 2 deleted",1,0,0);
    delay(3000);
  }
  else if (pressed == DOWN){
    enable_alarm[0]= false;
    display.clearDisplay();
    print_line("Alarm 1 deleted",1,0,0);
    delay(3000);
  }
}

int set_UTC_time(){
  int temp_UTC_hour =UTC_hours;
  while (true){
    display.clearDisplay();
    print_line("UTC hour:" + String(temp_UTC_hour),0,0,2);

    int pressed = wait_for_button_press();
    delay(1000);
    if (pressed == UP){
      delay(200);
      temp_UTC_hour +=1;
      temp_UTC_hour= temp_UTC_hour % 24;
    }
    else if (pressed == DOWN){
        delay(200);
        temp_UTC_hour -= 1;
        if (temp_UTC_hour<0){
           temp_UTC_hour=23;
        }
    }
    else if (pressed == OK){
        delay(200);
        UTC_hours = temp_UTC_hour;
        break;
    }
    else if (pressed == CANCEL){
        delay(200);
        break;
    }
  }
  int temp_UTC_minute = UTC_minutes;
  while (true){
    display.clearDisplay();
    print_line("UTC minute:" + String(temp_UTC_minute), 0, 0,2);

    int pressed = wait_for_button_press();
    if (pressed == UP){
      delay(200);
      temp_UTC_minute +=1;
      temp_UTC_minute = temp_UTC_minute % 60;
    }
    else if (pressed == DOWN){
      delay(200);
      temp_UTC_minute -= 1;
      if (temp_UTC_minute<0){
           temp_UTC_minute=59;
      }
    }
    else if (pressed == OK){
      delay(200);
      UTC_minutes = temp_UTC_minute;
      break;
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  
  display.clearDisplay();
  print_line("UTC : ", 1, 0, 0);
  print_line(String(UTC_hours), 1, 0, 24);
  print_line(":", 1, 0, 33);
  print_line(String(UTC_minutes), 1, 0, 38);
  delay(3000);
  UTC_OFFSET=UTC_minutes*60 + UTC_hours*60*60 ;
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  return  UTC_minutes*60 + UTC_hours*60*60 ;
}

void set_alarm(int alarm){
  alarm_triggered[alarm] = false;
  enable_alarm[alarm]=true;
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
      alarm_minutes[alarm] = temp_minute;
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
  if (mode == 0){
    show_alarm();
  }
  else if (mode == 1){
    display.clearDisplay();
    print_line("UP : Alarm 1", 1, 0, 0);
    print_line("DOWN : Alarm 2", 1, 8, 0);
    int pressed= wait_for_button_press();
    display.clearDisplay();
    if (pressed == UP){
      print_line("Alarm 1 selected", 1, 0, 0);
      int alarm=0;
      set_alarm(alarm);
    }
    if (pressed == DOWN){
      print_line("Alarm 2 selected", 1, 0, 0);
      int alarm=1;
      set_alarm(alarm);
    }
    
  }
  else if (mode == 2){
    set_UTC_time();
  }
  else if (mode == 3){
    delete_alarm();
  }
}

void go_to_menu(void){
  while (digitalRead(CANCEL)==HIGH){
    display.clearDisplay();
    print_line(options[current_mode],1,0,0);

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
      display.clearDisplay();
      print_line("Selected mode :" + options[current_mode],1,0,0);
      delay(3000);
      display.clearDisplay();
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

void ring_alarm(){
  display.clearDisplay();
  print_line("Medicine Time",2,0,0);
  
  digitalWrite(LED_1, HIGH);

  while (digitalRead(CANCEL)==HIGH){
    for (int i=0; i< n_notes; i++){
      if (digitalRead(CANCEL)==LOW){
        digitalWrite(LED_1, LOW);
        delay(200);
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(200);
      //display.clearDisplay();
      print_line("Push DOWN TO SNOOZE",1,32,0);
      delay(200);
      if(digitalRead(DOWN)==LOW){
        int snooze_time= 5*60*1000;
        delay(snooze_time);
    }
  }
}}

void update_time_with_check_alarm(){
  display.clearDisplay();
  update_time();
  print_time_now();

  if (enable_alarm[0] == true || enable_alarm[1] == true){
    for (int i=0; i<n_alarms; i++){
      if (alarm_triggered[i] ==false & hours == alarm_hours[i] & minutes==alarm_minutes[i]){
        ring_alarm();
        alarm_triggered[i] = true;
        enable_alarm[i]= false;
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
for ( ;; ); // Don't proceed, loop forever
}


display.display();
delay(2000); // Pause for 2 seconds

// Clear the buffer
display.clearDisplay();
print_line("Welcome to Medibox", 2, 0, 0); // (String, text_size, cursor_row, cursor_column)
delay(3000);

WiFi.begin("Wokwi-GUEST");
while (WiFi.status() != WL_CONNECTED) {
display.clearDisplay();
//print_line("Not connected", 2, 0, 0);
delay(250);
}
display.clearDisplay();
print_line("Connecting to WiFi", 2, 0, 0); // (String, text_size, cursor_row, cursor_column)
 
delay(1000);
display.clearDisplay();
print_line("Connected to WiFi", 2, 0, 0);

delay(1000);
display.clearDisplay();
print_line("Press CANCEL Button for Menu", 1, 0, 0);
delay(1000);

//UTC_OFFSET=set_UTC_time();
configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
}


void loop(){
// put your main code here, to run repeatedly:
update_time_with_check_alarm();
if (digitalRead(CANCEL) == LOW) {
Serial.println("Menu");
delay(200);
go_to_menu();
}
configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
check_temp();
}