#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

#define DEBUG_PRINT_ON	1

/**
 * Problem characteristics
 */
#define DEGREE_TO_RADIANS		0.0174533f
#define EARTH_RADIUS			6371.f
#define MAX_CITIES              300000

struct INPUT_MAP
{
    int nr_cities;
	float kmrange;
	unsigned int city_pop[MAX_CITIES];
	float city_lon[MAX_CITIES];
	float city_lat[MAX_CITIES];
};

struct OUTPUT_SOLUTION
{
    int nr_cities;
	unsigned int city_accpop[MAX_CITIES];
};

/**
 * MACRO error check
 */
#define DIE(assertion, call_description)                    \
do {                                                        \
    if (assertion) {                                        \
            fprintf(stderr, "(%d): ",                       \
                            __LINE__);                      \
            perror(call_description);                       \
            exit(EXIT_FAILURE);                             \
    }                                                       \
} while(0);

#if DEBUG_PRINT_ON
#define DEBUG_PRINT(...) do{ fprintf( stdout, __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


/**
 * General functions
 */
void read_input_map(const char* file_name, INPUT_MAP &input_map);
void write_output_solution(const char* file_name, OUTPUT_SOLUTION &output_solution);

float geo_distance(float lat1, float lon1, float lat2, float lon2);

