/*
 * lightshow.c
 * 
 * Copyright 2016  <tjohnsen@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#define TROUBLESHOOT 1  //Boolean to show debug statements    
#define TROUBLESHOOTHB 0  //Boolean to check heartbeat status
#define SECONDS 15      //seconds to run each show.
#include <stdio.h>
#include "Morse.h"
int isdigit(int);
int toupper(int);
size_t strlen(const char *);

byte ledPin[] = {13, 12, 11, 10, 9, 8, 7, 6};//, 5, 4};
int sensor = A3;

void setup() {
    //set all pins to output
    for (int i=0; i<sizeof(ledPin); i++)
    {
        pinMode(ledPin[i], OUTPUT);
    }
    #if TROUBLESHOOT
    Serial.begin(9600); //for troubleshooting
    #elif TROUBLESHOOTHB
    Serial.begin(9600); //for troubleshooting
    #endif
    //changeTime = millis();
    randomSeed(analogRead(0));
}

inline bool lt(int x, int y)
{
    return (x < y);
}

inline bool gt(int x, int y)
{
    return (x > y);
}

inline bool ul_lt(unsigned long x, unsigned long y)
{
    return (x < y);
}

inline bool ul_gt(unsigned long x, unsigned long y)
{
    return (x > y);
}

inline void ZeroPin(int pin)
{
    digitalWrite(ledPin[pin], LOW);
}

static inline void AllOff()
{
    for (int i=0; i<sizeof(ledPin); i++)
    {
        ZeroPin(i);
    }
}

static inline void AllOn()
{
    for (int i=0; i<sizeof(ledPin); i++)
    {
        digitalWrite(ledPin[i], HIGH); 
    }
}

inline void PinOp(int onOff, int pin)
{
    digitalWrite(ledPin[pin], onOff);
}

inline void BlinkPin(int pin, float rate)
{
    PinOp(HIGH, pin);
    delay(analogRead(sensor)*rate);
    PinOp(LOW, pin);
}

void IntToPin(int value)
{
    //Get a value and display the binary value
    for (int i=0; i<sizeof(ledPin); i++)
    {
        PinOp(value & (1<<i), i);
    }
}

inline void swap(int *a, int *b, int *direction)
{
    int temp;
    *direction *= -1;
    temp = *a;
    *a = *b;
    *b = temp;
}

void NPlusOne(int seconds)
{
    unsigned long then = millis();
    int MIN=0;
    int MAX=sizeof(ledPin);
    int direction=1;
    bool (*comparison)(int, int) = &lt;
    void (*ranger)(int) = &PinRangeOn;

    while(1)
    {
        for(int i=MIN; (*comparison)(i, MAX); i=i+direction)
        {
            ranger(i);
            if(timer(then, seconds, &ul_gt))
            return;
        }
        swap(&MIN, &MAX, &direction);
        MIN += direction;
        MAX += direction;
        comparison = (direction < 0) ? &gt : &lt;
        ranger = (direction < 0) ? &PinRangeOff : &PinRangeOn;
    }
}

void PinRangeOn(int pindex)
{
    for(int i=0; i<sizeof(ledPin); i+=(pindex+1))
    {
        for(int j=i; j<(i+pindex+1) && j<sizeof(ledPin); j++)
        {
            PinOp(1, j);
            delay(analogRead(sensor));
        }
        AllOff();
    }
}

void PinRangeOff(int pindex)
{
    for (int i=sizeof(ledPin)-1; i >= 0; i-= (pindex+1))
    {
        for (int j=i; j>(i-pindex-1) && j>=0; j--)
        {
            PinOp(1, j);
            delay(analogRead(sensor));
        }
        AllOff();
    }
}

void BinaryCounter(unsigned long seconds, bool countDown)
{
    //binary counter
    //arguments:  seconds to run, option to count down.
    int i;
    int MIN = 0;
    int MAX = 1 << sizeof(ledPin);
    int direction = 1;  //will be -1 when counting down
    unsigned long then = millis();
    bool (*comparison)(int, int) = &lt;
    
    while (1)   //will break when time requirement met
    {
        for (i=MIN; (*comparison)(i,MAX); i=i+direction)
        {
            IntToPin(i);
            delay(analogRead(sensor));
            if (timer(then, seconds, &ul_gt)) //break option since this loop can go way past the argument
            {
                return;
            }
        }
        delay(750);
        if (countDown)
        {
            direction *= -1;
            comparison = (direction < 0) ? &gt : &lt;
            i = MIN;
            MIN = MAX + direction;  // new max will be min - 1 (or back to original)
            MAX = i + direction;  // same with min
        }
    }
}//*/

