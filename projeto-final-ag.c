// --- Ligações entre PIC e LCD ---
sbit LCD_RS at RE2_bit; // PINO 2 DO PORTD LIGADO AO RS DO DISPLAY
sbit LCD_EN at RE1_bit; // PINO 3 DO PORTD LIGADO AO EN DO DISPLAY
sbit LCD_D7 at RD7_bit; // PINO 7 DO PORTD LIGADO AO D7 DO DISPLAY
sbit LCD_D6 at RD6_bit; // PINO 6 DO PORTD LIGADO AO D6 DO DISPLAY
sbit LCD_D5 at RD5_bit; // PINO 5 DO PORTD LIGADO AO D5 DO DISPLAY
sbit LCD_D4 at RD4_bit; // PINO 4 DO PORTD LIGADO AO D4 DO DISPLAY

// Selecionando direção de fluxo de dados dos pinos utilizados para a comunicação com display LCD
sbit LCD_RS_Direction at TRISE2_bit; // SETA DIREÇÃO DO FLUXO DE DADOS DO PINO 2 DO PORTD
sbit LCD_EN_Direction at TRISE1_bit; // SETA DIREÇÃO DO FLUXO DE DADOS DO PINO 3 DO PORTD
sbit LCD_D7_Direction at TRISD7_bit; // SETA DIREÇÃO DO FLUXO DE DADOS DO PINO 7 DO PORTD
sbit LCD_D6_Direction at TRISD6_bit; // SETA DIREÇÃO DO FLUXO DE DADOS DO PINO 6 DO PORTD
sbit LCD_D5_Direction at TRISD5_bit; // SETA DIREÇÃO DO FLUXO DE DADOS DO PINO 5 DO PORTD
sbit LCD_D4_Direction at TRISD4_bit; // SETA DIREÇÃO DO FLUXO DE DADOS DO PINO 4 DO PORTD

/*
// Button connections
sbit BUTTON1 at RB0_bit;
sbit BUTTON2 at RB1_bit;

// LED and Buzzer connections
sbit LED1 at RD0_bit;
sbit LED2 at RD1_bit;
sbit BUZZER at RC2_bit;
*/

// Variables
unsigned int seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 12;
bit feedingTime = 0;

// Function prototypes
void initUART();
void initTimer0();
void updateTime();
void displayTime();
void sendTimeToArduino();
void sendFeedCommand();

void main()
{
    ADCON1 = 0x0F; // Configure all pins as digital
    TRISB = 0;   // Set RB0 and RB1 as inputs
    TRISC = 0;   // Set PORTC as output
    TRISD = 0;   // Set PORTD as output

    LED1 = 0;
    LED2 = 0;
    BUZZER = 0;

    Lcd_Init();               // Initialize LCD
    Lcd_Cmd(_LCD_CLEAR);      // Clear display
    Lcd_Cmd(_LCD_CURSOR_OFF); // Cursor off

    initUART();
    initTimer0();

    Lcd_Out(1, 1, "Time:");

    while (1)
    {
        displayTime();
        updateTime();

        if (BUTTON1 == 0)
        {                  // Increment hours
            Delay_ms(200); // Debounce
            hours = (hours + 1) % 24;
            sendTimeToArduino();
        }

        if (BUTTON2 == 0)
        {                  // Increment minutes
            Delay_ms(200); // Debounce
            minutes = (minutes + 1) % 60;
            seconds = 0;
            sendTimeToArduino();
        }

        // Check if it's feeding time (6:00, 12:00, 18:00)
        if ((hours == 6 || hours == 12 || hours == 18) && minutes == 0 && seconds == 0)
        {
            feedingTime = 1;
            BUZZER = 1;
            sendFeedCommand();
        }
        else
        {
            feedingTime = 0;
            BUZZER = 0;
        }

        // Update status LEDs
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
    T0CON = 0x87; // Enable Timer0, 16-bit mode, 1:256 prescaler
    TMR0H = 0x3C; // Preload for 0.1 second interrupt
    TMR0L = 0xB0;
    INTCON.TMR0IE = 1; // Enable Timer0 interrupt
    INTCON.GIE = 1;    // Enable global interrupts
}

void updateTime()
{
    if (TMR0IF_bit)
    {
        TMR0IF_bit = 0; // Clear Timer0 interrupt flag
        TMR0H = 0x3C;   // Reload Timer0
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
    UART1_Write(0); // Tenths of a second (always 0 in this implementation)
}

void sendFeedCommand()
{
    UART1_Write('F');
}
