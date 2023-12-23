/*
contrlooer for star, no servo drive
*/

#include "MultithreadIr.h"

const int GARLAND = 23;
const int IR_PIN = 15;

struct LedInfo {
  int pins[3];
  int level;
};

struct RowColor {
  int r, g, b;
};

struct StarColor {
  RowColor row_color[3];  
};

LedInfo led_info[3] = {
  {{2, 4, 5}, 0},
  {{13, 12, 14}, 1},
  {{19, 21, 22}, 2}
};


class Operands {
  public:
    RowColor times(RowColor color, float multiplier) {
      RowColor result;
      result.r = color.r * multiplier;
      result.g = color.g * multiplier;
      result.b = color.b * multiplier;
      return result;
    }
};

Operands operands;
StarColor star_color;

class LedRow {
  public:
  int row_id;
  void setColor(RowColor color) {
    LedInfo& l = led_info[row_id];
    analogWrite(l.pins[0], color.r);
    analogWrite(l.pins[1], color.g);
    analogWrite(l.pins[2], color.b);
  }
};

class FadeThruBlack {
  private:
    int state;
    float x;
    int iter;
    RowColor colors[8] = {
      {255, 0, 0},
      {255, 191, 0},
      {255, 255, 0},
      {0, 255, 0},
      {0, 255, 255},
      {0, 0, 255},
      {191, 0, 255},
      {255, 0, 255}
    };
  public:
    RowColor target_color;
    RowColor cur_color;
    FadeThruBlack (RowColor start_color) {
      cur_color = start_color;
      target_color = colors[0];
      x = 1.0;
      state = 0;
      iter = 0;
    }
    RowColor tick() {
      switch (state) {
        case 0:
          x += .1;
          if (x >= 10) {
            state = 1;
            x = 10;
            iter++;
            target_color = colors[iter];
            if (iter >= 7) {
              iter = -1;
            }
          } else {
            cur_color = operands.times(target_color, 1.0/x);
          }
          break;
        case 1:
          x -= .1;
          if (x <= 1) {
            Serial.println(iter);
            state = 0;
            x = 1;
          } else {
            cur_color = operands.times(target_color, 1.0/x);
          }
          break;
        default:
          state = 0;
          target_color = colors[0];
          x = 1;
      }
      return cur_color;
    }
};

class Paint {
  private:
    int state;
    float x;
  public:
    RowColor fin_color;
    StarColor cur_color;
  Paint (RowColor fin_color, StarColor start_color) {
    this-> fin_color = fin_color;
    state = 0;
    cur_color = start_color;
    x = 10;
  }
  StarColor tick() {
    switch (state) {
      case 0:
        x -= .1;
        if (x <= 1) {
          state = 1;
          x = 10;
        } else {
          cur_color.row_color[0] = operands.times(fin_color, 1.0/x);
        }
        break;
      case 1:
        x -= .1;
        if (x <= 1) {
          state = 2;
          x = 10;
        } else {
          cur_color.row_color[1] = operands.times(fin_color, 1.0/x);
        }
        break;
      case 2:
        x -= .1;
        if (x <= 1) {
          state = 3;
          x = 10;
        } else {
          cur_color.row_color[2] = operands.times(fin_color, 1.0/x);
        }
        break;
      case 3:
        break;
      default:
        state = 0;
        x = 10;
    }
    return cur_color;
  }
};

class ColorCycle {
  private:
    int state;
  public:
    RowColor color;
    ColorCycle (RowColor color, int state) {
      this-> color = color;
      this-> state = state;
    }    
    RowColor tick() {
      switch (state) {
        case 0:
          color.g++;
          if (color.g >= 255) {
            color.g = 255;
            state++;
          }
          break;
        case 1:
          color.r--;
          if (color.r <= 0) {
            color.r = 0;
            state++;
          }
          break;
        case 2:
          color.b++;
          if (color.b >= 255) {
            color.b = 255;
            state++;
          }
          break;
        case 3:
          color.g--;
          if (color.g <= 0) {
            color.g = 0;
            state++;
          }
          break;
        case 4:
          color.r++;
          if (color.r >= 255) {
            color.r = 255;
            state++;
          }
          break;
        case 5:
          color.b--;
          if (color.b <= 0) {
            color.b = 0;
            state = 0;
          }
          break;
        default:
          state = 0;
          break;
      }
      return color;
    }
};

class Star {
  private:
    int t = 0;
    int k = 0;
    int state = 0;
  public:
  ColorCycle *color_cycles[3];
  Paint *paint;
  FadeThruBlack *fade[3];
  LedRow led_row[3];
  void setup() {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        pinMode(led_info[x].pins[y], OUTPUT);
      }
    }
    for (int x = 0; x < 3; x++) {
      led_row[x].row_id = x;
    }

  }
  
  void setColor(StarColor color) {
    for (int x = 0; x < 3; x++) {
      led_row[x].setColor(color.row_color[x]);
    }
  }

  void tick() {
    switch (state) {
      case 0:
        star_color.row_color[0] = color_cycles[0]-> tick();
        star_color.row_color[1] = color_cycles[1]-> tick();
        star_color.row_color[2] = color_cycles[2]-> tick();
        t++;
        break;
      case 1:
        star_color.row_color[0] = fade[0]-> tick();
        star_color.row_color[1] = fade[1]-> tick();
        star_color.row_color[2] = fade[2]-> tick();
        t++;
        break;
      case 2:
        star_color = paint-> tick();
        k++;
        break;
      default: state = 0;
    };
    if (t >= 2000) {
      t = 0;
      paint = new Paint(RowColor {int(random(256)), int(random(256)), int(random(256))}, star_color);
      state = 2;
      k = 0;
    }
    if (k >= 500) {
      int x = random(0, 2);
      switch (x) {
        case 0:
          color_cycles[0] = new ColorCycle(star_color.row_color[0], 0);
          color_cycles[1] = new ColorCycle(star_color.row_color[1], 0);
          color_cycles[2] = new ColorCycle(star_color.row_color[2], 0);
          state = 0;
          break;
        case 1:
          fade[0] = new FadeThruBlack(star_color.row_color[0]);
          fade[1] = new FadeThruBlack(star_color.row_color[1]);
          fade[2] = new FadeThruBlack(star_color.row_color[2]);
          state = 1;
          break;
        default:
          break;
      };
      k = 0;
    }
    setColor(star_color);
  }
};

Star star;

void setup() {
  Serial.begin(9600);
  star.setup();
  IrSetup(IR_PIN);
  pinMode(GARLAND, OUTPUT);
  star.color_cycles[0] = new ColorCycle(RowColor {255, 0, 0}, 0);
  star.color_cycles[1] = new ColorCycle(RowColor {255, 0, 0}, 0);
  star.color_cycles[2] = new ColorCycle(RowColor {255, 0, 0}, 0);
}

void loop() {
  if (on) {
    star.tick();
    digitalWrite(GARLAND, HIGH);
  } else {
    star.setColor(StarColor {RowColor {0, 0, 0}, RowColor {0, 0, 0}, RowColor {0, 0, 0}});
    digitalWrite(GARLAND, LOW);
  }
  delay(20);
}