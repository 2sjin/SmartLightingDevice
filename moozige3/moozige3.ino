#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <avr/power.h>

#define NUMPIXELS 52  // Neopixel LED 소자 수
#define GROUPS 4      // Neopixel LED 그룹 수
#define BRIGHT_MIN 15
#define BRIGHT_MAX 235
#define PIN_NEOPIXEL 7      // NeolPixel LED 디지털 단자 번호
#define PIN_PIR 8       // 인체 감지 센서 디지털 단자 번호

// 네오픽셀 라이브러리의 함수를 strip 이라는 이름으로 사용
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
SoftwareSerial btSerial(2,3);

// 전역변수 선언
unsigned long timeNext = millis();   // millis 사용을 위한 변수 1
unsigned long timeNextCds = millis(); // millis 사용을 위한 변수 2
char sMode;
byte led[4]={0,};
int Custom;
int Red, Green, Blue;
bool AutoBright, OnPIR, PIR_Power, PIR_Switch, sShow;
int sBright;

// 네오픽셀 클래스
class NeoPixel {
  int num, red, green, blue;
public:
  NeoPixel() {strip.begin();}
  void setNumber(int num) {this->num = num;}
  int getNumber() {num = this->num; return num;}
  void setColor(int r, int g, int b);
  void modeRainbow();
  void ledShow() { setColor(this->red, this->green, this->blue); strip.show(); }
};

void NeoPixel::setColor(int r, int g, int b) {
  for(int i = 0; i < NUMPIXELS / GROUPS; i++)
    strip.setPixelColor(this->num*(NUMPIXELS / GROUPS)+i, r, g, b);
  this->red = r;
  this->green = g;
  this->blue = b;
}

void NeoPixel::modeRainbow() {
  
  // 조명 새로고침
  strip.show();
  
  // RGB 값의 변경
  if(this->red == 255 && this->green < 255 && this->blue == 0) this->green += 16;
  else if(this->red > 0 && this->green == 255 && this->blue == 0) this->red -= 16;
  else if(this->red == 0 && this->green == 255 && this->blue < 255) this->blue += 16;
  else if(this->red == 0 && this->green > 0 && this->blue == 255) this->green -= 16;
  else if(this->red < 255 && this->green == 0 && this->blue == 255) this->red += 16;
  else if(this->red == 255 && this->green == 0 && this->blue > 0) this->blue -= 16;

  // RGB 값 범위 초과 시 0 또는 255로 보정
  if(this->red > 255) this->red = 255;
  else if(this->red < 0) this->red = 0;
  if(this->green > 255) this->green = 255;
  else if(this->green < 0) this->green = 0;  
  if(this->blue > 255) this->blue = 255;
  else if(this->blue < 0) this->blue = 0;

  // RGB 값 설정
  setColor(this->red, this->green, this->blue);
}

// 네오픽셀 객체 배열 생성
NeoPixel p[GROUPS];    

void setup(){
  Serial.begin(9600);
  btSerial.begin(9600);
  strip.begin();
  sBright=100;
  strip.setBrightness(sBright);
  
  AutoBright = false;   // 자동 밝기
  OnPIR = false;      // 인체 감지 센서 동작
  PIR_Switch = false;  // 인체 감지 ON 이후에 1번이라도 감지하면 true
  sShow = false;      // true 일 경우 strip.show() 한 번 실행하고 false로 바뀜
  for(int i = 0; i < GROUPS; i++){
    p[i].setNumber(i);
  }
  strip.show();
}

