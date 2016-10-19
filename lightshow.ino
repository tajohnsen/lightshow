/*
 * MIT License
 * 
 * Copyright (c) 2016 Timothy Johnsen
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define TROUBLESHOOT 0  //Boolean to show debug statements    
#define TROUBLESHOOTHB 0  //Boolean to check heartbeat status
#define SECONDS 30      //seconds to run each show.
#include "Morse.h"

int isdigit(int);
int toupper(int);
size_t strlen(const char *);

/* array of pins used in this project.
 * This order sets each PWM capable pin (on most Arduinos) to every other one. */
byte ledPin[] = {13, 11, 12, 10, 8, 9, 7, 6};//, 4, 5, 2, 3}; 
byte ledPinSize = sizeof(ledPin);
int sensor = A3;  // potentiometer analog sensor

void setup() {
    //set all pins to output
    for (int i=0; i<ledPinSize; i++)
    {
        pinMode(ledPin[i], OUTPUT);
    }
    #if TROUBLESHOOT
    Serial.begin(9600); //for troubleshooting
    #elif TROUBLESHOOTHB
    Serial.begin(9600); //for troubleshooting
    #endif
    // use analog0 to seed the random timer
    // ensure A0 is empty!
    randomSeed(analogRead(0));
}

//takes a previous time, a time in seconds that's the goal, and compares it using a comparison function
inline bool timer(unsigned long then, unsigned long seconds, bool (*comparison)(unsigned long, unsigned long))
{
    return ((*comparison)(millis() - then, seconds*1000));
}

// boolean comparison of integers.  is x < y?
// used for reversing the direction of loops
inline bool lt(int x, int y)
{
    return (x < y);
}

// boolean comparison of integers.  is x > y?
// used for reversing the direction of loops
inline bool gt(int x, int y)
{
    return (x > y);
}

// boolean comparison of unsigned longs.  is x < y?
// used for reversing the direction of loops
inline bool ul_lt(unsigned long x, unsigned long y)
{
    return (x < y);
}

// boolean comparison of unsigned longs.  is x > y?
// used for reversing the direction of loops
inline bool ul_gt(unsigned long x, unsigned long y)
{
    return (x > y);
}

// simple function to turn off a specific pin.
// int pin refers to the index of pin in the led array
inline void ZeroPin(int pin)
{
    digitalWrite(ledPin[pin], LOW);
}

// simple function to turn off all LEDs
inline void AllOff()
{
    for (int i=0; i<ledPinSize; i++)
    {
        ZeroPin(i);
    }
}

//simple function to turn on all LEDs
inline void AllOn()
{
    for (int i=0; i<ledPinSize; i++)
    {
        digitalWrite(ledPin[i], HIGH); 
    }
}

// simple function to perform a change on an LED pin.
// int onOff is 0 or 1 (LOW or HIGH)
// int pin is the index of the pin in the global array
inline void PinOp(int onOff, int pin)
{
    digitalWrite(ledPin[pin], onOff);
}

// turn a specific pin on, wait for a specified time, then turn it off.
// int pin - index number of pin in the array
// float rate - rate of delay multiple
inline void BlinkPin(int pin, float rate)
{
    PinOp(HIGH, pin);
    delay(analogRead(sensor)*rate);
    PinOp(LOW, pin);
}

// convert an integer to display it as a binary number on the LEDs
// int value is the integer value to display.
inline void IntToPin(int value)
{
    //Get a value and display the binary value
    for (int i=0; i<ledPinSize; i++)
    {
        PinOp(value & (1<<i), i); //and the value with 2^i, will be 1 if the bit is on
    }
}

// swap integers and change the direction of a loop.
// int a and b are pointers to be swapped
// direction is a pointer to the direction multiple have its sign switched. 
inline void swap(int *a, int *b, int *direction)
{
    int temp;
    *direction *= -1;
    temp = *a;
    *a = *b;
    *b = temp;
}

// lights up one pin at a time, then two, then three, etc.
void NPlusOne(unsigned long seconds)
{
    unsigned long then = millis();  //timed show, get before time to do math later
    int MIN=0;              //min and max will swap when changing directions
    int MAX=ledPinSize; 
    int direction=1;        //sign will change i+1 into i-1
    bool (*comparison)(int, int) = &lt; //pointer to comparison function (lt on way up, gt on way down)
    void (*ranger)(int) = &PinRangeOn;  //pointer to function that turns LEDs on and off, depending on direction

    while(1)  //loop breaks after argument seconds is reached.
    {
        for(int i=MIN; (*comparison)(i, MAX); i=i+direction)
        {
            ranger(i);  //Turn LEDs on ascending or descending depending on direction
            if(timer(then, seconds, &ul_gt))
              {return;} //return when time is up
        }
        swap(&MIN, &MAX, &direction); //swap direction
        MIN += direction; //adjust MIN and MAX so that they don't dip below...
        MAX += direction; //...or above actual range available.
        comparison = (direction < 0) ? &gt : &lt; //swap comparison function pointer
        ranger = (direction < 0) ? &PinRangeOff : &PinRangeOn;  //swap on or off function pointer
    }
}

