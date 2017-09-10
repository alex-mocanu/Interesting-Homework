/* MOCANU Alexandru - 311CB */
#include "DNSfct.h"

int InitHash(Hash *H, int M){
  int i;
  H->M = M;
  H->buck = (AB)malloc(M*sizeof(TB));
  if(!H->buck)
		return 0;
  
	for(i = 0; i < M; i++)
		H->buck[i] = NULL;

  return 1;
}

void DistrBuck(AB aB, TFElim elim){
	TB b = *aB;
	if(!b)
		return;
	
	while(*aB){
		b = *aB;
		*aB = (*aB)->urm;
		elim(b->info); //Eliminam informatia din celula din bucket
		free(b);
	}
	
	free(*aB);
}

int InsOrd(AB aB, TFCmp cmp, TWeb ae){
	/*Daca nu am ajuns la capat de lista si nu putem insera elementul,
	 trecem mai departe*/
	while(*aB != NULL && (*aB)->info != NULL && cmp(&ae, (*aB)->info) > 0)
		aB = &(*aB)->urm;

	/*Daca nu am ajuns la capat de lista, dar am gasit un element egal
	 cu cel pe care vrem sa il inseram, ne oprim*/
	if(*aB != NULL && (*aB)->info != NULL && !cmp(&ae, (*aB)->info))
		return 0;

	TB aux = (TB)malloc(sizeof(Bucket));
	if(!aux)
		return 0;

	aux->info = malloc(sizeof(TWeb));
	if(!aux->info){
		free(aux);
		return 0;
	}

	((TWeb*)aux->info)->name = malloc(MAXLENGTH);
	if(!((TWeb*)aux->info)->name){
		free(aux->info);
		free(aux);
		return 0;
	}

	((TWeb*)aux->info)->ip = malloc(IPLENGTH);
	if(!((TWeb*)aux->info)->ip){
		free(((TWeb*)aux->info)->name);
		free(aux->info);
		free(aux);
		return 0;
	}
	//Copiem informatia in celula ce va fi adaugata
	strcpy(((TWeb*)aux->info)->name, ae.name);
	strcpy(((TWeb*)aux->info)->ip, ae.ip);
	//Legam noua celula in lista
	aux->urm = *aB;
	*aB = aux;

	return 1;
}

int hashfct(char* Key, int M){
	int i, result = 0;
	for(i = 0; i < strlen(Key); i++)
		result += Key[i];

	//Calculam restul impartirii sumei caracterelor la numarul de bucket-uri
	result %= M;

	return result;
}

void put(Hash *H, TFCmp cmp, char* Key, char* Value){
	int index;	//Indexul bucket-ului unde vom insera elementul
	int M = H->M;
	index = hashfct(Key, M);
	TWeb ae;
	ae.name = malloc(MAXLENGTH);
	if(!ae.name)
		return;
	
	ae.ip = malloc(IPLENGTH);
	if(!ae.ip){
		free(ae.name);
		return;
	}
	
	strcpy(ae.name, Key);
	strcpy(ae.ip, Value);
	InsOrd(&H->buck[index], cmp, ae); //Introducem elementul in bucket-ul corespunzator

	free(ae.name);
	free(ae.ip);
}

char* get(Hash *H, char* Key){
	int index; //Indexul bucket-ului unde vom face cautarea
	int M = H->M;
	index = hashfct(Key, M);
	TB p = H->buck[index];
	for(; p != NULL; p = p->urm)
		if(p != NULL && !strcmp(((TWeb*)p->info)->name, Key))
			return ((TWeb*)p->info)->ip; //Returnam valoarea cand o gasim

	return NULL;
}

void remove_key(Hash *H, TFElim elim, char* Key){
	int index; //Indexul bucket-ului unde vom face cautarea
	int M = H->M;
	index = hashfct(Key, M);
	AB p = &H->buck[index];
	TB aux;
	//Parcurgem lista si eliminam elementul daca il gasim
	for(; *p != NULL; p = &(*p)->urm)
		if(*p != NULL && !strcmp(((TWeb*)(*p)->info)->name, Key)){
			aux = *p;
			*p = (*p)->urm;
			elim(aux->info);
			free(aux);
			return;
		}
}

int find(Hash H, char* Key){
	int index; //Indexul bucket-ului unde vom face cautarea
	int M = H.M;
	index = hashfct(Key, M);
	TB p = H.buck[index];
	//Cautam elementul in lista
	for(; p != NULL; p = p->urm)
		if(p != NULL && !strcmp(((TWeb*)p->info)->name, Key))
			return 1;

	return 0;
}

void print_bucket(Hash *H, FILE* f, int n){
	int M = H->M;
	if(n > M-1)
		return;

	TB B = H->buck[n];
	//Daca bucket-ul est vid afisam VIDA
	if(!B){
		fprintf(f, "VIDA");
		return;
	}
	//In caz contrar afisam ip-urile din lista
	for(; B != NULL; B = B->urm)
		fprintf(f, "%s ", ((TWeb*)B->info)->ip);
}

void print(Hash *H, FILE* f){
	int i, M = H->M;
	for(i = 0; i < M; i++)
		//Daca lista nu este vida afisam indexul ei si ip-urile din ea
		if(H->buck[i] != NULL){
			fprintf(f, "%d: ", i);
			print_bucket(H, f, i);
			fprintf(f, "\n");
		}
}
