#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#define MAX 10000

typedef struct Pair{
    int beg, end;
} Pair;

// Compare function for qsort
int comp(const void* a, const void* b){
    if( *(int*)a < *(int*)b) return -1;
    if( *(int*)a == *(int*)b) return 0;
    if( *(int*)a > *(int*)b) return 1;
}

// Read topology file to determine a node's neighbours
void determineNeighbours(FILE* topF, int rank, int* nrNeigh, int** neighs){
    int ind = -1;
    char line[MAX];
    char *p;
    while(ind != rank){
        fgets(line, MAX, topF);
        p = strtok(line, " \n");
        ind = atoi(p);
    }

    p = strtok(NULL, " \n");
    while(p){
        (*neighs)[*nrNeigh] = atoi(p);
        ++(*nrNeigh);
        p = strtok(NULL, " \n");
    }
}

// Read image from file
void readImage(FILE* image, int* width, int* height, unsigned char*** img){
    int ok = 0;
    char line[MAX];
    // Read width and height
    while(!ok){
        fgets(line, MAX, image);
        char* p = strtok(line, " \n");
        *width = atoi(p);
        if(*width){
            p = strtok(NULL, " \n");
            *height = atoi(p);
            if(*height)
                ok = 1;
        }
    }

    // Allocate memory for the image matrix
    (*img) = (unsigned char**)malloc((*height + 2) * sizeof(unsigned char*));
    for(int i = 0; i < *height + 2; ++i)
        (*img)[i] = (unsigned char*)calloc(*width + 2, sizeof(unsigned char));

    // Read matrix value limit
    ok = 0;
    while(!ok){
        fgets(line, MAX, image);
        if(atoi(line) || line[0] == '0')
            ok = 1;
    }

    // Read matrix elements
    for(int i = 1; i <= *height; ++i)
        for(int j = 1; j <= *width; ++j){
            ok = 0;
            while(!ok){
                fgets(line, MAX, image);
                (*img)[i][j] = atoi(line);
                if((*img)[i][j] || line[0] == '0')
                    ok = 1;
            }
        }
}

