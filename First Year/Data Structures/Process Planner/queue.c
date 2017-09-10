#include "procese.h"

int IntrQ(AQueue q, void* el, size_t d, TFCmp cmp){
	//Cream elementul de introdus in coada
	TQueue aux = malloc(sizeof(TCelQ));
	if(!aux) return 0;
	aux->info = malloc(d);
	if(!aux->info){
		free(aux);
		return 0;
	}
	memcpy(aux->info, el, d);

	//Avansam pana la pozitia potrivita pentru inserare
	while(*q && cmp(el, (*q)->info) < 0)
		q = &(*q)->urm;

	//Introducem elementul in coada
	aux->urm = *q;
	*q = aux;
	return 1;
}

void* ExtrQ(AQueue q, size_t d){
	/*Pregatim un pointer pentru a returna informatia din elementul de la 
		inceputul cozii*/
	void* val = malloc(d);
	if(!val) return NULL;
	//Pregatim un pointer pentru extragerea elementului de la inceputul cozii
	TQueue aux;
	memcpy(val, (*q)->info, d);
	//Extragem elementul si eliberam memoria
	aux = *q;
	*q = aux->urm;
	free(aux->info);
	free(aux);
	return val;
}

void AfisQ(TQueue q, FILE* f){
	while(q){
		fprintf(f, "%d ", ((TProces*)q->info)->id);
		q = q->urm;
	}
}

void DistrQ(AQueue q){
	TQueue ant = NULL;
	//Inaintam in coada retinand elementul anterior pentru care eliberam memoria
	for(; *q; ant = *q, q = &(*q)->urm)
		if(ant){
			free(ant->info);
			free(ant);
			ant = NULL;
		}
}
