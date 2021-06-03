/*	Author: lab
 *  Partner(s) Name: none
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "timer.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>

unsigned char randRow();
unsigned char randPat(unsigned char pat);
unsigned char LeftandRight(int cnt, unsigned char LoR);
unsigned char UpandDown(int cnt, unsigned char UoD);
void fillArr();
void correctAns();
void wrongAns();
void Lose();
void Start();

const int MAX = 100;
int size = 3;
int totCnt = 1;
int life = 8;
unsigned char arrP[100] = {0x20, 0x20, 0x20, 0x20, 0x20};
unsigned char arrR[100] = {0xFD, 0xFD, 0xFD, 0xFD, 0xFD};

unsigned char memoryOut = 0x01; //checks if random array sshould be outputted
unsigned char fill = 0x01;
unsigned char gameStart = 0x00;

enum forever {off, on};
int Endless(int state){
	static unsigned char pattern = 0x20;    // can go between 0x20 to 0x04
	static unsigned char row = 0xFD;	// can go between row 0xFD to xFB to xF7
	static int done = -1;
	
	if(fill == 0x01){fillArr(); fill = 0x00;}
	if(life < 1 || gameStart == 0x00){return state;}

	switch(state){
		case off:
			if(memoryOut == 0x01){state = on;}
			else{
				state = off;
			}
		break;

		case on:
			if(done > size){
				memoryOut = 0x00; 
				state = off; 
				done = -1;
			}
		break;
	}

	switch(state){
		case off:
		break;

		case on:
		if(done == -1 || done == size){
			PORTC = 0x3C;
			PORTD = 0xF1;
			done++;
		}
		else{
			pattern = arrP[done];
			row = arrR[done];
			done++;
			PORTC = pattern;
			PORTD = row;
		}
		break;
	}
	return state;
}

unsigned char lostLife = 0x00;
int cntR = 1;
int cntP = 1;
int match = 0;
enum input {init, wait, left, right, up, down, enter};
int UserIn(int state){
	unsigned char input = ~PINB;
	static unsigned char row = 0xFD;
	static unsigned char pattern = 0x20;
	
	if(memoryOut == 0x01){return state;} //if lights are being output then return until its finished
	if(life < 1 || gameStart == 0x00){return state;}
	
	switch(state){
		case init:
			if(input == 0x01){state = left;}
			else if(input == 0x02){state = right;}
			else if(input == 0x04){state = enter;}
			else if(input == 0x08){state = down;}
			else if(input == 0x10){state = up;}
			else{state = init;}
		break;

		case wait:
			if(input == 0x01 || input == 0x02 || input == 0x04 || input == 0x08 || input == 0x10){
			       state = wait;
			}
	 		else{state = init;}		
		break;

		case left:
			state = wait;
		break;

		case right:
			state = wait;
		break;

		case down:
			state = wait;
		break;

		case up:
			state = wait;
		break;

		case enter:
			state = wait;
		break;

		default:
			state = init;
		break;
	}

	switch(state){
		case init:
		break;
		
		case wait:
		break;

		case left:
			pattern = LeftandRight(cntP, 0x00);	
		break;

		case right:
			pattern = LeftandRight(cntP, 0x01);
		break;

		case down:
			row = UpandDown(cntR, 0x00);
		break;

		case up:
			row = UpandDown(cntR, 0x01);
		break;

		case enter:
			if(pattern == arrP[match] && row == arrR[match]){
				match++;
				correctAns();
			}
			else{
				wrongAns();
				lostLife = 0x01;
			}
		break;

		default:
			state = init;
		break;
	}
	if(match >= size){
		match = 0;
		memoryOut = 0x01;
		fill = 0x01;
		++totCnt;
		if(totCnt % 3 == 0 && size < 100){
			++size;
		}
	}
	PORTC = pattern;
	PORTD = row;

	return state;
}

enum points {Lives, Dec};
int Score(int state){
	static unsigned char botLife = 0xFF;

	if(memoryOut == 0x01){return state;} //if lights are being output then return until its finished
	if(life < 1 || gameStart == 0x00){return state;}
	
	if(life == 8){botLife = 0xFF;}

	switch(state){
		case Lives:
			if(lostLife == 0x01){state = Dec; lostLife = 0x00;}
			else{state = Lives;}
		break;

		case Dec:
			state = Lives;
		break;
	}

	switch(state){
		case Lives:
		break;

		case Dec:
			life--;
			botLife <<= 1;
		break;
	}
	PORTD = 0xEE;
	PORTC = botLife;

	return state;
}

enum end{finish, reset, start};
int GameStartandOver(int state){
	unsigned char input = ~PINB;
	if(life > 0 && gameStart == 0x01){return state;}
	
	switch(state){
		case finish:
			Lose();
			if(input == 0x04){
				state = reset;
			}
			else{state = finish;}
		break;
		
		case reset:
			life = 8;
			memoryOut = 0x01;
			match = 0;
			fill = 0x01;
			size = 3;
			state = start;
			gameStart = 0x00;
		break;

		case start:
			Start();
			if(input == 0x04){state = finish; gameStart = 0x01;}
			else{state = start;}
		break;
	}
	
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
    /* Insert your solution below */
	
	static task task1, task2, task3, task4;
	task *tasks[] = { &task1, &task2, &task3, &task4 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = off;
	task1.period = 1500;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Endless;
	
	task2.state = init;
	task2.period = 1;
	task2.elapsedTime = task2.period;
	task2.TickFct = &UserIn;

	task3.state = Lives;
	task3.period = 1;
	task3.elapsedTime = 1;
	task3.TickFct = &Score;
	
	task4.state = start;
	task4.period = 1;
	task4.elapsedTime = 1;
	task4.TickFct = &GameStartandOver;

	int i = 0;
	TimerSet(1);
	TimerOn();
    while (1) {
	for(i = 0; i < numTasks; ++i){
		if(tasks[i]->elapsedTime >= tasks[i]->period){	
		    tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
		    tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1;
	}
	while(!TimerFlag){}
	TimerFlag = 0;
    }
    return 1;
}


void fillArr(){
	int i;
	arrP[0] = randPat(arrP[0]);
	arrR[0] = randRow();
	for(i = 1; i < size + 1; ++i){
		arrP[i] = randPat(arrP[i-1]);
		arrR[i] = randRow();
	}
}

unsigned char randRow(){ //random row
	int random = rand() % 3;
	if(random == 0){return 0xFD;}
	else if(random == 1){return 0xFB;}
	else{return 0xF7;}
}

unsigned char randPat(unsigned char pat){ //random spot for pattern
	unsigned char check = pat;
	int random;
	while(check == pat){
		random = rand() % 4;
		if(random == 0){check = 0x20;}
		else if(random == 1){check = 0x10;}
		else if(random == 2){check = 0x08;}
		else{check = 0x04;}
	}
	return check;
}

unsigned char LeftandRight(int cnt, unsigned char LoR){
	unsigned char temp = 0x20;
	if(LoR == 0x00){ //left shift
		cnt--;
		if(cnt <= 0){cnt = 4;}
	}
	else{ //right shift
		cnt++;
		if(cnt >= 5){cnt = 1;}
	}

	if(cnt == 1){temp = 0x20;}
	else if(cnt == 2) {temp = 0x10;}
	else if(cnt == 3) {temp = 0x08;}
	else if(cnt == 4) {temp = 0x04;}
	
	cntP = cnt;

	return temp;
}

unsigned char UpandDown(int cnt, unsigned char UoD){
	unsigned char temp = 0xFD;
	if(UoD == 0x00){ //down shift
		cnt--;
		if(cnt <= 0){cnt = 3;}
	}
	else{ //up shift
		cnt++;
		if(cnt >= 4){cnt = 1;}
	}

	if(cnt == 1){temp = 0xFD;}
	else if(cnt == 2) {temp = 0xFB;}
	else if(cnt == 3) {temp = 0xF7;}
	
	cntR = cnt;

	return temp;
}

void correctAns(){
	int k = 0;
	PORTD = 0xFD;
	for(k=0; k < 30000; ++k){
		PORTC = 0x04;
		PORTD = 0xFD;

		PORTD = 0xFF;

		PORTC = 0x28;
		PORTD = 0xFB;

		PORTD = 0xFF;
		
		PORTC = 0x10;
		PORTD = 0xF7;

		PORTD = 0xFF;
	}
	return;
}

void wrongAns(){
	int k = 0;
	PORTD = 0xFD;
	for(k=0; k < 30000; ++k){
		PORTC = 0x24;
		PORTD = 0xFD;

		PORTD = 0xFF;

		PORTC = 0x18;
		PORTD = 0xFB;

		PORTD = 0xFF;
		
		PORTC = 0x24;
		PORTD = 0xF7;

		PORTD = 0xFF;
	}
	return;
}

void Lose(){
	int k = 0;

	for(k = 0; k < 30000; ++k){
	PORTD = 0xFF;

	PORTC = 0x20;
	PORTD = 0xF0;

	PORTD = 0xFF;
		
	PORTC = 0x3C;
	PORTD = 0xEF;

	PORTD = 0xFF;
	}
}

void Start(){
	int k = 0;

	for(k = 0; k < 30000; ++k){
	PORTD = 0xFF;

	PORTC = 0x3C;
	PORTD = 0xEA;

	PORTD = 0xFF;
		
	PORTC = 0x20;
	PORTD = 0xFD;

	PORTD = 0xFF;

	PORTC = 0x04;
	PORTD = 0xF7;

	PORTD = 0xFF;

/*	PORTC = 0x3C;
	PORTD = 0xEF;

	PORTD = 0xFF;*/
	}
}