//Turn LEDs on in ascending direction.
void PinRangeOn(int pindex)
{
    for(int i=0; i<ledPinSize; i+=(pindex+1)) //increase outer loop by pindex
    {
        for(int j=i; j<(i+pindex+1) && j<ledPinSize; j++) //start turning leds on at position of outer loop
        {
            PinOp(1, j);
            delay(analogRead(sensor));
        }
        AllOff(); //turn all LEDs off between passes (could do i-j but this saves on space)
    }
}

//Turn LEDs on in descending order
void PinRangeOff(int pindex)
{
    for (int i=ledPinSize-1; i >= 0; i-= (pindex+1))  //start at top and work down, decrease by pindex+1
    {
        for (int j=i; j>(i-pindex-1) && j>=0; j--)  //start at outer loop, turn on LEDs one by one.
        {
            PinOp(1, j);
            delay(analogRead(sensor));
        }
        AllOff(); //turn all LEDs off between passes (could do i-j but this saves on space)
    }
}

//display numbers in binary.
//argument 1 seconds is the number of seconds to run the loop
//argument 2 is whether or not to allow counting in reverse
void BinaryCounter(unsigned long seconds, bool countDown)
{
    int i;  //counter in for loops.
    int MIN = 0;  //start at 0
    int MAX = 1 << ledPinSize;  //max is 2^(number of LEDs)
    int direction = 1;  //will be -1 when counting down
    unsigned long then = millis();  //for timing to exit loop, get start time
    bool (*comparison)(int, int) = &lt; //pointer to comparison, lt for going up, gt for going down
    
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
        delay(750); //set time between reaching max and restarting/changing directions
        if (countDown)  //if argument set
        {
            swap(&MIN, &MAX, &direction);  //reverse direction
            MIN += direction; //adjust so count doesn't go out of range
            MAX += direction; //same with this one...
            comparison = (direction < 0) ? &gt : &lt; //swap comparison function 
        }
    }
}//*/

//flip every other LED on and off.
void PotFlip(unsigned long seconds)
{
    //bytes to keep track of on and off pins, can make this one byte but may not be worth the effort
    byte x = 0;  //off pins
    byte y = 1;  //on pins ... initially
    
    unsigned long start = millis(); //start time, loop exits after argument seconds

    while(timer(start, seconds, &ul_lt))  //break when time reaches seconds
    {
        x ^= 1; //flip status, i.e. - if 1, 1 xor 1 = 0
        y ^= 1; //flip status         if 0, 0 xor 1 = 1
        for (byte i=0; i<ledPinSize; i++)
        {
            PinOp((i&1) ? x : y, i);  //if odd, x, else y (x and y flip each pass)
        }
        delay(analogRead(sensor));
    }
}

//stack LEDs ON going to the LEFT
void StackOnL()
{
    char MAX = ledPinSize; //end at highest LED
    for (char top=MAX; top>0; top--)  //end at the top and work down for stack effect
    {
        for(char pin=0;; pin++) //start at 0 and go up to top
        {
            if (pin != (top-1)) //if this isn't the last LED
            {
                BlinkPin(pin, 1);   //flash on then off
            }
            else  //if this is the last LED
            {
                PinOp(1, pin);  //turn it on and leave it on.
                delay(analogRead(sensor));  //still need the delay
                break;  //break from inner loop
            }
        }
    }
}

//turn OFF LEDs going to the LEFT
void StackOffL()
{
    char MAX = ledPinSize;  //end at highest LED
    for (char top=0; top<MAX; top++)  //end at the lowest
    {
        for(char pin=top; pin > -1; pin--)  //start from top and get smaller each pass
          //must be char because value goes to -1
        { //flash pin on then off
            BlinkPin(pin, 1);
        }
    }
}
//turn LEDs on going to the right
void StackOnR()
{
    char MAX = ledPinSize;  //start from highest LED
    for (char top=0; top<MAX; top++)  //end at top
    {
        for(char pin=MAX-1;; pin--) //start at highest and gradually get smaller
        {
            if (pin != top) //if this isn't the last LED
            {
                BlinkPin(pin, 1);   //flash on then off
            }
            else  //if this is the last LED
            {
                PinOp(1, pin);  //turn it on and leave it on.
                delay(analogRead(sensor));  //still need the delay
                break;  //break from inner loop
            }
        }
    }
}

