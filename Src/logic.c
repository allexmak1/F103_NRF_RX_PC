/*Работа индикации

*/

/*Timer1
  настроен так:
  72000/7199*9 прерывание каждую каждую 1ms
*/

/*WatchDog
  настроен так:
  PCLK1/4096/VDGTB
  4500 кгц/4096/4 = 0.2746 кгц -> 3.6 ms
  таймер1 сбрасывает флаг каждую 1ms
*/
#include "logic.h"
#include "main.h"
#include "string.h"
#include "nrf24.h"


extern TIM_HandleTypeDef htim1;

jButton_t jButton       = {0};
jStick_t  jStickA       = {0};
jStick_t  jStickB       = {0};
jLed_t    jLed          = {0};
jMode_t   jMode   	= {MODE_1, MODE_1, 1};

extern uint32_t timer_Sleep;
extern uint32_t timer_SendState;
extern uint32_t timer_Led4;
extern uint32_t timer_LedLow;

volatile uint8_t flagDmaAdc = 0;
uint8_t flag1, flag2;

extern uint8_t nRF24_payload[32];
extern nRF24_RXResult pipe;
extern  uint8_t payload_length;
extern nRF24_TXResult tx_res;

void runRadio(void);
void UART_SendStr(char *string);
void UART_SendBufHex(char *buf, uint16_t bufsize);
//nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length);

void LOGICstart(){
HAL_Delay(500);
  vSetStateGpio(0, GPIOC, GPIO_PIN_13);
  vSetStateGpio(0, GPIOC, GPIO_PIN_14);
  vSetStateGpio(0, GPIOC, GPIO_PIN_15);
  vSetStateGpio(0, GPIOA, GPIO_PIN_15);
  
  HAL_TIM_Base_Start_IT(&htim1);
  //инициализация
  HAL_Delay(100);
  runRadio();
  payload_length = 12;
  HAL_Delay(500);
  
}


void LOGIC(){

  //чтение всех входов и выходов
  vReadStatePins();
  
  // подготовка данных
  nRF24_payload[0] = jButton.uint8Message.gr1;
  nRF24_payload[1] = jButton.uint8Message.gr2;;
  nRF24_payload[2] = jButton.uint8Message.gr3;;
  nRF24_payload[3] = jButton.uint8Message.gr4;;
  nRF24_payload[4] = jStickA.ValV;
  nRF24_payload[5] = jStickA.ValV >> 8;
  nRF24_payload[6] = jStickA.ValG;
  nRF24_payload[7] = jStickA.ValG >> 8;
  nRF24_payload[8] = jStickB.ValV;
  nRF24_payload[9] = jStickB.ValV >> 8;
  nRF24_payload[10] = jStickB.ValG;
  nRF24_payload[11] = jStickB.ValG >> 8;
  
  
//периодическая отправка данных по NRF
    vSendStateJ();
    
    
  if(timer_SendState > 200){
    timer_SendState = 0;
    
    //навигация между режимами
    vNavigationMode();
  }
  
  //индикация работы 4го светодиода 
  if(timer_Led4 > 3000){
  //timer1msLed = 0;
  //jLed.one = ~jLed.one &0x01;
  //jLed.two = ~jLed.two &0x01;
  //jLed.fhree = ~jLed.fhree &0x01;
  //jLed.four = ~jLed.four &0x01;
    if(timer_Led4 > 3000 && timer_Led4 < 3010) jLed.four = 1;
    if(timer_Led4 > 3100){
      timer_Led4 = 0;
      jLed.four = 0;
    }
  }

  //отсчет таймера засыпания 
  if(timer_Sleep > TIMER_SLEEP){
    //timer_Sleep=0;
    //индикация 4ой лампочки
    //vSetStateGpio(1, GPIOA, GPIO_PIN_15);
    //    //раскоментить
    //    //остановка NRF
    //    powerDown();	
    //    //остановка тактирования
    //    HAL_TIM_Base_Stop_IT(&htim1);
    //    HAL_TIM_Base_Stop(&htim1);
    //    HAL_SuspendTick();//регистр  TICKINT
    //    
    //    //засыпаем
    //    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON , PWR_SLEEPENTRY_WFI);
    //
    //    //просыпаемся
    //    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
    //    //!!!здесь проще запустить перезагрузку МК, чем запуск всей переферии, так как сбрасываются все регисторы настроек при глубоком сне
    //    //сделанно это просто - не вкючаем тактирование таймера1, там где сбрасывется сторожевой таймер, он и перезагрузит.
    //    
  }
  //HAL_Delay(10);
}

