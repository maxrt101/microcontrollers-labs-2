#include <project.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_ALGORITHMS 3
#define MAX_ALGO_PERIODS 7
#define CUSTOM_LIGHT_PERIOD 2
#define QUAD_DEC_MAX_VALUE 20
#define STARTUP_DELAY 5
#define AUTHOR "maxrt"

typedef enum State {
  S_STARTUP, S_IDLE, S_PATTERN_SETUP
} State;

typedef enum Color {
  _, R, Y, G
} Color;

typedef struct LightPeriod {
  uint8 startTime;
  uint8 endTime;
  Color color;
} LightPeriod;

static const LightPeriod periods[MAX_ALGORITHMS][MAX_ALGO_PERIODS] = {
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
  },
  {
    {0,  0,  R},
    {0,  0,  G},
    {0,  0,  Y},
    {0,  0,  _}
  }
};

State state = S_IDLE;
uint8 counter = 0;
int8 algorithm = -1;

uint8 quadDecValue = 0;
uint8 customPatternIndex = 0;
uint8 countDowns[4] = {0};

void switchLed(Color color, int state) {
  switch (color) {
    case R: Red_Pin_Write(state);     break;
    case Y: Yellow_Pin_Write(state);  break;
    case G: Green_Pin_Write(state);   break;
    default:                          break;
  }
}

void switchAlgo() {
  counter = 0;
  if (algorithm+1 < MAX_ALGORITHMS) {
    algorithm++;
  } else {
    algorithm = -1;
  }
}

void resetAlgo() {
  counter = 0;
  algorithm = 0;
}

void printAuthor() {
  LCD_ClearDisplay();
  LCD_Position(0, 0);
  LCD_PrintString(AUTHOR);
}

void printCountdown() {
  char buff[17] = {0};
  snprintf(buff, 17, "%02d %02d %02d", countDowns[R], countDowns[Y], countDowns[G]);

  LCD_ClearDisplay();

  LCD_Position(0, 0);
  LCD_PrintString("RED YELLOW GREEN");

  LCD_Position(1, 0);
  LCD_PrintString(buff);
}

void printMenu() {
  LCD_ClearDisplay();
  LCD_Position(0, 0);

  if (customPatternIndex == R-1) {
    LCD_PrintString("SET RED TIME:");
  } else if (customPatternIndex == Y-1) {
    LCD_PrintString("SET YELLOW TIME:");
  } else if (customPatternIndex == G-1) {
    LCD_PrintString("SET GREEN TIME:");
  }

  char buff[3] = {0};
  sprintf(buff, "%02d", quadDecValue);

  LCD_Position(1, 0);
  LCD_PrintString(buff);
}

void updateQuadDecValue() {
  quadDecCount = QuadDec_GetCounter();
  if (quadDecCount < 0) quadDecCount = QUAD_DEC_MAX_VALUE;
  if (quadDecCount > QUAD_DEC_MAX_VALUE) quadDecCount = 0;
  QuadDec_SetCounter(0);
}

void tick() {
  switch (status) {
    case S_STARTUP:
      printAuthor();
      if (counter >= STARTUP_DELAY) {
        status = S_IDLE;
      }
      break;
    case S_IDLE:
      if (algorithm >= 0 && algorithm < MAX_ALGORITHMS) {
        int i = 0, algoEndTime = 0;
        LightPeriod period;

        do {
          algoEndTime = period.endTime;
          period = periods[algorithm][i++];
          if (period.endTime < counter)
            continue;
          bool on = (counter >= period.startTime && counter < period.endTime);
          switchLed(period.color, on ? 0 : 1);
          countDowns[period.color] = on ? period.endTime - counter : 0;
        } while (period.color);

        if (counter >= algoEndTime) {
          switchAlgo();
        }
      } else {
        resetAlgo();
      }
      break;
    case S_PATTERN_SETUP:
      updateQuadDecValue();
      printMenu();
      break;
  }
}

void handleQuadDeck() {
  if (status == S_IDLE) {
    status = S_PATTERN_SETUP;
  }

  if (status == S_PATTERN_SETUP) {
    int start = 0;
    if (customPatternIndex-2 > 0) {
      start = periods[CUSTOM_LIGHT_PERIOD][customPatternIndex-2].endTime;
    }

    periods[CUSTOM_LIGHT_PERIOD][customPatternIndex-1].startTime = start;
    periods[CUSTOM_LIGHT_PERIOD][customPatternIndex-1].endTime = start + quadDecCount;

    if (customPatternIndex < 3) {
      customPatternIndex++;
    } else {
      status = S_IDLE;
      resetAlgo();
    }
  }
  QuadDec_Pin_ClearInterrupt();
}

CY_ISR(isr_timer_interrupt) {
  counter++;
  tick();
  Timer_ClearInterrupt(Timer_INTR_MASK_TC);
}

CY_ISR(isr_quaddec_button_interrupt) {
  handleQuadDeck();
  QuadDec_Pin_ClearInterrupt();
}

CY_ISR(isr_button_interrupt) {
  state = S_IDLE;
  resetAlgo();
  Button_Pin_ClearInterrupt();
}

int main() {
  CyGlobalIntEnable;

  Timer_Start();
  LCD_Start();
  QuadDec_Start();

  isr_timer_StartEx(isr_timer_interrupt);
  isr_button_StartEx(isr_button_interrupt);
  isr_quaddec_button_StartEx(isr_quaddec_button_interrupt);

  for (;;) ;
}
