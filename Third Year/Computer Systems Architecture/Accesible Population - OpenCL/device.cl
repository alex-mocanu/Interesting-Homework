#define DEGREE_TO_RADIANS       0.0174533f
#define EARTH_RADIUS            6371.f

float geo_distance(float lat1, float lon1, float lat2, float lon2)
{
    float dist_lat = (lat2 - lat1) * DEGREE_TO_RADIANS;
    float dist_lon = (lon2 - lon1) * DEGREE_TO_RADIANS;
    float inter = sin(dist_lat / 2) * sin(dist_lat / 2) +
        cos(lat1 * DEGREE_TO_RADIANS) * cos(lat2 * DEGREE_TO_RADIANS) * sin(dist_lon / 2) * sin(dist_lon / 2);
    return 2 * EARTH_RADIUS * atan2(sqrt(inter), sqrt(1 - inter));
}

__kernel void
accpop(__global uint* population,
        __global float* lat,
        __global float* lon,
        __global uint* out,
        int size,
        float range)
{
    int i;
    int id = get_global_id(0);
    uint total_population = 0;

    for(i = 0; i < size; ++i)
        if(geo_distance(lat[id], lon[id], lat[i], lon[i]) <= range)
            total_population += population[i];

    out[id] = total_population;
}
