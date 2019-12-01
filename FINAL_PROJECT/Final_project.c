#include <tm4c123gh6pm.h>


void SysTick_Init(void);
void SysTick_Wait10ms(uint32_t delay);
void PORTD_Init(void);
void PORTA_Init(void);
void delayMs( unsigned int dTime );
void PORTB_init(void);
void GATE_ON(uint32_t count);
void GATE_OFF(uint32_t count);
void GPIOPortB_Handler(void);
void EnableInterrupts(void);
uint32_t count=0; // used to count the step for every cycle of fonction
uint8_t Data = 0x00; // just an initialization
uint8_t STATE = 1;
uint8_t ON = 0;
uint8_t OFF = 1;

struct State{
uint8_t Out;
const struct State *Next;
uint32_t Delay;
};
typedef const struct State StateType;
typedef StateType *StatePtr;
StateType fsm[4]={
  {10,&fsm[1],2},
  { 9,&fsm[2],2},
  { 5,&fsm[3],2},
  { 6,&fsm[0],2}
};
StateType fsm2[4]={
  {5,&fsm2[1],2},
  {6,&fsm2[2],2},
  {10,&fsm2[3],2},
  {9,&fsm2[0],2}
};
const struct State *Pt; // Current State
#define STEPPER (*((volatile uint32_t *)0x4000703C))
int main(void){

    PORTA_Init();
	  PORTD_Init();
	  PORTB_init();
	  GPIO_PORTA_DATA_R |= 0x06;

    while(1){
      if(STATE == ON){
				GATE_ON(count);
				
			}
			else if( STATE == OFF ){
				
				GATE_OFF(count);
			}
     }
}

 

void PORTD_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x08; // 1) port D clock enabled
	GPIO_PORTD_AMSEL_R &= ~0x0F; // 3) disable analog function
	GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) GPIO
  GPIO_PORTD_DIR_R |= 0x0F; // 5) make PD3-0 out
  GPIO_PORTD_AFSEL_R &= ~0x0F;// 6) disable alt func on PD3-0
  GPIO_PORTD_DR8R_R |= 0x0F; // enable 8 mA drive on PD3-0
  GPIO_PORTD_DEN_R |= 0x0F; // 7) enable digital I/O on PD3-0
}
void PORTA_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x01;
	GPIO_PORTA_DIR_R |= 0x06;
	GPIO_PORTA_DEN_R |= 0x06;
}

void delayMs( unsigned int dTime )
{
  unsigned int i ;
  SYSCTL_RCGCTIMER_R |= 1;
	TIMER0_CTL_R = 0;
  TIMER0_CFG_R = 0x04;
  TIMER0_TAMR_R = 0x02;
  TIMER0_TAILR_R = 16000 - 1 ;
  TIMER0_ICR_R = 0x1;
  TIMER0_CTL_R |= 0x01;

  for ( i = 0; i < dTime ; i++) {
    while ( ( TIMER0_RIS_R & 0x1 ) == 0 ){}
    TIMER0_ICR_R = 0x1;
 }
}
void PORTB_init(void) {

	SYSCTL_RCGCGPIO_R |= 0x02; // 1) activate Port B
	GPIO_PORTB_DIR_R &= ~0x01; // input PB0 is used as sensor;
	GPIO_PORTB_DEN_R |= 0x01;  // digital enabled
	
		// USED Interrupt
	GPIO_PORTB_IS_R &= ~0x01; 
	GPIO_PORTB_IBE_R &= ~0x01; 
	GPIO_PORTB_IEV_R |= 0x01; 
	GPIO_PORTB_ICR_R = 0x01; 
	GPIO_PORTB_IM_R |= 0x01; 
	NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;; 
	NVIC_EN0_R = 0x00000002; 
	
	EnableInterrupts();
}
void GPIOPortB_Handler(void){
	GPIO_PORTB_ICR_R = 0x01;
	Data = GPIO_PORTB_DATA_R;
	
	if(STATE == OFF){
		if(Data == 0x01){
			STATE = ON;
		}
	}
	else	if(STATE == ON){
		
		if(Data != 0x01 && count != 2000){
			STATE = OFF;
			count=0;
		}
	}
}
void GATE_ON(uint32_t G){
	
	Pt = &fsm2[0];
	while(G > 0){
		
		STEPPER = Pt->Out; // step motor
    delayMs (Pt->Delay);
    Pt = Pt->Next; // circular linked list
		G--;
	}
	count=G;
	delayMs(5000);
	STATE = OFF;
	Pt=&fsm2[0];
}
void GATE_OFF( uint32_t C){

	Pt = &fsm[0];
	while(C < 2000){
		
		STEPPER = Pt->Out; // step motor
    delayMs (Pt->Delay);
    Pt = Pt->Next; // circular linked list
		C++;
		
	}
	count = C;
	Pt=&fsm[0];
}
