	/*
		Программа включает и выключает светодиод по кнопке. Если кнопка отпустилась то таймер сбрасывается и выключается.
	*/
	#include "stm32f4xx.h"
	
 volatile uint8_t wait_stat = 0;
	
	void GPIOC_Init (void);
	void TIME2_Init(void);

	int main (void) {
		__disable_irq(); // Запрещяем прерывания
		
		GPIOC_Init();
		TIME2_Init();
		
		__enable_irq(); // Разрешаем прерывания
		while (1) {
			
		}
			
		return 0;
	}

	void GPIOC_Init (void) {
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Запуск GPIO PORT C
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Запуск GPIO PORT A
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
		
		GPIOA->MODER &= ~(GPIO_MODER_MODE0_0 | GPIO_MODER_MODE0_1); // PA0 на режим Input
		GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD0_1 | GPIO_PUPDR_PUPD0_0); // Зануляем PUPDR
		GPIOA->PUPDR |= GPIO_PUPDR_PUPD0_0; // Pull-up
		
		GPIOC->MODER &= ~(GPIO_MODER_MODE13_0 | GPIO_MODER_MODE13_1); // Зануляем PC13 - LED
		GPIOC->MODER |= GPIO_MODER_MODE13_0; // Делаем в режиме Output
		GPIOC->ODR ^= GPIO_ODR_OD13;
		
		SYSCFG->EXTICR[0]&=~ SYSCFG_EXTICR1_EXTI0;  //Привязывает EXTI0 к ножке PA0 (очищает бит выбора порта).
		EXTI->IMR|= EXTI_IMR_MR0; // Разрешает прерывание EXTI0
		EXTI->FTSR|= EXTI_FTSR_TR0; //Прерывание срабатывает на нажатие
		EXTI->RTSR |= EXTI_RTSR_TR0; //Прерывание срабатывает на отпускание
		
		NVIC_SetPriority(EXTI0_IRQn, 1); //Устанавливает приоритет EXTI0 (меньше = выше)
		NVIC_EnableIRQ(EXTI0_IRQn); // Включает прерывание в NVIC, чтобы процессор реагировал на событие.
	}

	void TIME2_Init(void){
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
		
		TIM2->PSC= 1600-1; // Устанавливаем частоту fтаймера = fшины/PSC+1 = 10000Hz
		TIM2->ARR= 500; // Устанавливаем автоматический сброс регистр(время до сробатывания)
		TIM2->CNT=0; // Начало отсчёта, после первого же переполнения всегда становится 0
		TIM2->CR1 &= ~TIM_CR1_DIR	; //DIR = Direction → направление счёта 0	Upcounting (счётчик идёт 0 → ARR) 1	Downcounting (счётчик идёт ARR → 0)
		
		TIM2->DIER|=TIM_DIER_UIE; //Разрешает прерывание по обновлению таймера (UIE = Update Interrupt Enable).
		NVIC_SetPriority(TIM2_IRQn, 2); //Приоритет таймера ниже, чем у EXTI (1 < 2).
		NVIC_EnableIRQ(TIM2_IRQn); // Приоритет таймера ниже, чем у EXTI (1 < 2).
		TIM2->CR1 &= ~TIM_CR1_CEN; // Таймер не считает, пока мы его явно не включим.
		
		//Формула времени tтаймера = (ARR+1)/(fтай	мера)
	}

	void TIM2_IRQHandler(void){
		TIM2->SR &= ~TIM_SR_UIF;  // Сброс флага
		if(wait_stat == 1){
			if (!(GPIOA->IDR & GPIO_IDR_ID0)) {
				// Кнопка все еще нажата - это реальное нажатие
				GPIOC->ODR ^= GPIO_ODR_OD13; 
			}
		}
		wait_stat = 0;
		
		// Включаем обратно EXTI прерывание
		EXTI->IMR |= EXTI_IMR_MR0;
					
		// Останавливаем таймер
		TIM2->CR1 &= ~TIM_CR1_CEN;
		TIM2->CNT = 0;
	}

	void EXTI0_IRQHandler(void) {		
		if (!(GPIOA->IDR & GPIO_IDR_ID0)) {
			// кнопка нажата
			if(wait_stat == 0){
				EXTI->IMR &= ~EXTI_IMR_MR0; // Блокирует новое прерывание по кнопке, чтобы не было дребезга.
				TIM2->CNT = 0; // Сбрасывает счётчик таймера в 0, чтобы отсчёт начался с начала.
				TIM2->CR1 |= TIM_CR1_CEN; // Запускает таймер.
			}
			wait_stat = 1;
		} 
		else {
			// кнопка отпущена значит сбрасываем таймер
			wait_stat = 0;
		}
		EXTI->PR |= EXTI_PR_PR0;  //Сбрасывает флаг прерывания EXTI, чтобы процессор не думал, что прерывание ещё активно.
	}