void loop() {
  char sRead;

  if(btSerial.available()){
    sRead=btSerial.read();
    switch(sRead) {
      case 10:
          break;
      case 'a':
         if(AutoBright == true) AutoBright = false;
         else AutoBright = true;
         break;
      case 'u':
        if(AutoBright == false){  
          sBright += 20;
          if(sBright > BRIGHT_MAX) sBright = BRIGHT_MAX;
          if(sBright < BRIGHT_MIN) sBright = BRIGHT_MIN;
          strip.setBrightness(sBright);
          sShow = true;
        }
         break;
      case 'd':
        if(AutoBright == false){  
          sBright -= 20;
          if(sBright > BRIGHT_MAX) sBright = BRIGHT_MAX;
          if(sBright < BRIGHT_MIN) sBright = BRIGHT_MIN;
          strip.setBrightness(sBright);
          sShow = true;
        }
         break;
      case 'p':
          if(OnPIR == false) OnPIR = true;
          else OnPIR = false;
          PIR_Switch = false;
          break;
      case 'C':
          Custom=btSerial.parseInt();
          btSerial.readBytes(led,5);
          p[Custom].setColor(led[1],led[2],led[3]);
          sShow = true;
          break;
      case 'V':
          Custom=btSerial.parseInt();
          Red=btSerial.parseInt();
          Green=btSerial.parseInt();
          Blue=btSerial.parseInt();
          p[Custom].setColor(Red,Green,Blue);
      case 'E':
          sMode='E';
          p[3].setColor(255, 0, 0);
          p[2].setColor(255, 80, 0);
          p[1].setColor(255,255,0);
          p[0].setColor(0,255,0);
          sShow = true;
          break;
      case 'H':
          sMode='H';
          p[3].setColor(0,255, 255);
          p[2].setColor(0, 0, 255);
          p[1].setColor(128,0,255);
          p[0].setColor(255,80,166);
          sShow = true;
          break;
      case 'F':
          sMode = 'F';
          for(int i = 0; i < GROUPS; i++)
              p[i].setColor(255, 0, 0);
          break;
      case 'D':
          sMode='D';
          p[3].setColor(255,0,0);
          p[2].setColor(255, 50,0 );
          p[1].setColor(255, 255, 0);
          p[0].setColor(0, 255, 0);
          break;
      case 'r':
          sMode='r';
          Red=btSerial.parseInt();
          break;
      case 'g':
          Green=btSerial.parseInt();
          break;
      case 'b':
          Blue=btSerial.parseInt();
          for(int i=0;i<GROUPS;i++){
            p[i].setColor(Red,Green,Blue);
          }
          break;
      case 'e':
          sMode='e';
          p[3].setColor(255,0,0);
          p[2].setColor(255,20,20);
          p[1].setColor(255,41,41);
          p[0].setColor(255,68,50);
          break;
      case 'f':
          sMode='e';
          p[3].setColor(255,85,0);
          p[2].setColor(255,100,0);
          p[1].setColor(255,130,10);
          p[0].setColor(255,150,10); 
          break;
      case 'h':
          sMode='e';
          p[3].setColor(255,255,0);
          p[2].setColor(255,255,51);
          p[1].setColor(255,255,85);
          p[0].setColor(255,255,115);
          break;
      case 'i':
          sMode='e';
          p[3].setColor(0,255,0);
          p[2].setColor(55,255,35);
          p[1].setColor(101 ,255,81);
          p[0].setColor(130,255,110);
          break;
      case 'j':
          sMode='e';
          p[3].setColor(0,255,255);  
          p[2].setColor(75,255,255);
          p[1].setColor(110,255,255);
          p[0].setColor(135,255,255);
          break;
      case 'k':
          sMode='e';
          p[3].setColor(30,50,255);
          p[2].setColor(41,86,255);
          p[1].setColor(65,105,245);
          p[0].setColor(103,129,255);
          break;
      case 'l':
          sMode='e';
          p[3].setColor(128,0,255);  
          p[2].setColor(130,45,255);
          p[1].setColor(144,82,255);
          p[0].setColor(181,130,255);
          break;
      case 'm':
          sMode='e';
          p[3].setColor(255,101,160);
          p[2].setColor(255,140,166);
          p[1].setColor(255,170,167);  
          p[0].setColor(255,190,168);
          break;
      case 'M':
          sMode='M';
          btSerial.readBytes(led,4);
          p[0].setColor(led[1],led[2],led[3]);
          p[1].setColor(led[1],led[2],led[3]);
          p[2].setColor(led[1],led[2],led[3]);
          p[3].setColor(led[1],led[2],led[3]);
          sShow = true;
          break;
   }
}

  // 자동 밝기
  if(AutoBright == true) {   
    int cds;
    if(millis() >= timeNextCds) {
      timeNextCds = millis() + 100;
      cds = analogRead(A0);
      cds = map(cds, 0, 1023, 300, -50);
      if(cds < BRIGHT_MIN) cds = BRIGHT_MIN;
      if(cds > BRIGHT_MAX) cds = BRIGHT_MAX;
      strip.setBrightness(cds);
      strip.show();
    }
  }

  // 인체 감지 센서
  if(OnPIR == true) {
    int readPIR = digitalRead(PIN_PIR);
    if(PIR_Switch == false && readPIR == 0) {
      PIR_Power = false;
      PIR_Switch = true;
    }  
    if(readPIR == 1) PIR_Power = true;
  }
  else
    PIR_Power = true;
    
  // 조명 전원이 false 이면 전원을 끄고, 그렇지 않으면 동작
  if(PIR_Power == false){
    strip.clear(); 
    strip.show();
  }
  else {
      if(sMode=='D' or sMode=='F') {
          if(millis() >= timeNext){
            timeNext = millis() + 100;
            for(int i = 0; i < GROUPS; i++)
               p[i].modeRainbow();
            }
        }
        else {
          if(millis() >= timeNext){
            timeNext = millis() + 100;
            for(int i = 0; i < GROUPS; i++)
               p[i].ledShow();
          }
        }  
    }   
}
