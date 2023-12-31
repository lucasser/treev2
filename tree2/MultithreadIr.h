#include "esp32-hal.h"
#include <IRremote.hpp>

bool on = false;
String input = "";
int ir_pin;
TaskHandle_t ir_task;
QueueHandle_t queue;

void translateIR() {
  switch(IrReceiver.decodedIRData.decodedRawData) {
  case 3125149440: input = ("POWER"); break;
  case 3091726080: input = ("FUNC/STOP"); break;
  case 3108437760: input = ("VOL+"); break;
  case 3141861120: input = ("FAST BACK");    break;
  case 3208707840: input = ("PAUSE");    break;
  case 3158572800: input = ("FAST FORWARD");   break;
  case 4161273600: input = ("DOWN");    break;
  case 3927310080: input = ("VOL-");    break;
  case 4127850240: input = ("UP");    break;
  case 3860463360: input = ("EQ");    break;
  case 4061003520: input = ("ST/REPT");    break;
  case 3910598400: input = ("0");    break;
  case 4077715200: input = ("1");    break;
  case 3877175040: input = ("2");    break;
  case 2707357440: input = ("3");    break;
  case 4144561920: input = ("4");    break;
  case 3810328320: input = ("5");    break;
  case 2774204160: input = ("6");    break;
  case 3175284480: input = ("7");    break;
  case 2907897600: input = ("8");    break;
  case 3041591040: input = ("9");    break;
  case 0: break;

  default: 
    input = String(IrReceiver.decodedIRData.decodedRawData);
  }
  delay(500);
}

void irMultithread(void * pvParameters) {
  for(;;) {
    if (IrReceiver.decode()) {
      translateIR();
      IrReceiver.resume();
      if (input == "POWER") {
        on = !on;
        input = "";
      }
    }
    delay(10);
    //queue = xQueueCreate(10, sizeof(int));
    //xQueueReceive(queue, &data, portMAX_DELAY);
  }
}

void IrSetup (int irPin) {
  ir_pin = irPin;
  IrReceiver.begin(ir_pin, DISABLE_LED_FEEDBACK);
  xTaskCreatePinnedToCore(irMultithread, "ir_task", 10000, NULL, 1, &ir_task, 0);
  delay(500);
}