#include <project.h>
#include <stdbool.h>

typedef enum Color {
  _, R, Y, G
} Color;

typedef struct LightPeriod {
  uint8 startTime;
  uint8 endTime;
  Color color;
} LightPeriod;

static const LightPeriod periods[2][7] = {
  {
    {0,  8,  R},
    {8,  10, Y},
    {10, 14, G},
    {16, 17, G},
    {18, 19, G},
    {19, 20, Y},
    {0,  0,  _}
  },
  {
    {0,  4,  G},
    {6,  7,  G},
    {9,  10, G},
    {10, 12, Y},
    {12, 20, R},
    {20, 22, Y},
    {0,  0,  _}
  }
};

uint8 counter = 0;
uint8 algorithmLength = 0;
int8 algorithm = -1;
uint8 maxAlgorithms = sizeof(periods) / sizeof(LightPeriod);

void switchLed(Color color, int state) {
  switch (color) {
    case R: Red_Pin_Write(state);     break;
    case Y: Yellow_Pin_Write(state);  break;
    case G: Green_Pin_Write(state);   break;
    default:                          break;
  }
}

void tick() {
  if (algorithm != -1 && algorithm < maxAlgorithms) {
    int i = 0;
    LightPeriod period;
    do {
      period = periods[algorithm][i++];
      switchLed(period.color, (counter >= period.startTime && counter < period.endTime) ? 0 : 1);
    } while (period.color);
  }
}

void switchAlgo() {
  if (algorithm+1 < maxAlgorithms) {
    algorithm++;
    algorithmLength = 0;
    while (periods[algorithm][algorithmLength].color) algorithmLength++;
  } else {
    algorithm = -1;
  }
}

CY_ISR(isr_timer_interrupt) {
  counter++;

  Timer_ClearInterrupt(Timer_INTR_MASK_TC);
}

CY_ISR(isr_button_interrupt) {
  counter = 0;
  switchAlgo();

  Button_Pin_ClearInterrupt();
}

int main() {
  CyGlobalIntEnable;

  Timer_Start();
  isr_timer_StartEx(isr_timer_interrupt);
  isr_button_StartEx(isr_button_interrupt);

  for(;;) {
    tick();
  }
}
