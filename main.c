#include <project.h>

int main() {
    CyGlobalIntEnable;

    const uint8 patterns[] = {0b00111, 0b01101, 0b10101, 0b01110, 0b10110, 0b11100, 0b11010, 0};

    for(;;) {
        uint8 btn = BUTTON_Read();
        if (btn == 0b11111) continue;

        uint8 wasLastPatternGood = 0;

        for (int i = 0; patterns[i]; i++) {
            if (btn == patterns[i]) {
                LED_Write(0);
                wasLastPatternGood = 1;
                break;
            }
        }

        if (!wasLastPatternGood) {
            LED_Write(1);
        }
    }
}
