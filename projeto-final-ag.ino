#include <LiquidCrystal.h>

// Pin Planner
#define FEEDING_LED 9
#define FOOD_LEVEL_LED_PIN_LOW 10
#define FOOD_LEVEL_LED_PIN_MEDIUM 11
#define FOOD_LEVEL_LED_PIN_HIGH 12
#define LDR1 A0
#define LDR2 A1
#define LDR3 A2

#define LCD_RS 22
#define LCD_EN 24
#define LCD_D4 26
#define LCD_D5 28
#define LCD_D6 30
#define LCD_D7 32

// Definição Display LCD
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Variables
unsigned long lastFeedTime = 0;
const unsigned long feedInterval = 60000; // 1 minuto para teste
int foodAmount = 0;
const int maxFoodAmount = 100;

void fadeLED(int, int, int, int, int);
void feedCat();
void updateFoodLevel();
void updateLCD(unsigned long);

void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600); // Comunicação serial com o PIC

    // Initialize LEDs
    pinMode(FEEDING_LED, OUTPUT);
    pinMode(FOOD_LEVEL_LED_PIN_LOW, OUTPUT);
    pinMode(FOOD_LEVEL_LED_PIN_MEDIUM, OUTPUT);
    pinMode(FOOD_LEVEL_LED_PIN_HIGH, OUTPUT);

    // Initialize LCD
    lcd.begin(16, 2);
    lcd.print("Kitty Feeder");

    // Set up Timer1 for 1 second interrupt
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 15624; // 16MHz / (256 * 1Hz) - 1
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
    interrupts();
}

void loop()
{
    static unsigned long currentTime = 0;

    
    if (Serial1.available() >= 3)
    {
        int hours = Serial1.read();
        int minutes = Serial1.read();
        int seconds = Serial1.read();

        updateLCD(hours, minutes, seconds);

        currentTime = (hours * 3600000UL) + (minutes * 60000UL) + (seconds * 1000UL);
    }

    updateLCD(millis() / 1000 / 60 / 60, (millis() / 1000 / 60) % 60, (millis() / 1000) % 60);

    updateFoodLevel();

    if (millis() - lastFeedTime >= feedInterval)
    {
        feedCat();
    }

    // Verifica comandos do PIC para alimentação
    if (Serial1.available())
    {
        char command = Serial1.read();
        if (command == 'F')
        {
            feedCat();
            Serial1.write('D');
        }
    }
}

void fadeLED(int pin, int start, int end, int step, int delayTime)
{
    if (start < end)
    {
        for (int i = start; i <= end; i += step)
        {
            analogWrite(pin, i);
            delay(delayTime);
        }
    }
    else
    {
        for (int i = start; i >= end; i -= step)
        {
            analogWrite(pin, i);
            delay(delayTime);
        }
    }
}

void feedCat()
{
    lcd.clear();
    lcd.print("Feeding...");

    fadeLED(FEEDING_LED, 0, 255, 5, 30);
    delay(2000);
    fadeLED(FEEDING_LED, 255, 0, 5, 30);

    lastFeedTime = millis();
    lcd.clear();
    lcd.print("Fed: ");
    lcd.print(foodAmount);
    lcd.print(" units");

    foodAmount = 0;

    Serial.write('D');
}

void updateFoodLevel()
{
    int ldr1 = analogRead(LDR1);
    int ldr2 = analogRead(LDR2);
    int ldr3 = analogRead(LDR3);

    int lumus = 500;
    bool covered1 = ldr1 < lumus;
    bool covered2 = ldr2 < lumus;
    bool covered3 = ldr3 < lumus;

    digitalWrite(FOOD_LEVEL_LED_PIN_LOW, LOW);
    digitalWrite(FOOD_LEVEL_LED_PIN_MEDIUM, LOW);
    digitalWrite(FOOD_LEVEL_LED_PIN_HIGH, LOW);

    // Atualiza foodAmount com base no nível detectado
    if (covered1 && !covered2 && !covered3)
    {
        foodAmount = 33;
        digitalWrite(FOOD_LEVEL_LED_PIN_LOW, HIGH);
    }
    else if (covered1 && covered2 && !covered3)
    {
        foodAmount = 66;
        digitalWrite(FOOD_LEVEL_LED_PIN_MEDIUM, HIGH);
    }
    else if (covered1 && covered2 && covered3)
    {
        foodAmount = 100;
        digitalWrite(FOOD_LEVEL_LED_PIN_HIGH, HIGH);
    }
    else
    {
        foodAmount = 0;
    }
}

void updateLCD(unsigned long currentTime)
{
    unsigned long seconds = currentTime / 1000;
    int hours = (seconds / 3600) % 24;
    int minutes = (seconds % 3600) / 60;

    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print((hours < 10) ? "0" : "");
    lcd.print(hours);
    lcd.print(':');
    lcd.print((minutes < 10) ? "0" : "");
    lcd.print(minutes);

    unsigned long nextFeed = lastFeedTime + feedInterval;
    unsigned long nextSeconds = nextFeed / 1000;
    int nextHours = (nextSeconds / 3600) % 24;
    int nextMinutes = (nextSeconds % 3600) / 60;

    lcd.setCursor(0, 1);
    lcd.print("Next: ");
    lcd.print((nextHours < 10) ? "0" : "");
    lcd.print(nextHours);
    lcd.print(':');
    lcd.print((nextMinutes < 10) ? "0" : "");
    lcd.print(nextMinutes);
}

ISR(TIMER1_COMPA_vect)
{
    // This interrupt service routine runs every second
    // You can add code here if needed
}
