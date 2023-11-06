#include "logic.h"
#include "main.h"
#include "string.h"
#include "nrf24.h"
#include "config.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

jButton_t       jButton       = {0};
jStick_t        jStickA       = {0};
jStick_t        jStickB       = {0};
jMode_t         jMode         = {MODE_1, MODE_1, 1};
led_t           led;
stateNrf_e      stateNrf      = NRF_OFF;

extern uint32_t timer_Sleep;
extern uint32_t timer_SendState;
extern uint32_t timer_Led4;
extern uint32_t timer_LedLow;
extern uint32_t timer_LedToogle;

volatile uint8_t flagDmaAdc = 0;
uint8_t flag1, flag2;
uint8_t flagFtont = 1;
uint8_t flagBack = 1;
uint8_t flagLeft, flagRight;

uint32_t valLR = 0;
uint8_t flag3 = 0;
uint8_t flag4 = 0;

extern uint8_t nRF24_payload[32];
extern nRF24_RXResult pipe;
extern uint8_t payload_length;

uint8_t isLedToogle, isFlagLedToogle;

void runRadio(void);
#ifdef UART_ON
void UART_SendStr(char *string);
void UART_SendInt(int32_t num);
void UART_SendBufHex(char *buf, uint16_t bufsize);
#endif

void LOGICstart(){
  int wile = 0;
  HAL_Delay(300);
  vSetStateGpio(0, GPIOC, GPIO_PIN_13, 1);
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  HAL_Delay(100);
  
  //инициализация NRF
  //runRadio();
  nRF24_CE_L();// RX/TX disabled
  while(wile == 0){
    if(nRF24_Check()){
      wile = 1;
    }
#ifdef UART_ON
    UART_SendStr("nRF24L01+ check: FAIL\r\n");
#endif
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    Delay_ms(50);
  }
#ifdef UART_ON
  UART_SendStr("nRF24L01+ check: OK\r\n");
#endif
  // Initialize the nRF24L01 to its default state
  nRF24_Init();
  
  // This is simple receiver with one RX pipe:
  //   - pipe#1 address: '0xE7 0x1C 0xE3'
  //   - payload: 5 bytes
  //   - RF channel: 115 (2515MHz)
  //   - data rate: 250kbps (minimum possible, to increase reception reliability)
  //   - CRC scheme: 2 byte
  // The transmitter sends a 5-byte packets to the address '0xE7 0x1C 0xE3' without Auto-ACK (ShockBurst disabled)
  // Disable ShockBurst for all RX pipes
  nRF24_DisableAA(0xFF);
  // Set RF channel
  nRF24_SetRFChannel(115);
  // Set data rate
  nRF24_SetDataRate(nRF24_DR_250kbps);
  // Set CRC scheme
  nRF24_SetCRCScheme(nRF24_CRC_2byte);
  // Set address width, its common for all pipes (RX and TX)
  nRF24_SetAddrWidth(3);
  // Configure RX PIPE#1
  static const uint8_t nRF24_ADDR[] = { 0xE7, 0x1C, 0xE3 };
  nRF24_SetAddr(nRF24_PIPE1, nRF24_ADDR); // program address for RX pipe #1
  nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_OFF, 12); // Auto-ACK: disabled, payload length: 5 bytes
  // Set operational mode (PRX == receiver)
  nRF24_SetOperationalMode(nRF24_MODE_RX);
  // Wake the transceiver
  nRF24_SetPowerMode(nRF24_PWR_UP);
  // Put the transceiver to the RX mode
  nRF24_CE_H();/*
  // This is simple receiver with Enhanced ShockBurst:
  //   - RX address: 'ESB'
  //   - payload: 10 bytes
  //   - RF channel: 40 (2440MHz)
  //   - data rate: 2Mbps
  //   - CRC scheme: 2 byte
  // The transmitter sends a 10-byte packets to the address 'ESB' with Auto-ACK (ShockBurst enabled)
  // Set RF channel
  nRF24_SetRFChannel(40);
  // Set data rate
  nRF24_SetDataRate(nRF24_DR_2Mbps);
  // Set CRC scheme
  nRF24_SetCRCScheme(nRF24_CRC_2byte);
  // Set address width, its common for all pipes (RX and TX)
  nRF24_SetAddrWidth(3);
  // Configure RX PIPE
  static const uint8_t nRF24_ADDR[] = {'E', 'S', 'B'};
  nRF24_SetAddr(nRF24_PIPE1, nRF24_ADDR); // program address for pipe
  nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_ON, 10); // Auto-ACK: enabled, payload length: 10 bytes
  // Set TX power for Auto-ACK (maximum, to ensure that transmitter will hear ACK reply)
  nRF24_SetTXPower(nRF24_TXPWR_0dBm);
  // Set operational mode (PRX == receiver)
  nRF24_SetOperationalMode(nRF24_MODE_RX);
  // Clear any pending IRQ flags
  nRF24_ClearIRQFlags();
  // Wake the transceiver
  nRF24_SetPowerMode(nRF24_PWR_UP);
  // Enable DPL
  nRF24_SetDynamicPayloadLength(nRF24_DPL_ON);
  nRF24_SetPayloadWithAck(1);
  // Put the transceiver to the RX mode
  nRF24_CE_H();*/
  
  
  payload_length = 12;//16
  HAL_Delay(500);
  
}


