#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


struct LedInfo {
  int pins[3];
  int level;
};

struct RowColor {
  int r, g, b;
};

LedInfo led_info[3] = {
  {{2, 4, 5}, 0},
  {{13, 12, 14}, 1},
  {{19, 21, 22}, 2}
};

class StarRow {
  public:
  int row_id;
  void setColor(RowColor color) {
    LedInfo& l = led_info[row_id];
    analogWrite(l.pins[0], color.r);
    analogWrite(l.pins[1], color.g);
    analogWrite(l.pins[2], color.b);
  }
};

void StarSetup(StarRow led_row[3]) {
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      pinMode(led_info[x].pins[y], OUTPUT);
    }
  }
  for (int x = 0; x < 3; x++) {
    led_row[x].row_id = x;
  }
}

class Strip {
  public:
    int index;
    void setColor(RowColor color) {
      float red=scale*color.r;
      float blue=scale*color.b;
      float green=scale*color.g;
      if(color.r == 0){
        red = 4096;
      }
      if(color.b == 0){
        blue = 4096;
      }
      if(color.g == 0){
        green = 4096;
      }
      if(color.r == 255){
        red = 0;
      }
      if(color.b == 255){
        blue = 0;
      }
      if(color.g == 255){
        green = 0;
      }
      pwm.setPWM((2+index)*3, blue, 4096-blue); //blue
      pwm.setPWM((2+index)*3+1, red, 4096-red); //red
      pwm.setPWM((2+index)*3+2, green, 4096-green); //green
      pwm.setPWM((2-index)*3, blue, 4096-blue); //blue
      pwm.setPWM((2-index)*3+1, red, 4096-red); //red
      pwm.setPWM((2-index)*3+2, green, 4096-green); //green
    }
  private:
    const float scale = 2048.0/255.0;
};

void StripSetup(Strip strip_row[10]) {
  for (int x = 0; x < 3; x++) {
    strip_row[x].index = x;
  }
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(1600);
  for (int i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, 4096);
  }
}