void PotFlip(unsigned long seconds)
{
    int count=0;
    int x = 0;  //off pins
    int y = 1;  //on pins ... initially
    
    unsigned long now = millis();
    
    while((millis() - now) < (seconds * 1000))
    {
        x ^= 1; //flip status
        y ^= 1; //flip status
        for (int i=0; i<sizeof(ledPin); i++)
        {
            PinOp((i&1) ? x : y, i);
        }
        delay(analogRead(sensor));
    }
}

void StackOnL()
{
    int MAX = sizeof(ledPin);
    int MIN = 0;
    for (int top=MAX; top>MIN; top--)
    {
        for(int pin=0; pin < top; pin++)
        {
            PinOp(1, pin);
            delay(analogRead(sensor));
            if (pin != (top-1))
            {
                PinOp(0, pin);  //don't turn off the last one
            }
        }
    }
}
        
void StackOffL()
{
    int MAX = sizeof(ledPin);
    for (int top=0; top<MAX; top++)
    {
        for(int pin=top; pin > -1; pin--)
        {
            PinOp(1, pin);
            delay(analogRead(sensor));
            PinOp(0, pin);
        }
    }
}

void StackOnR()
{
    int MAX = sizeof(ledPin);
    for (int top=0; top<MAX; top++)
    {
        for(int pin=MAX-1; pin > top-1; pin--)
        {
            PinOp(1, pin);
            delay(analogRead(sensor));
            if (pin != top)
            {
                PinOp(0, pin);  //don't turn off the last one
            }
        }
    }
}

void StackOffR()
{
    int MAX = sizeof(ledPin);
    int MIN = 0;
    for (int top=MAX-1; top>MIN-1; top--)
    {
        for(int pin=top; pin < MAX; pin++)
        {
            PinOp(1, pin);
            delay(analogRead(sensor));
            PinOp(0, pin);
        }
    }
}

void Stack(unsigned long seconds)
{
    int count = 0;
    unsigned long start = millis();
    
    while (timer(start, seconds, &ul_lt))
    {
        switch(count)
        {
            case 0:
                StackOnL();
                break;
            case 1:
                StackOffL();
                break;
            case 2:
                StackOnR();
                break;
            case 3:
                StackOffR();
        }
        count++;
        count %= 4;
    }
}

void Linear(int seconds)
{
    unsigned long then = millis();
    int i = 0;
    
    while(timer(then, seconds, &ul_lt))
    {
        BlinkPin(i++ % sizeof(ledPin),1);
        /*PinOp(1, i % sizeof(ledPin));
        delay(analogRead(sensor));
        PinOp(0, i % sizeof(ledPin));*/
    }
}

void PingPong(int seconds)
{
    float rate_pin = .1; //rate per pin
    unsigned long then = millis();
    int MIN = 0;
    int MAX = sizeof(ledPin)-1;
    int i=MIN;  //for swapping as well
    int direction = 1;  //+1 counts up, -1 counts down
    bool (*comparison)(int, int) = &lt;
    
    while(timer(then, seconds, &ul_lt))
    {
        for (; (*comparison)(i,MAX); i+=direction)
        {
            BlinkPin(i, rate_pin);
        }
        PinOp(1, MAX);
        delay(analogRead(sensor));   //sleep longer for end
        direction *= -1;    //switch directions
        comparison = (direction < 0) ? &gt : &lt;
        i = MAX;
        MAX = MIN;
        MIN = i;
    }
}

inline bool timer(unsigned long then, int seconds, bool (*comparison)(unsigned long, unsigned long))
{
    return ((*comparison)(millis() - then, seconds*1000));
}

void Bogo(int seconds)
{
    unsigned long then = millis();
    while (timer(then, seconds, &ul_lt))
    {
        for (int i=0; i<sizeof(ledPin); i++)
        {
            PinOp((int)random(0,2), i);
        }
        delay(analogRead(sensor));
    }
}