void LOGIC(){
  //чтение данных
  vReadNRF();
  
  //движение
  vMove();
  
  //зуммер
  if(jButton.bit.A){
  }
  if(jButton.bit.B){
  }
  if(jButton.bit.X){
  }
  if(jButton.bit.Y){
  }
  
  //фары передние
  if(jButton.bit.Rs){
    if(flagFtont){
      flagFtont = 0;
      led.ftont = ~led.ftont & 0x01;
    }
    timer_Sleep=0;
  }else {
    flagFtont = 1;
  }
  
  //фары задние
  if(jButton.bit.Ls){
    if(flagBack){
      flagBack = 0;
      led.back = ~led.back & 0x01;
    }
    timer_Sleep=0;
  }else flagBack = 1;
  
  //мигалка
  if(jButton.bit.home){
    if(!isFlagLedToogle){
      isFlagLedToogle = 1;
      isLedToogle = ~isLedToogle & 0x01;
      led.toogleR = 1;
    }
  }else{
    isFlagLedToogle = 0;
  }
  vLedToogle();
  
  //поворотники правый
  if(jButton.bit.Rb){
    if(flagRight == 0){
      flagRight = 1;
      flagLeft = 0;
    }
    timer_Sleep=0;
  }
  if(flagRight){
    led.right = xToogleLedLow(led.right);
  }else led.right = 0;
  
  //поворотники левый
  if(jButton.bit.Lb){
    if(flagLeft == 0){
      flagLeft = 1;
      flagRight = 0;
    }
    timer_Sleep=0;
  }
  if(flagLeft){
    //led.left = 1;
    led.left = xToogleLedLow(led.left);
  }else {
    led.left = 0;}
  
  //индикация работы светодиода состояния NRF
  if(timer_SendState > 400){
    stateNrf = NRF_OFF;
  }  
  if(stateNrf == NRF_OFF){
    //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    led.nrfState = (~led.nrfState)& 0x1;
    Delay_ms(150);
  }else{
    vToogleLedNrfNormal();
  }
  
  //вывод индикации
  vSetStateGpio(led.nrfState,   GPIOC, GPIO_PIN_13, 0);
  vSetStateGpio(led.ftont,      GPIOB, GPIO_PIN_12, 0);
  vSetStateGpio(led.back,       GPIOB, GPIO_PIN_13, 0);
  vSetStateGpio(led.right,      GPIOB, GPIO_PIN_14, 0);
  vSetStateGpio(led.left,       GPIOB, GPIO_PIN_15, 0);
  vSetStateGpio(led.toogleR,    GPIOA, GPIO_PIN_11, 0);
  vSetStateGpio(led.toogleL,    GPIOA, GPIO_PIN_12, 0);
  
  //отсчет таймера засыпания 
  if(timer_Sleep > TIMER_SLEEP){
    timer_Sleep=0;
    //индикация лампочки состояния
    vSetStateGpio(1, GPIOC, GPIO_PIN_13, 1);
    //остановка NRF
    nRF24_SetPowerMode(nRF24_PWR_DOWN);	
    //остановка тактирования
    HAL_TIM_Base_Stop_IT(&htim1);
    HAL_TIM_Base_Stop(&htim1);
    HAL_SuspendTick();//регистр  TICKINT
    //засыпаем
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON , PWR_SLEEPENTRY_WFI);
    //просыпаемся
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
    //!!!здесь проще запустить перезагрузку МК, чем запуск всей переферии, так как сбрасываются все регисторы настроек при глубоком сне
    //сделанно это просто - не вкючаем тактирование таймера1, там где сбрасывется сторожевой таймер, он и перезагрузит.
  }
  //HAL_Delay(10);
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
void vSetStateGpio(uint8_t stateInput, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t reverse){
  if(stateInput == reverse){
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
  }else {
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
  }
}

