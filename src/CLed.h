#ifndef CLed_h
#define CLed_h
#include "Defy.h"

typedef enum {LED_OFF,LED_ON,LED_BREATH,LED_FLASH} CLedTryb;

class CLed
{
    uint8_t _pin;
    CLedTryb _tryb;
    uint16_t _okres;
    uint16_t _delayBefore;
    uint16_t _startON;
    uint16_t _stopON;
    uint16_t _delayAfter;


};

#endif