void vReadStatePins(){
  //светодиоды
  vSetStateGpio(jLed.one, GPIOC, GPIO_PIN_13);
  vSetStateGpio(jLed.two, GPIOC, GPIO_PIN_14);
  vSetStateGpio(jLed.fhree, GPIOC, GPIO_PIN_15);
  vSetStateGpio(jLed.four, GPIOA, GPIO_PIN_15);
  //кнопки (обрабатываем только те которые не EXTI)
  jButton.bit.Lb        = xGetStateGpio(jButton.bit.Lb,      GPIOB, GPIO_PIN_3);
  jButton.bit.Ls        = xGetStateGpio(jButton.bit.Ls,      GPIOB, GPIO_PIN_4);
  jButton.bit.home      = xGetStateGpio(jButton.bit.home,      GPIOB, GPIO_PIN_8);
  jButton.bit.list      = xGetStateGpio(jButton.bit.list,      GPIOB, GPIO_PIN_9);
  //какой то глюк (всегда на этой ноге 1)
  jButton.bit.back      = 0;//xGetStateGpio(jButton.bit.back,    GPIOA, GPIO_PIN_12);
  
  jButton.bit.StickA    = xGetStateGpio(jButton.bit.StickA,  GPIOB, GPIO_PIN_11);
  jButton.bit.StickB    = xGetStateGpio(jButton.bit.StickB,  GPIOB, GPIO_PIN_10);
  //
  jButton.bit.up        = xGetStateGpio(jButton.bit.up,  GPIOB, GPIO_PIN_12);
  jButton.bit.dawn      = xGetStateGpio(jButton.bit.dawn,  GPIOB, GPIO_PIN_13);
  jButton.bit.write     = xGetStateGpio(jButton.bit.write,  GPIOB, GPIO_PIN_14);
  jButton.bit.left      = xGetStateGpio(jButton.bit.left,  GPIOB, GPIO_PIN_15);
  jButton.bit.O         = xGetStateGpio(jButton.bit.O,  GPIOA, GPIO_PIN_8);
  
  
  //jButton.bit.X         = xGetStateGpio(jButton.bit.X,  GPIOA, GPIO_PIN_9);
  //jButton.bit.A         = xGetStateGpio(jButton.bit.A,  GPIOA, GPIO_PIN_10);
  
  jButton.bit.B         = xGetStateGpio(jButton.bit.B,  GPIOA, GPIO_PIN_11);
  jButton.bit.Wb        = xGetStateGpio(jButton.bit.Wb,  GPIOB, GPIO_PIN_5);
  jButton.bit.Ws        = xGetStateGpio(jButton.bit.Ws,  GPIOB, GPIO_PIN_6);
  jButton.bit.start     = xGetStateGpio(jButton.bit.start,  GPIOA, GPIO_PIN_0);
  jButton.bit.select    = xGetStateGpio(jButton.bit.select,  GPIOB, GPIO_PIN_7);

  //кнопки которые в EXTI смотрим только отжатие
  /*if(jButton.bit.up)    jButton.bit.up    = xGetStateGpio(jButton.bit.up,      GPIOB, GPIO_PIN_12);
  if(jButton.bit.dawn)  jButton.bit.dawn  = xGetStateGpio(jButton.bit.dawn,    GPIOB, GPIO_PIN_13);
  if(jButton.bit.write) jButton.bit.write = xGetStateGpio(jButton.bit.write,   GPIOB, GPIO_PIN_14);
  if(jButton.bit.left)  jButton.bit.left  = xGetStateGpio(jButton.bit.left,    GPIOB, GPIO_PIN_15);
  if(jButton.bit.O)     jButton.bit.O     = xGetStateGpio(jButton.bit.O,       GPIOA, GPIO_PIN_8);
  if(jButton.bit.X)     jButton.bit.X     = xGetStateGpio(jButton.bit.X,       GPIOA, GPIO_PIN_9);
  if(jButton.bit.A)     jButton.bit.A     = xGetStateGpio(jButton.bit.A,       GPIOA, GPIO_PIN_10);
  if(jButton.bit.B)     jButton.bit.B     = xGetStateGpio(jButton.bit.A,       GPIOA, GPIO_PIN_11);
  if(jButton.bit.start) jButton.bit.start = xGetStateGpio(jButton.bit.start,   GPIOA, GPIO_PIN_0);
  if(jButton.bit.select)jButton.bit.select= xGetStateGpio(jButton.bit.select,  GPIOB, GPIO_PIN_7);
  if(jButton.bit.Wb)    jButton.bit.Wb    = xGetStateGpio(jButton.bit.Wb,    GPIOB, GPIO_PIN_5);
  if(jButton.bit.Ws)    jButton.bit.Ws    = xGetStateGpio(jButton.bit.Ws,    GPIOB, GPIO_PIN_6);*/
  

}
//определение изменения состояния входа
uint8_t xGetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
  uint8_t state = stateInput;
  if(state != (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin))){
    state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    timer_Sleep=0;
  }
  return state;
}
//определение изменения состояния выхода
void vSetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
  if(stateInput == 1){
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
  }else {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
  }
}
void vSendStateJ(){
  
          if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY) {
            // Get a payload from the transceiver
            pipe = nRF24_ReadPayload(nRF24_payload, &payload_length);

            // Clear all pending IRQ flags
            nRF24_ClearIRQFlags();

            // Print a payload contents to UART
            UART_SendStr("RCV PIPE#");
            UART_SendInt(pipe);
            UART_SendStr(" PAYLOAD:>");
            UART_SendBufHex((char *)nRF24_payload, payload_length);
            UART_SendStr("<\r\n");
        }

}