//turn OFF LEDs going to the RIGHT
void StackOffR()
{
    char MAX = ledPinSize;
    for (char top=MAX-1; top>-1; top--) //start at top and get smaller
    {
        for(char pin=top; pin < MAX; pin++)
        { //flash pin on then off
            BlinkPin(pin, 1);
        }
    }
}

//stack LEDs on and off and in each direction
void Stack(unsigned long seconds)
{
    byte count = 0; //tracks 0-3 for each case
    unsigned long start = millis();
    
    while (timer(start, seconds, &ul_lt))
    { //loops until seconds are up
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
        count++;    //increment counter
        count %= 4; //stay between 0-3
    }
}

//turn LEDs on in order
void Linear(unsigned long seconds)
{
    unsigned long then = millis();
    byte i = 0;
    
    while(timer(then, seconds, &ul_lt))
    {
        i %= ledPinSize;  //ensure i stays within range of LED array
        BlinkPin(i,1);
        i++;                  //go to next i after blinking
    }
}

//bounce single LED back and forth with a slight pause at each end
void PingPong(unsigned long seconds)
{
    float rate_pin = .1; //rate per pin (float is expensive atmel chip)
    int MIN = 0;
    int MAX = ledPinSize-1;
    int i=MIN;  //for loop
    int direction = 1;  //+1 counts up, -1 counts down
    bool (*comparison)(int, int) = &lt; //pointer for comparison when switching directions
    unsigned long then = millis();  //time before starting
    
    while(timer(then, seconds, &ul_lt)) //run until over seconds argument
    {
        for (; (*comparison)(i,MAX); i+=direction)
        {
            BlinkPin(i, rate_pin);
        }
        PinOp(1, MAX);  //quickly turn last LED back on
        delay(analogRead(sensor));   //sleep longer for end
        i = MAX;  //store MAX in i for next pass in for loop
        swap(&MIN, &MAX, &direction);  //reverse direction
        comparison = (direction < 0) ? &gt : &lt;
    }
}

//Completely random display
void Bogo(unsigned long seconds)
{
    word MAX = 1 << ledPinSize;  //max is 2^(number of LEDs)
    unsigned long then = millis();  //get time to start
    while (timer(then, seconds, &ul_lt))  //run until reaching the end of the time in seconds
    {
        IntToPin(random(MAX)); //generate a random number from 0 - (Max-1)
        delay(analogRead(sensor));
    }
}

//flash Morse code letter using array in Morse.h
void LetterFlash(char letter)
{
    word index, i;
    if (isdigit(letter))
    {
        index = 26 + letter - '0';  //numbers start after alphabet in array
    }
    else if (isalpha(letter))
    {
        index = toupper(letter) - 'A';  //index in array, 0=A - 25=Z
    }
    else  //if it isn't a number or letter just leave for now
    {
        return; //leave since theres nothing to write
    }
    word morseLen = strlen(MorseCode[index]);
    for (i=0; i<morseLen; i++)  //using index, get Morse code, display each . or -
    {
        AllOn();  //turn all on no matter what
        if(MorseCode[index][i] == '.')  //if it's a dot...
        {
            delay(DIT * analogRead(sensor)*12); //delay is at the DIT rate
        }
        else if (MorseCode[index][i] == '-')  //if it's a dash...
        {
            delay(DAH * analogRead(sensor)*12); //delay is at the DAH rate
        }
        AllOff();  //Turn off after delay
        delay(DIT * analogRead(sensor)*12);  //delay between each char
    }
}

//Flash a whole sentence
void MorseFlash(char * sentence)
{
    word sentenceLength = strlen(sentence);
    for(word i=0; i<sentenceLength; i++) //for each letter in the sentence
    {
        if (isalnum)  //if it's a letter or number
        {
            LetterFlash(sentence[i]);
        }
        else
        { //most likely a space, delay otherwise
            delay(((DAH*2)+DIT) * analogRead(sensor)*12);
        }
        delay(DAH * analogRead(sensor)*12); //delay after each letter
    }
}

