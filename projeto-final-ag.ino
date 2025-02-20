#include <LiquidCrystal.h>

// Pin definitions
#define DISPENSER_LED_PIN 9
#define FOOD_LEVEL_LED_PIN 10
#define POTENTIOMETER_PIN A0
#define LCD_RS 22
#define LCD_EN 24
#define LCD_D4 26 
#define LCD_D5 28
#define LCD_D6 30
#define LCD_D7 32

// Objects
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Variables
unsigned long lastFeedTime = 0;
const unsigned long feedInterval = 21600000; // 6 hours in milliseconds
int foodAmount = 0;
const int maxFoodAmount = 100;

void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600); // UART communication with PIC

    // Initialize LEDs
    pinMode(DISPENSER_LED_PIN, OUTPUT);
    pinMode(FOOD_LEVEL_LED_PIN, OUTPUT);

    // Initialize LCD
    lcd.begin(16, 2);
    lcd.print("Cat Feeder Sim");

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

    // Check if it's time to feed
    if (currentTime - lastFeedTime >= feedInterval)
    {
        feedCat();
    }

    // Update LCD
    updateLCD(currentTime);

    // Read potentiometer for food amount
    foodAmount = map(analogRead(POTENTIOMETER_PIN), 0, 1023, 0, maxFoodAmount);

    // Update food level LED
    analogWrite(FOOD_LEVEL_LED_PIN, map(foodAmount, 0, maxFoodAmount, 0, 255));

    // Check for commands from PIC
    if (Serial1.available())
    {
        char command = Serial1.read();
        if (command == 'F')
        {
            feedCat();
        }
        else if (command == 'T')
        {
            // Receive time update from PIC
            while (Serial1.available() < 4)
                ;                                     // Wait for 4 bytes
            currentTime = Serial1.read() * 3600000UL; // Hours
            currentTime += Serial1.read() * 60000UL;  // Minutes
            currentTime += Serial1.read() * 1000UL;   // Seconds
            currentTime += Serial1.read() * 10UL;     // Tenths of a second
        }
    }
}

void feedCat()
{
    lcd.clear();
    lcd.print("Feeding...");

    // Simulate opening food dispenser
    digitalWrite(DISPENSER_LED_PIN, HIGH);
    delay(2000); // Simulate dispensing time
    digitalWrite(DISPENSER_LED_PIN, LOW);

    lastFeedTime = millis();
    lcd.clear();
    lcd.print("Fed: ");
    lcd.print(foodAmount);
    lcd.print(" units");

    // Reset food amount for next feeding
    foodAmount = 0;
}

void updateLCD(unsigned long currentTime)
{
    unsigned long seconds = currentTime / 1000;
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;

    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    if (hours < 10)
        lcd.print('0');
    lcd.print(hours);
    lcd.print(':');
    if (minutes < 10)
        lcd.print('0');
    lcd.print(minutes);

    lcd.setCursor(0, 1);
    lcd.print("Next: ");
    unsigned long nextFeed = lastFeedTime + feedInterval;
    hours = (nextFeed / 3600000) % 24;
    minutes = (nextFeed / 60000) % 60;
    if (hours < 10)
        lcd.print('0');
    lcd.print(hours);
    lcd.print(':');
    if (minutes < 10)
        lcd.print('0');
    lcd.print(minutes);
}

ISR(TIMER1_COMPA_vect)
{
    // This interrupt service routine runs every second
    // You can add code here if needed
}
