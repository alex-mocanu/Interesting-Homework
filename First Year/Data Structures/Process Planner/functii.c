#include "procese.h"

int CmpProc(void* a, void* b){
  TProces *x, *y;
  x = (TProces*)a;
  y = (TProces*)b;

  if(x->pr < y->pr) return -1;
  if(x->pr > y->pr) return 1;
  if(x->pr == y->pr){
    if(x->time < y->time) return 1;
    if(x->time > y->time) return -1;
		if(x->time == y->time) return 0;
  }
}

int string_to_number(char* a){
  int i, n = 0;
  for(i = 0; i < strlen(a); i++){
    n = 10 * n + a[i] - '0';
  }
  return n;
}

void* ExtrElQ(AQueue q, int id, TFCmp cmp){
	//Stiva auxiliara pentru stocarea elementelor din coada
	TStack auxs = NULL;
	//Pointer auxiliar pentru deplasarea elementelor din coada in stiva si invers
	void* auxq;
	//Pointerul unde vom retine informatiile despre elementul extras
	void* info = NULL;

	/*Cat timp nu am extras toate elementele din coada sau nu am gasit elementul 
		de extras, scoatem elemente din coada si le introducem in stiva*/
	while(*q && ((TProces*)(*q)->info)->id != id){
		auxq = ExtrQ(q, sizeof(TProces));
		Push(&auxs, auxq, sizeof(TProces));
		free(auxq);
	}

	//Daca nu am golit coada inseamna ca am gasit elementul si il extragem
	if(*q)
		info = ExtrQ(q, sizeof(TProces));

	//Reintroducem elementele extrase in coada
	while(auxs){
		auxq = Pop(&auxs, sizeof(TProces));
		IntrQ(q, auxq, sizeof(TProces), cmp);
		free(auxq);
	}

	return info;
}
