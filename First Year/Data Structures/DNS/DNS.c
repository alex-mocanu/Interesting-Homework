/* MOCANU Alexandru - 311CB */
#include "DNSfct.h"

//Functie de comparare a cheilor
int cmpkeys(void* a, void* b){
	return strcmp(((TWeb*)a)->name, ((TWeb*)b)->name);
}

//Functie de eliminare a informatiei dintr-o celula dintr-un bucket
void elimweb(void* web){
	if(!(TWeb*)web) return;
	free(((TWeb*)web)->name);
	free(((TWeb*)web)->ip);
	free((TWeb*)web);
}

int main(int argc, char *argv[]){
	Hash H;
	int M = 0, i;
	char *input_file, *output_file, *file_line;
	//Determinam numarul de bucket-uri din argumentul din linia de comanda
	for(i = 0; i < strlen(argv[1]); i++)
		M = 10 * M + (argv[1][i] - '0');
	//Initializam hash-ul
	InitHash(&H, M);

	//Deschiderea fiserelor de citire s scriere
	input_file = argv[2];
	output_file = argv[3];
	FILE* fin = fopen(input_file, "r");
	FILE* fout = fopen(output_file, "w");
	//In acest vector citim liniile din fiserul de test
	file_line = malloc(MAXLENGTH);

	//Prelucrarea comenzilor din fisierul de intrare
	while(fgets(file_line, MAXLENGTH, fin)){
		//In cazul in care citim o linie noua trecem mai departe
		if(file_line[0] == '\n')
			continue;
		//Eliminam newline-ul din linia citita
		file_line[strlen(file_line) - 1] = 0;
		char* p; //pointerul cu care parsam linia
		p = strtok(file_line, " ");

		//Executarea comenzii din linia citita
		if(!strcmp(p, "put")){
			char *key, *value;
			p = strtok(NULL, " ");
			key = p;
			p = strtok(NULL, " ");
			value = p;
			put(&H, cmpkeys, key, value);
		}

		else if(!strcmp(p, "get")){
			char *key;
			p = strtok(NULL, " ");
			key = p;
			if(get(&H, key) == NULL)
				fprintf(fout, "NULL\n");
			else
				fprintf(fout, "%s\n", get(&H, key));
		}

		else if(!strcmp(p, "remove")){
			char *key;
			p = strtok(NULL, " ");
			key = p;
			remove_key(&H, elimweb, key);
		}

		else if(!strcmp(p, "find")){
			char *key;
			p = strtok(NULL, " ");
			key = p;
			if(find(H, key))
				fprintf(fout, "True\n");
			else
				fprintf(fout, "False\n");
		}

		else if(!strcmp(p, "print")){
			print(&H, fout);
		}

		else if(!strcmp(p, "print_bucket")){
			int n = 0;
			p = strtok(NULL, " ");
			//Determinam indicele de bucket-ului
			for(i = 0; i < strlen(p); i++)
				n = 10 * n + (p[i] -'0');
			if(n < M){
				print_bucket(&H, fout, n);
				fprintf(fout, "\n");
			}
		}
	}

	//Eliberare de memorie
	free(file_line);
	for(i = 0; i < M; i++)
		DistrBuck(&H.buck[i], elimweb);
	free(H.buck);

	//Inchidem fisierele
	fclose(fin);
	fclose(fout);

	return 0;
}

