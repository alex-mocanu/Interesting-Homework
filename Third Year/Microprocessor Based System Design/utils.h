#ifndef UTILS_H_
#define UTILS_H_

#define F_CPU		16000000
#define NUM_KEYS	54
#define POT_EQ		478
#define NUM_CHANS	4
#define MAX_ADC		1024
#define SAMPLE_RATE	20000
#define NUM_NOTES	45

#define D	73
#define DE	78
#define E	82
#define F	87
#define FG	92
#define G	98
#define GA	104
#define A	110
#define AB	117
#define B	123
#define cS	131
#define cdS	139
#define dS	147
#define deS	156
#define eS	165
#define fS	175
#define fgS	185
#define gS	196
#define gaS	208
#define aS	220
#define abS	233
#define bS	247
#define c1	262
#define cd1	277
#define d1	294
#define de1	311
#define e1	330
#define f1	349
#define fg1	370
#define g1	392
#define ga1	415
#define a1	440
#define ab1	466
#define b1	494
#define c2	523
#define cd2	554
#define d2	587
#define de2	622
#define e2	659
#define f2	698
#define fg2	740
#define g2	784
#define ga2	831
#define a2	880
#define ab2	932
#define b2	988
#define c3	1047
#define cd3	1109
#define d3	1175
#define de3	1245
#define e3	1319
#define f3	1397
#define fg3	1480
#define g3	1568

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Initializeaza lista de note */
void init_tones(int *tones);

/* Initializeaza notele pentru cantec */
void init_song(int *notes, int *duration);

#endif
