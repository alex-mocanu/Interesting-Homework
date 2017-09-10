#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define MAXLENGTH 100
#define IPLENGTH 20

//Sablonul unei functii de comparare
typedef int (*TFCmp)(void*, void*);

//Sablonul unei functii de eliminare a unui element dintr-o lista
typedef void (*TFElim)(void*);

//Structura ce retine numele si ip-ul unei pagini web
typedef struct{
	char* name; //Numele paginii web
	char* ip; //IP-ul paginii web
} TWeb;

//Structura bucket
typedef struct Bucket{
	struct Bucket *urm; //Elementul urmator din bucket
	void* info; //Informatia din celula curenta din bucket
} Bucket, *TB, **AB;

//Structura hash
typedef struct{
	TB* buck; //Bucket-urile din hash
	int M; //Numarul de bucket-uri din hash
} Hash;

//Functia hash
int hashfct(char* Key, int M);

//Functiile aplicate pe hash
//Adauga un element in hash
void put(Hash *H, TFCmp cmp, char* Key, char* Value);
//Intoarce valoarea asociata unei chei
char* get(Hash *H, char* Key);
//Elimina celula cu cheia Key din bucket-ul in care se afla aceasta
void remove_key(Hash *H, TFElim elim, char* Key);
//Intoarce 1 daca elementul a fost gasit, 0 in caz contrar
int find(Hash H, char* Key);
//Afiseaza hash-ul
void print(Hash *H, FILE* f);
//Afiseaza bucket-ul cu indicele n
void print_bucket(Hash *H, FILE* f, int n);

//Functii aditionale
//Initializeaza un hash
int InitHash(Hash *H, int M);
//Elibereaza memoria alocata bucket-ului
void DistrBuck(AB aB, TFElim elim);
//Insereaza elementul de la adresa ae in bucket, pastrandu-i ordonarea
int InsOrd(AB aB, TFCmp cmp, TWeb ae);
