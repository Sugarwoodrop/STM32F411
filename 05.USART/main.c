/*
Программа отправляет строку на PA2 и принемает её на PA3 и если символ меньше 0x35 то включаем диод иначе выклычаем и между чтением символов делей. Читаем всю принятую строку.
*/
#include "stm32f4xx.h"

void delay (void){
	for (uint32_t i=0; i < 500000; i++)
		__asm("nop");
}

void GPIOC_Init (void);
void USART2_Init(void);
void USART2_RX_Init(void);
void USART2_TX_Init(void);

void USART2_Tx_Char(uint8_t c);
void USART2_Tx_String(char *s);

void USART2_Rx_Data(void);
int main (void) {

	GPIOC_Init();
	USART2_Init();
	
	while (1) {
		USART2_Tx_String("Hello World! \r\n");
		
		USART2_Rx_Data();
	}
		
	return 0;
}

void GPIOC_Init (void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Запуск GPIO PORT C
	
	GPIOC->MODER &= ~(GPIO_MODER_MODE13_0 | GPIO_MODER_MODE13_1); // Зануляем PC13 - LED
	GPIOC->MODER |= GPIO_MODER_MODE13_0; // Делаем в режиме Output
	GPIOC->ODR ^= GPIO_ODR_OD13; //выключаем лампочку
}

void USART2_Init(void){ //USART2_TX - PA2,PD5 USART2_RX - PA3,PD6
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Запуск USART2
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Запуск GPIO PORT A
	
	USART2_RX_Init();
	USART2_TX_Init();
}

void USART2_RX_Init(void){
	GPIOA->MODER &= ~(GPIO_MODER_MODE3_0 | GPIO_MODER_MODE3_1); //Зануляем PA3
	GPIOA->MODER |= GPIO_MODER_MODE3_1; //Ставить на Alternate function mode
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL3; // зануляем выбор альтернативной функции
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_2; //выбераем альтернативную функцию AF7 USART_RX
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3_0 | GPIO_OSPEEDER_OSPEEDR3_1;
	
	USART2->CR1|= USART_CR1_OVER8|USART_CR1_RE;//USART_CR1_OVER8 → включаем 8-кратное over-sampling (обычно 16×). Это влияет на формулу расчёта скорости. OVER8 = 1 → USART делит частоту на 8 вместо 16 для расчёта BRR.
																						//USART_CR1_RE, включаем приём.
	// See RM0383 page 518-527
	// HSI=16MHz, FPCLK=16MHz, Baud rate = 115200 (Actual 115108), OVER8=1 (DIV_Fraction[2:0] bits)
	// Tx/Rx baud = fCK/(8*(2-OVER8)*USARTDIV)
	// 115200 = 16000000/8*USARTDIV
	// USARTDIV = 16000000/(8*115108)= 17.375
	// Mantisa = 17
	// If OVER8=1, fraction * 8
	// Fraction = 8*0.375= 3 
	
	//Рассчитываем делитель BRR
	USART2->BRR &=~USART_BRR_DIV_Mantissa;// Зануляем
	USART2->BRR|=(17 << USART_BRR_DIV_Mantissa_Pos); //Mantissa — целая часть делителя
	USART2->BRR &=~USART_BRR_DIV_Fraction; // Зануляем
	USART2->BRR|=(3 << USART_BRR_DIV_Fraction_Pos);//Fraction — дробная часть
	//Если OVER8=1, дробь умножается на 8 → 0.375*8 = 3.

}

void USART2_TX_Init(void){
	GPIOA->MODER &= ~(GPIO_MODER_MODE2_0 | GPIO_MODER_MODE2_1); //Зануляем PA2
	GPIOA->MODER |= GPIO_MODER_MODE2_1; //Ставить на Alternate function mode
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL2; // зануляем выбор альтернативной функции
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_0 | GPIO_AFRL_AFSEL2_1 | GPIO_AFRL_AFSEL2_2; //выбераем альтернативную функцию AF7 USART_TX
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2_0 | GPIO_OSPEEDER_OSPEEDR2_1; //Ставим скорость на fast. Чем выше скорость пина, тем круче фронты сигнала, меньше «размазывания»
	
	USART2->CR1|= USART_CR1_OVER8|USART_CR1_TE;
																						//USART_CR1_TE → Transmit Enable, включаем передачу.
	// See RM0383 page 518-527
	// HSI=16MHz, FPCLK=16MHz, Bps = 115200 (Actual 115108), OVER8=1 
	// Tx/Rx baud = fCK/(8*(2-OVER8)*USARTDIV)
	// 115108 = 16000000/(8*USARTDIV)
	// USARTDIV = 16000000/(8*115108)= 17.375
	// Mantisa = 17
	// If OVER8=1, fraction * 8
	// Fraction = 8*0.375= 3 
		
	//Рассчитываем делитель BRR
	USART2->BRR &=~USART_BRR_DIV_Mantissa;// Зануляем
	USART2->BRR|=(17 << USART_BRR_DIV_Mantissa_Pos); //Mantissa — целая часть делителя
	USART2->BRR &=~USART_BRR_DIV_Fraction; // Зануляем
	USART2->BRR|=(3 << USART_BRR_DIV_Fraction_Pos);//Fraction — дробная часть
	//Если OVER8=1, дробь умножается на 8 → 0.375*8 = 3.
	
	// USART1 Enable
	USART2->CR1|= USART_CR1_UE;				
}

void USART2_Tx_Char(uint8_t c){
	while (!(USART2->SR & USART_SR_TXE));
	USART2->DR=(c);
	}

void USART2_Tx_String(char *s)	{
	while(*s!='\0')
	{
	USART2_Tx_Char(*s);
	s++;
		}
	}

void USART2_Rx_Data(void){
	 while (USART2->SR & USART_SR_RXNE) {
        uint8_t d = USART2->DR;

        if (d < 0x35)
            GPIOC->ODR &= ~GPIO_ODR_OD13;
        else
            GPIOC->ODR |= GPIO_ODR_OD13;
				
			delay();
    }
}
	
