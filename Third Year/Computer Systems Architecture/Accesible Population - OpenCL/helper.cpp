#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>

#include "helper.hpp"

using namespace std;

/**
* Read input data
*/
void read_input_map(const char* file_name, INPUT_MAP &input_map)
{	
	FILE *ifp = NULL;
	unsigned int line_id = 0;
	char line[512];
    
	float location_lat;
	float location_lon;
	unsigned int population;

	ifp = fopen(file_name, "r");
	DIE(ifp == NULL, "could not open input file");

	while(fgets(line, sizeof(line), ifp) != NULL)
	{
		int rs = sscanf(line, "%f,%f\t%u\n", &location_lat, &location_lon, &population);
		if (rs != 3) {
			cout << "Skiped invalid line " << line_id << " " << line << endl;
			continue;
		}

		input_map.city_pop[line_id] = population;
		input_map.city_lon[line_id] = location_lon;
		input_map.city_lat[line_id] = location_lat;
		line_id++;
	}

	input_map.nr_cities = line_id;

	fclose(ifp);
}

/**
* Write output solution
*/
void write_output_solution(const char* file_name, OUTPUT_SOLUTION &output_solution)
{
	ofstream file_out;

	file_out.open (file_name);
	for(int i = 0; i < output_solution.nr_cities; i++)
		file_out << output_solution.city_accpop[i] << endl;

	file_out.close();
}

/**
 * Compute distance between 2 cities
 */
float geo_distance(float lat1, float lon1, float lat2, float lon2)
{
	float dist_lat = (lat2 - lat1) * DEGREE_TO_RADIANS;
	float dist_lon = (lon2 - lon1) * DEGREE_TO_RADIANS;
	float inter = sin(dist_lat / 2) * sin(dist_lat / 2) +
		cos(lat1 * DEGREE_TO_RADIANS) * cos(lat2 * DEGREE_TO_RADIANS) * sin(dist_lon / 2) * sin(dist_lon / 2);
	return 2 * EARTH_RADIUS * atan2(sqrt(inter), sqrt(1 - inter));
}


