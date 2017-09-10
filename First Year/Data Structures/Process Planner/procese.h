#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*TFCmp)(void*, void*); //Functie de comparare a doua elemente

typedef struct{
	int id; //id-ul procesului
	int pr; //prioritatea procesului
	int time; //timpul de lansare al procesului
} TProces;

//Structura stiva
typedef struct celst{
	struct celst* urm;
	void* info;
} TCelSt, *TStack, **AStack;

//Structura coada
typedef struct celq{
	struct celq* urm;
	void* info;
} TCelQ, *TQueue, **AQueue;

//Operatia de inserare in stiva
int Push(AStack s, void* el, size_t d);
//Operatia de scoatere din stiva
void* Pop(AStack s, size_t d);
//Operatia de afisare a elementelor din stiva
void AfisS(TStack s, FILE* f);
//Functie de eliberare de memorie a unei stive
void DistrS(AStack s);

//Operatia de introducere ordonata in coada
int IntrQ(AQueue q, void* el, size_t d, TFCmp cmp);
//Operatia de scoatere a elementului cu prioritatea cea mai mare din coada
void* ExtrQ(AQueue q, size_t d);
//Operatia de afisare a cozii
void AfisQ(TQueue q, FILE* f);
//Functie de eliberare de memorie a unei cozi
void DistrQ(AQueue q);

//Functia de comparare a doua procese
int CmpProc(void* a, void* b);
//Functie de transformare a unui numar din forma text in intreg
int string_to_number(char* a);
//Functie de extragere a elementelor din coada in functie de identificator
void* ExtrElQ(AQueue q, int id, TFCmp cmp);
