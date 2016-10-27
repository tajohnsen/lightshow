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
#define RANDOM 0        //Boolean to go through the shows in a random order
#define LINEAR 1        //Boolean to go through each show via an array pointer to functions
#define USEPHOTOCELL 1  //Boolean to use photocell to delay the start of the show
#define PHOTOCELLDELAY 1000
#define SECONDS 30      //seconds to run each show.
#define HBRATIO 75

/* Photocell use */
#define THRESHOLD 100  //maximum threshold to keep lights off
#define PHOTOCELL A5   //use Analog 5 for photocell

#include "Morse.h"

//void PinRangeOn(int pindex);
//void PinRangeOff(int pindex);
//void BinaryCounter(unsigned long seconds, bool countDown);

typedef void (*Show)(unsigned long);

int isdigit(int);
int toupper(int);
size_t strlen(const char *);

/* array of pins used in this project.
 * This order sets each PWM capable pin (on most Arduinos) to every other one. */
byte ledPin[] = {13, 11, 12, 10, 8, 9, 7, 6};//, 4, 5, 2, 3}; 
byte ledPinSize = sizeof(ledPin);
int sensor = A3;  // potentiometer analog sensor

inline bool timer(unsigned long then, unsigned long seconds, bool (*comparison)(unsigned long, unsigned long));
inline bool lt(int x, int y);
inline bool gt(int x, int y);
inline bool ul_lt(unsigned long x, unsigned long y);
inline bool ul_gt(unsigned long x, unsigned long y);
inline void ZeroPin(int pin);
inline void AllOff();
inline void AllOn();
inline void PinOp(int onOff, int pin);
inline void BlinkPin(int pin, float rate);
inline void IntToPin(int value);
inline void swap(int *a, int *b, int *direction);
inline bool DarkEnough();
void NPlusOne(unsigned long seconds);
void PinRangeOn(int pindex);
void PinRangeOff(int pindex);
void BinaryCounterCaller(unsigned long seconds);
void BinaryCounter(unsigned long seconds, bool countDown);
void PotFlip(unsigned long seconds);
void StackOnL();
void StackOffL();
void StackOnR();
void StackOffR();
void Stack(unsigned long seconds);
void Linear(unsigned long seconds);
void PingPong(unsigned long seconds);
void Bogo(unsigned long seconds);
void LetterFlash(char letter);
void MorseFlash(char * sentence);
void Morse(unsigned int seconds);
void HeartBeat(unsigned long seconds);
void Blinker(unsigned long seconds);
void FlashX(byte byGrouping);
void MiddleMeet();
void FlashGroups(unsigned long seconds);
