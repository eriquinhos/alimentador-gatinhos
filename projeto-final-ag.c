// --- Ligações entre PIC e LCD ---
sbit LCD_RS at RE2_bit;
sbit LCD_EN at RE1_bit;
sbit LCD_D7 at RD7_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D4 at RD4_bit;

sbit LCD_RS_Direction at TRISE2_bit;
sbit LCD_EN_Direction at TRISE1_bit;
sbit LCD_D7_Direction at TRISD7_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD4_bit;

// Botões para ajuste manual do tempo
sbit BUTTON1 at RB0_bit;
sbit BUTTON2 at RB1_bit;

// LED e Buzzer
sbit LED1 at RD0_bit;
sbit LED2 at RD1_bit;
sbit BUZZER at RC2_bit;

// Variáveis de tempo
unsigned int seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 12;
bit feedingTime = 0;

// Protótipos das funções
void initUART();
void initTimer0();
void updateTime();
void displayTime();
void sendTimeToArduino();
void sendFeedCommand();

void main()
{
    ADCON1 = 0x0F; // Configura todos os pinos como digitais
    TRISB = 0xFF;  // Define PORTB como entrada (botões)
    TRISC = 0;     // Define PORTC como saída (buzzer)
    TRISD = 0;     // Define PORTD como saída (LEDs e LCD)

    LED1 = 0;
    LED2 = 0;
    BUZZER = 0;

    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);

    initUART();
    initTimer0();

    Lcd_Out(1, 1, "Time:");

    while (1)
    {
        displayTime();
        updateTime();

        // Ajusta manualmente a hora
        if (BUTTON1 == 0)
        {
            Delay_ms(200);
            hours = (hours + 1) % 24;
            sendTimeToArduino();
        }
        if (BUTTON2 == 0)
        {
            Delay_ms(200);
            minutes = (minutes + 1) % 60;
            seconds = 0;
            sendTimeToArduino();
        }

        // Alimentação nos horários programados
        if ((hours == 6 || hours == 12 || hours == 18) && minutes == 0 && seconds == 0)
        {
            feedingTime = 1;
            BUZZER = 1;
            sendFeedCommand();
        }

        // Verifica se o Arduino respondeu
        if (UART1_Data_Ready())
        {
            char received = UART1_Read();
            if (received == 'D')
            {
                BUZZER = 0; // Desliga o buzzer após a confirmação da alimentação
            }
        }

        // Atualiza LEDs
        LED1 = feedingTime;
        LED2 = !feedingTime;
    }
}

void initUART()
{
    UART1_Init(9600);
    Delay_ms(100);
}

void initTimer0()
{
    T0CON = 0x87; // Ativa Timer0, modo 16-bit, prescaler 1:256
    TMR0H = 0x3C;
    TMR0L = 0xB0;
    INTCON.TMR0IE = 1;
    INTCON.GIE = 1;
}

void updateTime()
{
    if (TMR0IF_bit)
    {
        TMR0IF_bit = 0;
        TMR0H = 0x3C;
        TMR0L = 0xB0;

        seconds++;
        if (seconds >= 60)
        {
            seconds = 0;
            minutes++;
            if (minutes >= 60)
            {
                minutes = 0;
                hours = (hours + 1) % 24;
            }
        }
    }
}

void displayTime()
{
    char timeStr[9];
    sprintf(timeStr, "%02u:%02u:%02u", hours, minutes, seconds);
    Lcd_Out(1, 7, timeStr);
}

void sendTimeToArduino()
{
    UART1_Write(hours);
    UART1_Write(minutes);
    UART1_Write(seconds);
}

void sendFeedCommand()
{
    UART1_Write('F');
}
