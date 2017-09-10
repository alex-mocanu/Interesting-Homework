#include "procese.h"
#define MAX 100

int main(int argc, char* argv[]){
	int nr_events, step = 0, identity, priority, event, i;
	char *action, *parse, input[MAX], output[MAX];
	char redundant;
	TStack *s;
	TQueue q = NULL;
	void *aux, *aux2;
	aux = (TProces*)malloc(sizeof(TProces));
	if(!aux) return 0;
	
	strcpy(input, argv[1]); //Identificam numele fisierului de intrare
	strcpy(output, argv[2]); //Identificam numele fisierului de iesire
	//Deschidem fisierele de intrare si de iesire
	FILE* fin = fopen(input, "r");
	FILE* fout = fopen(output, "a");

	//Alocam memorie pentru zona in care vom citi actiunile
	action = malloc(MAX);
	if(!action){
		free(aux);
		return 0;
	}

	//Citim numarul de evenimente si generam stivele
	fscanf(fin, "%d", &nr_events);
	fscanf(fin, "%c", &redundant);
	s = (AStack)calloc(nr_events, sizeof(TStack));
	if(!s){
	 free(aux);
	 free(action);
 	 return 0;
	}
	
	//Executam actiunile citite din fisierul de intrare
	while(fgets(action, MAX, fin)){
		parse = strtok(action, " ");
		if(!strcmp(parse, "\n"))
			break;

		step++;
		//Executam instructiunea corespunzatoare
		if(!strcmp(parse, "start")){
			parse = strtok(NULL, " ");
			identity = string_to_number(parse);
			parse = strtok(NULL, " \n");
			priority = string_to_number(parse);
			((TProces*)aux)->id = identity;
			((TProces*)aux)->pr = priority;
			((TProces*)aux)->time = step;

			//Introducem elementul in coada de prioritati
			IntrQ(&q, aux, sizeof(TProces), CmpProc);
		}

    if(!strcmp(parse, "wait")){
			parse = strtok(NULL, " ");
			event = string_to_number(parse);
			parse = strtok(NULL, " \n");
			identity = string_to_number(parse);
			
			//Extragem elementul din coada si il punem in stiva corespunzatoare
			aux2 = ExtrElQ(&q, identity, CmpProc);
			if(aux2)
				Push(&s[event], aux2, sizeof(TProces));
			free(aux2);
		}

    if(!strcmp(parse, "event")){
			parse = strtok(NULL, " \n");
			event = string_to_number(parse);
			
			/*Extragem toate elementele din stiva asociata evenimentului si le 
				punem inapoi in coada*/
			while(s[event]){
				aux2 = Pop(&s[event], sizeof(TProces));
				IntrQ(&q, aux2, sizeof(TProces), CmpProc);
				free(aux2);
			}
		}

    if(!strcmp(parse, "end")){
			parse = strtok(NULL, " \n");
			identity = string_to_number(parse);
			//Extragem elementul din coada
			aux2 = ExtrElQ(&q, identity, CmpProc);
			free(aux2);
		}

		//Afisam elementele din coada si din stive pentru pasul curent
		fprintf(fout, "%d\n", step);
		AfisQ(q, fout);
		fprintf(fout, "\n");
		for(i = 0; i < nr_events; i++)
			if(s[i]){
				fprintf(fout, "%d: ", i);
				AfisS(s[i], fout);
				fprintf(fout, "\n");
			}
		
		fprintf(fout, "\n");
	}
	
	//Eliberam memoria si inchidem fisierele
	for(i = 0; i < nr_events; i++)
			DistrS(&s[i]);
	free(s);
	DistrQ(&q);
	free(aux);
	free(action);
	fclose(fin);
	fclose(fout);
	return 0;
}