/*#define HBMIN 0x40
#define HBMAX 0xFF
#define DIRECTION 5
#define MAINPULSE(MAX, MIN, DIRECTION) ((MIN - MAX) / DIRECTION)
#define FADEPULSE(MAX, DIRECTION) (MAX / DIRECTION)
#define TOTALPULSE(MIN, MAX, DIRECTION) ((MAINPULSE(MAX, MIN, DIRECTION) * 2) + (FADEPULSE(MAX, DIRECTION) * 2))
#define HBTOTAL TOTALPULSE(HBMIN, HBMAX, DIRECTION)*/

//show off PWC for analog display
/* First half:
 *   rises from completely off to all the way on,
 *   lowers half way,
 *   goes all the way back up,
 *   goes all the way back off.
 * 
 * sleeps for second half
  */
void HeartBeat(unsigned long seconds)
{
    #if TROUBLESHOOTHB
    unsigned long totalpasses = 0;
    #endif
    #define HBRATIO 75
    int MIN = 0x40;
    int MAX = 0xFF;
    int direction = 5; //char can be negative
    bool (*comparison)(int, int) = &lt; //pointer to comparison when switching directions
    int i=0;   //for loops
    /* these next lines calculate the pulse rate for a smooth heartbeat */
    word mainPulse = ((MAX-MIN) / direction);   //calculate the number of steps for the main pulse
    word fadePulse = MAX / direction;           //number of steps for fading
    word totalPulse = (mainPulse * 2) + (fadePulse * 2);  //total number of passes for light ops
    float pulseRate = (1000 / 2) / totalPulse; //length of delay for each pass to take up rate
    pulseRate++;  //make up for lost decimal points, take sides with more time
    byte sCase; //switch for 5 cases
    unsigned long then = millis();  //for timer, get start time
    
    while (timer(then, seconds, &ul_lt))
    {
        sCase %= 5; //loop between 0-4
        #if TROUBLESHOOTHB
        //Serial.println(count, DEC);
        //Serial.println(times, DEC);
        #endif
        switch(sCase)
        {
            case 0:
                //ramp up from zero on first pass
                for (; i<MIN; i=i+direction)  //i shouldn't need readjustment
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
            { // regular pulse between mid and high, goes both directions
                analogWrite(6,  i);
                analogWrite(9,  i);
                analogWrite(10, i);
                analogWrite(11, i);
                delay(analogRead(sensor)/HBRATIO*pulseRate);
                #if TROUBLESHOOTHB
                totalpasses++;
                #endif  
            }
            i = MAX;  //set i for other direction
            swap(&MIN, &MAX, &direction);
            comparison = (direction < 0) ? &gt : &lt;
        }
        else
        { //turn off all lights for second half
            AllOff();
            delay(analogRead(sensor)/HBRATIO*500);  //500 = half of 1000
            #if TROUBLESHOOTHB
            if ((sCase) == 4)
            {
              Serial.println(totalpasses, DEC);
              totalpasses=0;
            }
            #endif
        }
        sCase++;  //increase counter

    }
}

void Blinker(unsigned long seconds)
{
    byte x = 0; //used for turning LED on or off, 0=off, 1=on
    const float MAX = 0.0625;         //Max speed
    const float MIN = 0.0009765625;   //minimum speed
    float y = MAX;       //starting value for blink speed
    char direction = -1;    //start in reverse
    bool (*comparison)(int, int) = &gt; //pointer to comparison when switching directions
    int pindir = -1;
    int bottom = ledPinSize-1;
    int top = -1;
    unsigned long then = millis();

    while(timer(then, seconds, &ul_lt))
    {
        x ^= 1; //swap on/off
        y += (direction * MIN); //always adjust speed by the minimum interval
            #if TROUBLESHOOT
            Serial.print("Y = ");
            Serial.println(y, 8);
            delay(250);
            #endif
        for (int pin = bottom; comparison(pin, top); pin=pin+pindir)
        {
            PinOp(x, pin);
            delay(analogRead(sensor)*(y*2));
        }
        delay(analogRead(sensor)*MAX);  //delay after each pass
        if (y<=MIN)
        {   //after reaching the fastest speed reverse direction of LED flash too
            direction = -direction;
            swap(&bottom, &top, &pindir);
            bottom += pindir;   //adjust for the loop
            top += pindir;      //-1 <--> 0, max-1 <--> max
            comparison = (pindir < 0) ? &gt : &lt; //swap comparison function 
        }
        if (y>=MAX)
        {   //at slowest speed reverse the speed only
            direction = -direction;
        }
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
    Blinker(SECONDS);
      #if TROUBLESHOOT
      t2 = millis();
      Serial.println("Blinker took:");
      Serial.println((t2-t1), DEC);
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
