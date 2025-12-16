#include "stm32f4xx.h"

void delay (void){
	for (uint32_t i=0; i < 500000; i++)
		__asm("nop");
}

void GPIOC_Init (void);
void TIME2_Init(void);

int main (void) {

	GPIOC_Init();
	TIME2_Init();
	
	while (1) {
		while (!(TIM2->SR & TIM_SR_UIF)) {} //пока таймер не сработал ничего не делаем
		TIM2->SR &=~TIM_SR_UIF; // Обнуляем флаг срабатывания таймера
			
		GPIOC->ODR ^= GPIO_ODR_OD13; // XOR для led
	}
		
	return 0;
}

void GPIOC_Init (void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Запуск GPIO PORT C
	
	GPIOC->MODER &= ~(GPIO_MODER_MODE13_0 | GPIO_MODER_MODE13_1); // Зануляем PC13 - LED
	GPIOC->MODER |= GPIO_MODER_MODE13_0; // Делаем в режиме Output
}

void TIME2_Init(void){
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	
	TIM2->PSC= 1600-1; // Устанавливаем частоту fтаймера = fшины/PSC+1 = 10000Hz
	TIM2->ARR= 5000; // Устанавливаем автоматический сброс регистр(время до сробатывания)
	TIM2->CNT=0; // Начало отсчёта, после первого же переполнения всегда становится 0
	TIM2->CR1 |= TIM_CR1_CEN; // Counter Enable → включение счётчика CEN = 1 → таймер начинает считать CNT, 
	TIM2->CR1 &= ~TIM_CR1_DIR	; //DIR = Direction → направление счёта 0	Upcounting (счётчик идёт 0 → ARR) 1	Downcounting (счётчик идёт ARR → 0)
	
	//Формула времени tтаймера = (ARR+1)/(fтаймера)
}
