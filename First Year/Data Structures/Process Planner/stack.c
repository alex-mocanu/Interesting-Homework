#include "procese.h"

int Push(AStack s, void* el, size_t d){
	//Cream elementul de introdus in stiva
  TStack aux = malloc(sizeof(TCelSt));
  if(!aux) return 0;
	aux->info = malloc(d);
	if(!aux->info){
		free(aux);
		return 0;
	}
	aux->urm = NULL;
	memcpy(aux->info, el, d);

	//Avansam pana la finalul stivei
  while(*s)
    s = &(*s)->urm;

	//Asezam elementul in varful stivei
	*s = aux;
  return 1;
}

void* Pop(AStack s, size_t d){
	//Pregatim un pointer pentru a returna informatia din varful stivei
  void* aux = malloc(d);
  if(!aux) return NULL;
	//Avansam pana la varful stivei	
	while((*s)->urm)
		s = &(*s)->urm;
	
	//Copiem elementul de la sfarsit de stiva si eliberam memoria alocata acestuia
	memcpy(aux, (*s)->info, d);
	free((*s)->info);
	free(*s);
	*s = NULL;
	return aux;
}

void AfisS(TStack s, FILE* f){
	if(s == NULL)
		return;

	AfisS(s->urm, f);
	fprintf(f, "%d ", ((TProces*)s->info)->id);
}

void DistrS(AStack s){
	TStack ant = NULL;
	//Inaintam in stiva retinand elementul anterior pentru care eliberam memoria	
	for(; *s; ant = *s, s = &(*s)->urm)
		if(ant){
			free(ant->info);
			free(ant);
			ant = NULL;
		}
}
