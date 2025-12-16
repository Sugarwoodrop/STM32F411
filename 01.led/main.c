#include "stm32f4xx.h"

void delay (void){
	for (uint32_t i=0; i < 500000; i++)
		__asm("nop");
}

void GPIOC_Init (void);

int main (void) {

	GPIOC_Init();

	while (1) {
		GPIOC->ODR &= ~GPIO_ODR_OD13; // Output Data Register 0 → LED ON
		delay();
		GPIOC->ODR |= GPIO_ODR_OD13; // 1 → LED OFF
		delay(); 
	}
		
	return 0;
}

void GPIOC_Init (void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Запуск GPIO PORT C
	
	GPIOC->MODER &= ~(GPIO_MODER_MODE13_0 | GPIO_MODER_MODE13_1); // Зануляем PC13 - LED
	GPIOC->MODER |= GPIO_MODER_MODE13_0; // Делаем в режиме Output
}