void LetterFlash(char letter)
{
    char index, i;
    if (isdigit(letter))
    {
        index = 26 + letter - '0';
    }
    else if (isalpha(letter))
    {
        index = toupper(letter) - 'A';
    }
    else
    {
        return; //leave since theres nothing to write
    }
    for (i=0; i<strlen(MorseCode[index]); i++)
    {
        AllOn();
        if(MorseCode[index][i] == '.')
        {
            delay(DIT * analogRead(sensor)*12);
        }
        else if (MorseCode[index][i] == '-')
        {
            delay(DAH * analogRead(sensor)*12);
        }
        AllOff();
        delay(DIT * analogRead(sensor)*12);  //delay between each char
        //printf("%c\n", MorseCode[index][i]);
    }
}

void MorseFlash(char * sentence)
{
    for(int i=0; i<strlen(sentence); i++)
    {
        if (isalnum)
        {
            LetterFlash(sentence[i]);
        }
        else
        { //most likely a space, delay otherwise
            delay(((DAH*2)+DIT) * analogRead(sensor)*12);
        }
        delay(DAH * analogRead(sensor)*12);
    }
}

void HeartBeat(int seconds)
{
    #if TROUBLESHOOTHB
    unsigned long totalpasses = 0;
    #endif
    #define HBRATIO 75
    //int rate = time in seconds for each (double) pump
    byte MIN = 0x40;
    byte MAX = 0xFF;
    char direction = 5; //char can be negative
    bool (*comparison)(int, int) = &lt;
    //byte temp;  //for swapping
    int i=0;   //for loops
    unsigned long then = millis();
    unsigned int mainPulse = (MAX - MIN) / direction;
    unsigned int fadePulse = MAX / direction;
    unsigned int totalPulse = (mainPulse * 2) + (fadePulse * 2);  //total number of passes for light ops
    float pulseRate = (1000 / 2) / totalPulse; //length of delay for each pass to take up rate
    pulseRate++;  //make up for lost decimal points, take sides with more time
    int count = 0;
    int sCase;
    
    while (timer(then, seconds, &ul_lt))
    {
        sCase = count % 5;
        #if TROUBLESHOOTHB
        //Serial.println(count, DEC);
        //Serial.println(times, DEC);
        #endif
        switch(sCase)
        {
            case 0:
                //ramp up from zero on first pass
                for (; i<MIN; i=i+direction)
                {
                    analogWrite(6,  i);
                    analogWrite(9,  i);
                    analogWrite(10, i);
                    analogWrite(11, i);
                    delay((analogRead(sensor)/HBRATIO)*pulseRate);
                    #if TROUBLESHOOTHB
                    totalpasses++;
                    #endif
                    
                }
                MIN = 0x40;
                break;
            case 3:
                MAX = 0;
                break;
        }
        if ((sCase) < 4)  //only pulse for the first half
        {
            for (; (*comparison)(i,MAX); i=i+direction)
            {
                analogWrite(6,  i);
                analogWrite(9,  i);
                analogWrite(10, i);
                analogWrite(11, i);
                delay(analogRead(sensor)/HBRATIO*pulseRate);
                #if TROUBLESHOOTHB
                totalpasses++;
                #endif  
            }
            direction = -direction;    //switch directions
            comparison = (direction < 0) ? &gt : &lt;
            i = MAX;
            MAX = MIN;
            MIN = i;
        }
        else
        {
            AllOff();
            //delay((rate*1000)/2);
            delay(analogRead(sensor)/HBRATIO*500);
            #if TROUBLESHOOTHB
            if ((sCase) == 4)
            {
              Serial.println(totalpasses, DEC);
              totalpasses=0;
            }
            #endif
        }
        count++;

    }
}

void loop()
{
      #if TROUBLESHOOT
      unsigned long t1;
      unsigned long t2;
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    HeartBeat(SECONDS);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("HeartBeat took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    BinaryCounter(SECONDS, true);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("BinaryCounter took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    PotFlip(SECONDS);//*/
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("Flipper took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    Stack(SECONDS);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("Stack took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    Linear(SECONDS);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("Linear took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    PingPong(SECONDS);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("PingPong took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    Bogo(SECONDS);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("Bogo took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    MorseFlash("cq cq de m6tfj");
    //MorseFlash("cq cq de m6tfj m6tfj merry xmas k");
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("Morse took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
    NPlusOne(SECONDS);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("N+1 took:");
      Serial.println((t2-t1), DEC);
      Serial.println("Potentiometer level:");
      Serial.println(analogRead(sensor), DEC);
      t1 = millis();
      #endif
}