//навигация между режимами
void vNavigationMode(){
   if(jButton.bit.back){ //перекл, осн/доп режим
    if(flag1){
      flag1 = 0;
      jMode.isMode = ~jMode.isMode & 0x01;
    }
  }else flag1 = 1;
  if(jButton.bit.list){ //перекл, режима
    if(flag2){
      flag2 = 0;
      if(jMode.isMode){
        switch(jMode.osn){
        case MODE_1:
          jMode.osn = MODE_2;
          break;
        case MODE_2:
          jMode.osn = MODE_3;
          break;
        case MODE_3:
          jMode.osn = MODE_1;
          break;
        }
      }else{
        switch(jMode.dop){
        case MODE_1:
          jMode.dop = MODE_2;
          break;
        case MODE_2:
          jMode.dop = MODE_3;
          break;
        case MODE_3:
          jMode.dop = MODE_1;
          break;
        }
      }
    }
  }else flag2 = 1;
  if(jMode.isMode == 0){
    //режим
    switch(jMode.osn){
    case MODE_1:
      jLed.one   = 1;
      jLed.two   = 0;
      jLed.fhree = 0;
      break;
    case MODE_2:
      jLed.one   = 0;
      jLed.two   = 1;
      jLed.fhree = 0;
      break;
    case MODE_3:
      jLed.one   = 0;
      jLed.two   = 0;
      jLed.fhree = 1;
      break;
    }
  }else{
    //режим дополнительный
    switch(jMode.dop){
    case MODE_1:
      vToogleLedLow(jLed.one);
      jLed.two   = 0;
      jLed.fhree = 0;
      break;
    case MODE_2:
      jLed.one   = 0;
      vToogleLedLow(jLed.two);
      jLed.fhree = 0;
      break;
    case MODE_3:
      jLed.one   = 0;
      jLed.two   = 0;
      vToogleLedLow(jLed.fhree);
      break;
    }
  }
}

//мигание светодиода, медленное
void vToogleLedLow(uint8_t led){
  if(timer_LedLow > 1000){
    timer_LedLow = 0;
    led = ~led & 0x01;
  }
}

//масштабирование
int map_i (int x, int in_min, int in_max, int out_min, int out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


