// Process the lines that reached a leaf
void processImage(unsigned char*** img, int height, int width, int filter){
    int vecX[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    int vecY[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
    int smooth[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    int blur[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
    int sharpen[9] = {0, -2, 0, -2, 11, -2, 0, -2, 0};
    int removal[9] = {-1, -1, -1, -1, 9, -1, -1, -1, -1};

    int** aux;
    aux = (int**)malloc((height + 1) * sizeof(int*));
    for(int i = 0; i < height + 1; ++i)
        aux[i] = (int*)calloc(width + 1, sizeof(int));
    for(int i = 1; i <= height; ++i)
        for(int j = 1; j <= width; ++j){
            aux[i][j] = 0;
            // Smooth filter
            if(filter == 2){
                for(int k = 0; k < 9; ++k)
                    aux[i][j] += (int)(*img)[i + vecX[k]][j + vecY[k]] * smooth[k];
                aux[i][j] /= 9;
            }
            // Blur filter
            else if(filter == 3){
                for(int k = 0; k < 9; ++k)
                    aux[i][j] += (int)(*img)[i + vecX[k]][j + vecY[k]] * blur[k];
                aux[i][j] /= 16;
            }
            // Sharpen filter
            else if(filter == 4){
                for(int k = 0; k < 9; ++k)
                    aux[i][j] += (int)(*img)[i + vecX[k]][j + vecY[k]] * sharpen[k];
                aux[i][j] /= 3;
            }
            // Mean-removal filter
            else if(filter == 5){
                for(int k = 0; k < 9; ++k)
                    aux[i][j] += (int)(*img)[i + vecX[k]][j + vecY[k]] * removal[k];
            }
            else
                aux[i][j] = (int)(*img)[i][j];
        }

    for(int i = 1; i <= height; ++i)
        for(int j = 1; j <= width; ++j)
            if(aux[i][j] < 0)
                (*img)[i][j] = 0;
            else if(aux[i][j] > 255)
                (*img)[i][j] = 255;
            else
                (*img)[i][j] = aux[i][j];
    for(int i = 0; i < height + 1; ++i)
        free(aux[i]);
    free(aux);
}

int main(int argc, char** argv){
    int rank, size;
    char *topFile, *imFile, *statFile;
    MPI_Status s;
    int buff;
    int nrNeigh = 0, *neighs;   // number of neighbours and neighbours
    int parent, nrChildren = 0, *children;  // parent, number of children and children
    int childPos[MAX];  // order number of a child
    Pair* indices;  // first and last line distributed to each child
    int nrLeaves = 0;   // number of leaves in the node's subtree
    int width, height;  // width and height of the image
    unsigned char** img;    // image
    int linesToSend, remainingLines;    // number of lines to send to children
    int linesStat = 0;  // number of lines processed

    // Open topology, image and statistics files
    topFile = argv[1];
    imFile = argv[2];
    statFile = argv[3];
    FILE* topF = fopen(topFile, "r");
    FILE* imF = fopen(imFile, "r");
    FILE* stF = fopen(statFile, "w");

    neighs = (int*)malloc(MAX * sizeof(int));
    children = (int*)malloc(MAX * sizeof(int));
    indices = (Pair*)malloc(MAX * sizeof(Pair));

    // Initiate MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // If the node is root
    if(rank == 0){
        determineNeighbours(topF, rank, &nrNeigh, &neighs);

        // Determine the topology
        for(int i = 0; i < nrNeigh; ++i)
            MPI_Send(&buff, 1, MPI_INT, neighs[i], 0, MPI_COMM_WORLD);
        for(int i = 0; i < nrNeigh; ++i){
            ++nrChildren;
            children[i] = neighs[i];
            childPos[children[i]] = i;
            MPI_Recv(&buff, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &s);
            nrLeaves += buff;
        }
        qsort(children, nrChildren, sizeof(int), comp);

        // Apply filters to images
        int nrImages;
        char line[MAX], *p, *filter, *inImg, *outImg;
        fscanf(imF, "%d\n", &nrImages);
        for(int i = 0; i < nrImages; ++i){
            fgets(line, MAX, imF);
            p = strtok(line, " \n");
            filter = p;
            p = strtok(NULL, " \n");
            inImg = p;
            p = strtok(NULL, " \n");
            outImg = p;

            // Read image
            FILE* image = fopen(inImg, "r");
            readImage(image, &width, &height, &img);
            fclose(image);

            // Establish the filter
            int tag;
            if(strstr(filter, "smooth"))
                tag = 2;
            else if(strstr(filter, "blur"))
                tag = 3;
            else if(strstr(filter, "sharpen"))
                tag = 4;
            else if(strstr(filter, "mean_removal"))
                tag = 5;

            // Send lines to be processed
            linesToSend = height / nrChildren;
            for(int i = 0; i < nrChildren; ++i){
                if(i < nrChildren - 1)
                    buff = linesToSend;
                else
                    buff = height - (nrChildren - 1) * linesToSend;
                if(i == 0)
                    indices[i].beg = 1;
                else
                    indices[i].beg = indices[i - 1].end;
                indices[i].end = indices[i].beg + buff;
                // If there aren't lines to be sent, stop
                if(buff == 0)
                    continue;
                // Send number of lines to be processed and their size
                MPI_Send(&buff, 1, MPI_INT, children[i], 0, MPI_COMM_WORLD);
                buff = width;
                MPI_Send(&buff, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);
                // Send the lines
                for(int j = indices[i].beg - 1; j < indices[i].end + 1; ++j)
                    MPI_Send(img[j], width + 2, MPI_CHAR, children[i], tag, MPI_COMM_WORLD);
            }

            // Receive processed lines from children
            for(int j = 0; j < height; ++j){
                unsigned char lineBuff[width + 2];
                MPI_Recv(lineBuff, width + 2, MPI_CHAR, MPI_ANY_SOURCE, 6, MPI_COMM_WORLD, &s);
                int source = s.MPI_SOURCE;
                for(int k = 1; k <= width; ++k)
                    img[indices[childPos[source]].beg][k] = lineBuff[k];
                ++indices[childPos[source]].beg;
            }

            // Print new image into file
            image = fopen(outImg, "w");
            fprintf(image, "P2\n# CREATOR: GIMP PNM Filter Version 1.1\n%d %d\n255\n", width, height);
            for(int i = 1; i <= height; ++i)
                for(int j = 1; j <= width; ++j)
                    fprintf(image, "%d\n", img[i][j]);
            fclose(image);

            for(int j = 0; j < height + 2; ++j)
                free(img[j]);
            free(img);
        }

        // Send final signal
        for(int i = 0; i < nrChildren; ++i)
            MPI_Send(&buff, 1, MPI_INT, children[i], 7, MPI_COMM_WORLD);
        // Wait for the statistics
        int linesProc[MAX] = {0};
        for(int i = 0; i < nrLeaves; ++i){
            MPI_Recv(&buff, 1, MPI_INT, MPI_ANY_SOURCE, 8, MPI_COMM_WORLD, &s);
            int leaf = buff >> 16;
            int lines = buff & 0xffff;
            linesProc[leaf] = lines;
        }

        // Write statistics to stats file
        for(int i = 0; i < size; ++i)
            fprintf(stF, "%d: %d\n", i, linesProc[i]);
    }
    else{
        determineNeighbours(topF, rank, &nrNeigh, &neighs);

        // Determine the topology
        MPI_Recv(&buff, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &s);
        parent = s.MPI_SOURCE;
        for(int i = 0; i < nrNeigh; ++i)
            if(neighs[i] != parent)
                MPI_Send(&buff, 1, MPI_INT, neighs[i], 0, MPI_COMM_WORLD);
        for(int i = 0; i < nrNeigh - 1; ++i){
            MPI_Recv(&buff, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
            if(s.MPI_TAG == 1){
                children[nrChildren] = s.MPI_SOURCE;
                childPos[s.MPI_SOURCE] = nrChildren;
                ++nrChildren;
                nrLeaves += buff;
            }
        }
        qsort(children, nrChildren, sizeof(int), comp);

        // If the node is a leaf
        if(nrChildren == 0)
            nrLeaves = 1;
        // Send number of leaves to parent
        MPI_Send(&nrLeaves, 1, MPI_INT, parent, 1, MPI_COMM_WORLD);

        int finalTag = 0;   // indicator for the last message from root
        while(!finalTag){
            // Receive lines to process/send forward from parent or the end signal
            MPI_Recv(&buff, 1, MPI_INT, parent, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
            if(!s.MPI_TAG){
                height = buff;
                MPI_Recv(&buff, 1, MPI_INT, parent, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
                width = buff;
                img = (unsigned char**)malloc((height + 2) * sizeof(unsigned char*));

                for(int i = 0; i < height + 2; ++i)
                    img[i] = (unsigned char*)calloc(width + 2, sizeof(unsigned char));
                for(int i = 0; i <= height + 1; ++i)
                    MPI_Recv(img[i], width + 2, MPI_CHAR, parent, MPI_ANY_TAG, MPI_COMM_WORLD, &s);

                // If the node is a leaf process the image and send it to the parent
                if(nrChildren == 0){
                    linesStat += height;
                    processImage(&img, height, width, s.MPI_TAG);
                }
                // Else distribute lines to children and wait for the processed lines
                else{
                    // Send lines to be processed
                    linesToSend = height / nrChildren;
                    for(int i = 0; i < nrChildren; ++i){
                        if(i < nrChildren - 1)
                            buff = linesToSend;
                        else
                            buff = height - (nrChildren - 1) * linesToSend;
                        if(i == 0)
                            indices[i].beg = 1;
                        else
                            indices[i].beg = indices[i - 1].end;
                        indices[i].end = indices[i].beg + buff;
                        // If there aren't anymore lines to be sent, stop
                        if(buff == 0)
                            continue;
                        // Send number of lines to be processed
                        MPI_Send(&buff, 1, MPI_INT, children[i], 0, MPI_COMM_WORLD);
                        buff = width;
                        MPI_Send(&buff, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);
                        // Send the lines
                        for(int j = indices[i].beg - 1; j < indices[i].end + 1; ++j)
                            MPI_Send(img[j], width + 2, MPI_CHAR, children[i], s.MPI_TAG, MPI_COMM_WORLD);
                    }

                    // Receive processed lines from children
                    for(int i = 0; i < height; ++i){
                        unsigned char lineBuff[width + 2];
                        MPI_Recv(lineBuff, width + 2, MPI_CHAR, MPI_ANY_SOURCE, 6, MPI_COMM_WORLD, &s);
                        int source = s.MPI_SOURCE;
                        for(int j = 1; j <= width; ++j)
                            img[indices[childPos[source]].beg][j] = lineBuff[j];
                        ++indices[childPos[source]].beg;
                    }
                }

                // Send lines to parent
                for(int i = 1; i <= height; ++i)
                    MPI_Send(img[i], width + 2, MPI_CHAR, parent, 6, MPI_COMM_WORLD);

                for(int i = 0; i < height + 2; ++i)
                    free(img[i]);
                free(img);
            }
            else{
                finalTag = 1;
                // The leaf transmits the number of processed lines to the root
                if(nrChildren == 0){
                    buff = rank << 16;
                    buff += linesStat & 0xffff;
                    MPI_Send(&buff, 1, MPI_INT, parent, 8, MPI_COMM_WORLD);
                }
                else{
                    // Send finish message to children
                    for(int i = 0; i < nrChildren; ++i)
                        MPI_Send(&buff, 1, MPI_INT, children[i], 7, MPI_COMM_WORLD);
                    // Receive responses from children and send them to the root
                    for(int i = 0; i < nrLeaves; ++i){
                        MPI_Recv(&buff, 1, MPI_INT, MPI_ANY_SOURCE, 8, MPI_COMM_WORLD, &s);
                        MPI_Send(&buff, 1, MPI_INT, parent, 8, MPI_COMM_WORLD);
                    }
                }
            }
        }
    }

    free(neighs);
    free(children);
    free(indices);

    fclose(topF);
    fclose(imF);
    fclose(stF);

    // End MPI
    MPI_Finalize();
    return 0;
}