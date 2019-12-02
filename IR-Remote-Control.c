#include <mega128.h>   
#include <delay.h>
 
void system_init();
 
unsigned int time_count = 0;
unsigned int edge=0; 
unsigned char flag = 0;  
unsigned char flag_count = 0;  
unsigned char IR_start=0;  
unsigned char IR_rx_data[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char IR_data;
unsigned long mask=0x01;
unsigned char S_flag = 0, old_flag = 0;

 
interrupt[EXT_INT4] void input(void)   // 입력 캡쳐 인터럽트
{
        edge = time_count; 
        time_count=0;       
        if(edge > 87 && edge < 93) // lead code           13.5ms   
                IR_start=1;
                          
        else if(IR_start==1) 
        {
                if(edge > 10 && edge < 13)//  0        1.125ms 
                {          
                        IR_rx_data[flag_count] &= ~mask;
                        mask=mask<<1;
                        flag++;
                }
                else if(edge > 21 && edge < 24)//  1        2.25ms 
                {     
                        IR_rx_data[flag_count] |= mask;
                        mask=mask<<1;
                        flag++;
                }
                if(flag == 8) 
                {
                        flag = 0;
                        mask = 0x01;
                        flag_count++;
                }
                if(flag_count == 4)// data를 모두 받으면.. 
                {             
                        if((IR_rx_data[2] ^ IR_rx_data[3]) == 0xFF)//data가 정확히 들어왔는가? 
                        {   
                                if(S_flag == 0)         
                                {
                                        old_flag = S_flag;
                                        S_flag = 1;     
                                }
                                else
                                {       
                                        old_flag = S_flag;
                                        S_flag = 0;       
                                }
                                IR_data = IR_rx_data[2];        //리모컨 수신 데이터 저장
                               
                                IR_start=0; 
                                mask=0x01;
                                flag_count = 0;
                                IR_rx_data[0] = 0x00;     
                                IR_rx_data[1] = 0x00;
                                IR_rx_data[2] = 0x00;
                                IR_rx_data[3] = 0x00;
                                flag = 0;
                        }
                        else {
                                IR_start=0; 
                                mask=0x01;
                                flag_count = 0;
                                IR_rx_data[0] = 0x00;     
                                IR_rx_data[1] = 0x00;
                                IR_rx_data[2] = 0x00;
                                IR_rx_data[3] = 0x00;
                                flag = 0;
                        }
                }       
        }
}
 
interrupt[TIM3_COMPA] void compa(void)       // 100us 인터럽트
{
        time_count++;                        // IR timer 
}
 
void main()
{
DDRE = 0x00;    //IR Rx Pin ( ICP3 )
DDRB = 0x6F;    //OC1A , OC1B PWM 출력
DDRA = 0xff;
 
system_init();

        while (1)
        {       
            switch(IR_data)
            {   
                case 0x11 :     //Number 0
                        PORTA=0x00;
                        break;
                case 0x04 :     //Number 1
                        PORTA=0x01;
                        break;
                case 0x05 :     //Number 2
                        PORTA=0x03;
                        break;
                case 0x06 :     //Number 3
                        PORTA=0x07;
                        break;
                case 0x08 :     //Number 4
                        PORTA=0x0f;
                        break;
                case 0x09 :     //Number 5
                        PORTA=0x1f;
                        break;
                case 0x0A :     //Number 6
                        PORTA=0x3f;
                        break;
                case 0x0C :     //Number 7
                        PORTA=0x7f;
                        break;
                default :       //그 외 키 입력 시
                        PORTA=0x00;
                        delay_ms(500);
                        PORTA=0xff;
                        delay_ms(500);
                        break;
            }             
        }       
}
                                                            
void system_init()                                          
{
 
TCCR3A=0x00;  //CTC mode() , 프리스케일 8
TCCR3B=0x8A;  //0xCA rising edge  0x8A falling edge
 
OCR3AH=0x00;
OCR3AL=199;   //16Mhz : 199 = 100u,
 
// f(PWM) = 8Mhz / 2 / 64 / 255 = 250Hz
TCCR1A = 0xA1;  //phase corect PWM(8bit) - TOP : 0x00FF
TCCR1B = 0x03;  //프리스케일 : 64
TCCR1C = 0X00;
 
// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x00;
ETIMSK=0x10;
 
EICRB = 0x02;   //INT4 falling edge interrupt
EIMSK = 0x10;   //INT4 interrupt enable  
SREG |= 0x80;
}
