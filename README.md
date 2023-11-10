# F103_NRF_RX_PC

![Screnshot][(https://github.com/allexmak1/F103_NRF_RX_PC/blob/main/image/image.png)

ПО для радиоуправляемой модели.
Более подробные настройки в Inc/config.h

https://www.youtube.com/watch?v=npfDz2VAjL4

## Hardware 
 * STM32F103REV6 (72 MHz SYSCLK)
 * NRF модуль NRF24L 
 * драйверный мост для моторов
 * АКБ Li-Ion (3,7v),
 * зарядная плата

### Pins

|STM32F103REV6|Description |Info|
| ----------- |:----------:|	--------------:|
|PB12         | Led_Front  | передние фары  |
|PB13         | Led_Back   | задние фары    |
|PB14         | Led_Left   | поворотнфик левый  |
|PB15         | Led_Right   | поворотник правый  |
|PA8          |            |   |
|PA9          | UART_TX    | для отладки  |
|PA10         | UART_RT   |   |
|PA11         | Led_ToogleR   | мигалка  |
|PA12         | Led_ToogleL   | мигалка  |
|PA15         |            |   |
|PB3          | driver_IN1 |  ШИМ на мотор |
|PB4          | driver_IN2 |  ШИМ на мотор |
|PB5           |            |   |
|PB6           |            |   |
|PB7           |            |   |
|PB8           |            |   |
|PB9           |            |   |
|5V          |            |   |
|GND          |            |   |
|3V3          |            |   |
| ----------- |:----------:|	--------------:|
|GND          |            |   |
|GND          |            |   |
|3V3          |            |   |
|NRST          |            |   |
|PB11          |            |   |
|PB10          |            |   |
|PB1           | SPI1_CS           | NRF  |
|PB0           | SPI1_DT           | NRF  |
|PA7           | SPI1_MOSI           | NRF  |
|PA6           | SPI1_MISO           | NRF  |
|PA5           | SPI1_SCK           | NRF  |
|PA4           | bizzer_PWM           | сигнал  |
|PA3           | voltage_ADC           | напряжение на акб  |
|PA2           | driver_ENA_PWM           |   |
|PA1           |            |   |
|PA0           |            |   |
|PC15          |            |   |
|PC14          |            |   |
|PC13          |            |   |

## To-Do

 - [x] Закончить проект, протестироть
 - [ ] Отобразить заряд акб машины на пульте

## Resources

 allexmak, 2021