//чтение NRF
void vReadNRF(){
  if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY) {
    // Get a payload from the transceiver
    pipe = nRF24_ReadPayload(nRF24_payload, &payload_length);
    // Clear all pending IRQ flags
    nRF24_ClearIRQFlags();
    // Print a payload contents to UART
#ifdef UART_ON
    UART_SendStr("RCV PIPE#");
    UART_SendInt(pipe);
    UART_SendStr(" PAYLOAD:>");
    UART_SendBufHex((char *)nRF24_payload, payload_length);
    UART_SendStr("<\r\n");
#endif
    stateNrf = NRF_ON;
    timer_SendState = 0;
  }/*
  //
  // Constantly poll the status of the RX FIFO and get a payload if FIFO is not empty
  //
  // This is far from best solution, but it's ok for testing purposes
  // More smart way is to use the IRQ pin :)
  //
  if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY) {
  // Get a payload from the transceiver
  pipe = nRF24_ReadPayloadDpl(nRF24_payload, &payload_length);
  if(payload_length > 0) {
  nRF24_WriteAckPayload(pipe, "aCk PaYlOaD",11);
}
  
  // Clear all pending IRQ flags
  nRF24_ClearIRQFlags();
  
  // Print a payload contents to UART
  UART_SendStr("RCV PIPE#");
  UART_SendInt(pipe);
  UART_SendStr(" PAYLOAD:>");
  Toggle_LED();
  UART_SendBufHex((char *) nRF24_payload, payload_length);
  UART_SendStr("<\r\n");
}*/
  
  // подготовка данных
  jButton.uint8Message.gr1 = nRF24_payload[0];
  jButton.uint8Message.gr2 = nRF24_payload[1];
  jButton.uint8Message.gr3 = nRF24_payload[2];
  jButton.uint8Message.gr4 = nRF24_payload[3];
  jStickA.ValV = nRF24_payload[4];
  jStickA.ValV |= nRF24_payload[5]<<8;
  jStickA.ValG = nRF24_payload[6];
  jStickA.ValG |= nRF24_payload[7]<<8;
  jStickB.ValV = nRF24_payload[8];
  jStickB.ValV |=nRF24_payload[9]<<8;
  jStickB.ValG = nRF24_payload[10];
  jStickB.ValG |= nRF24_payload[11]<<8;
}

//движение
void vMove(){
  static uint32_t speedPrivod;
  uint32_t maxSpeedPrivod;  
  static uint32_t speedServo;
  uint16_t temp;
  
  //вперед
  if(jStickA.ValV > 0 || jButton.bit.up){
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    timer_Sleep=0;
  }
  //назад
  if(jStickA.ValV < 0 || jButton.bit.dawn){
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    timer_Sleep=0;
  }
  //стоп
  if(jStickA.ValV == 0 && jButton.bit.up == 0 && jButton.bit.dawn == 0 ){
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    speedPrivod = 0;
  }
  //скорости
  temp = jStickA.ValV < 0 ? -1*jStickA.ValV : jStickA.ValV;//abs(jStickA.ValV);
  if(jStickA.ValV > 10)jStickA.ValV = 0;
  if(jButton.bit.osn == 1)maxSpeedPrivod = SPEED_1;
  else if(jButton.bit.osn == 2)maxSpeedPrivod = SPEED_2;
  else if(jButton.bit.osn == 3)maxSpeedPrivod = SPEED_3;
  if(jButton.bit.up || jButton.bit.dawn){
    if(speedPrivod < maxSpeedPrivod)speedPrivod += 100;
    else speedPrivod = maxSpeedPrivod;
  }else speedPrivod = map((uint32_t)temp, 0, 2080, 0, maxSpeedPrivod);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, speedPrivod);
  //40000-1 = 39999
  
  //вправо
  if(jStickB.ValG > 0){
    speedServo = map((uint32_t)jStickB.ValG, 2100, 0, SERVO_WRITE, SERVO_CENTR);
    flagRight = 0;
    timer_Sleep=0;
  }
  if(jButton.bit.write){
    speedServo = SERVO_WRITE;
    flagRight = 0;
    timer_Sleep=0;
  }
  //влево
  if(jStickB.ValG < 0){
    temp = jStickB.ValG < 0 ? -1*jStickB.ValG : jStickB.ValG;//abs(jStickB.ValG);
    speedServo = map((uint32_t)temp, 0, 2100, SERVO_CENTR, SERVO_LEFT);
    flagLeft = 0;
    timer_Sleep=0;
  }
  if(jButton.bit.left){
    speedServo = SERVO_LEFT;
    flagLeft = 0;
    timer_Sleep=0;
  }
  //стоп
  if(jStickB.ValG == 0 && jButton.bit.write == 0 && jButton.bit.left == 0){
    speedServo = SERVO_CENTR;
  }
  //speedServo = 1700;
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, speedServo);
}

//мигание светодиода, медленное
uint8_t xToogleLedLow(uint8_t led){
  if(timer_LedLow > 500){
    timer_LedLow = 0;
    return led = ~led & 0x01;
  }
  return led;
}

//мигание светодиода 4, режим норма
void vToogleLedNrfNormal(){
  if(timer_Led4 > 3000){
    if(timer_Led4 > 3000 && timer_Led4 < 3010) led.nrfState = 1;
    if(timer_Led4 > 3100){
      timer_Led4 = 0;
      led.nrfState = 0;
    }
  }
}

//масштабирование
uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// мигалка на кабине
void vLedToogle(){
  if(isLedToogle){
    if(timer_LedToogle>250){
      timer_LedToogle = 0;
      led.toogleR = ~led.toogleR & 0x01;
      led.toogleL = ~led.toogleL & 0x01;
    }
  }else{
    led.toogleR = 0;
    led.toogleL = 0;}
}