#include "main.h"
#include "stm32f1xx_hal.h"

//кнопки
typedef union{
  struct{
    uint8_t up    :1;//0.0
    uint8_t dawn  :1;//0.1
    uint8_t write :1;//0.2
    uint8_t left  :1;//0.3
    uint8_t A     :1;//0.4
    uint8_t Y     :1;//0.5
    uint8_t X     :1;//0.6
    uint8_t B     :1;//0.7
    
    uint8_t Lb    :1;//1.0
    uint8_t Ls    :1;//1.1
    uint8_t Rb    :1;//1.2
    uint8_t Rs    :1;//1.3
    uint8_t start :1;//1.4
    uint8_t select:1;//1.5
    uint8_t home  :1;//1.6
    uint8_t back  :1;//1.7--не работает
    
    uint8_t list  :1;//2.0--не работает
    uint8_t StickA:1;//2.1
    uint8_t StickB:1;//2.2
    uint8_t       :5;//2.3
    
    uint8_t osn   :4;//3.0
    uint8_t dop   :4;//3.4
  }bit;
  struct{
    uint8_t gr1;
    uint8_t gr2;
    uint8_t gr3;
    uint8_t gr4;
  }uint8Message;
  uint32_t uint32Message;
}jButton_t;

//стики
typedef struct{
  int16_t ValV;
  int16_t ValG;
}jStick_t;

//режимы работы
typedef enum{
  MODE_1 = 1,
  MODE_2, 
  MODE_3
} mode_e;

typedef struct{
  mode_e    osn;
  mode_e    dop;
  uint8_t   isMode;
}jMode_t;

//светодиоды и лампы
typedef struct{
  int16_t    nrfState;
  int16_t    ftont;
  int16_t    back;
  int16_t    left;
  int16_t    right;
  int16_t    toogleR;
  int16_t    toogleL;
}led_t;

typedef enum{
  NRF_OFF = 0,
  NRF_ON
}stateNrf_e;

void LOGICstart();
void LOGIC();
void vReadStatePins();
uint8_t xGetStateGpio(uint8_t stateOutpt, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void vSetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t reverse);
void vReadNRF();
void vMove();
uint8_t xToogleLedLow(uint8_t led);
void vToogleLedNrfNormal();
uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);
void vLedToogle();