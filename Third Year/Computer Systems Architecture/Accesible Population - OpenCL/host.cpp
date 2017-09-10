#include "utils.h"
using namespace std;

void solution_opencl(INPUT_MAP &input_map,
		OUTPUT_SOLUTION &output_solution)
{
	cl_device_id device;
	cl_int ret;

	cl_context context;
	cl_command_queue cmd_queue;
	cl_program program;
	cl_kernel kernel;

	string kernel_src;

	uint platform_num = 0;
	uint device_num = 0;
	int no_cities = input_map.nr_cities;
	int lat1, lon1, lat2, lon2;

	/* setup OpenCL */
	gpu_find(device, platform_num, device_num);

	/* create a context for the device */
	context = clCreateContext(0, 1, &device, NULL, NULL, &ret);
	CL_ERR( ret );

	/* create a command queue for the device in the context */
	cmd_queue = clCreateCommandQueue(context, device, 0, &ret);
	CL_ERR( ret );

	/* allocate buffers on the CPU (HOST) */
	cl_uint *pop_host = input_map.city_pop;
	cl_float *lat_host = input_map.city_lat;
	cl_float *lon_host = input_map.city_lon;
	cl_uint *out_host = output_solution.city_accpop;
	output_solution.nr_cities = no_cities;

	/* allocate buffers on the GPU (DEVICE)*/
	cl_mem pop_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
				  sizeof(cl_uint) * no_cities, NULL, &ret);
	CL_ERR( ret );
	cl_mem lat_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
				  sizeof(cl_float) * no_cities, NULL, &ret);
	CL_ERR( ret );
	cl_mem lon_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
				  sizeof(cl_float) * no_cities, NULL, &ret);
	CL_ERR( ret );
	cl_mem out_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,
				  sizeof(cl_uint) * no_cities, NULL, &ret);
	CL_ERR( ret );

	/* retrieve kernel source */
	read_kernel("device.cl", kernel_src);
	const char* kernel_c_str = kernel_src.c_str();

	/* create kernel program from source */
	program = clCreateProgramWithSource(context, 1,
		  &kernel_c_str, NULL, &ret);
	CL_ERR( ret );

	/* compile the program for the given set of devices */
	ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	CL_COMPILE_ERR( ret, program, device );

	/* create kernel associated to compiled source kernel */
	kernel = clCreateKernel(program, "accpop", &ret);
	CL_ERR( ret );

	/* set kernel arguments */
	CL_ERR(clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&pop_dev));
	CL_ERR(clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&lat_dev));
	CL_ERR(clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&lon_dev));
	CL_ERR(clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&out_dev));
	CL_ERR(clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&no_cities));
	CL_ERR(clSetKernelArg(kernel, 5, sizeof(cl_float), (void *)&input_map.kmrange));

	/* adjust the globalSize */
	size_t globalSize = no_cities;

	/* Write buffers to GPU */
	CL_ERR( clEnqueueWriteBuffer(cmd_queue, pop_dev, CL_TRUE, 0,
		  sizeof(uint) * no_cities, pop_host, 0, NULL, NULL));
	CL_ERR( clEnqueueWriteBuffer(cmd_queue, lat_dev, CL_TRUE, 0,
		  sizeof(float) * no_cities, lat_host, 0, NULL, NULL));
	CL_ERR( clEnqueueWriteBuffer(cmd_queue, lon_dev, CL_TRUE, 0,
		  sizeof(float) * no_cities, lon_host, 0, NULL, NULL));

	/* compute accesible population */
	CL_ERR(clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL));
	/* copy the buffers back */
	CL_ERR( clEnqueueReadBuffer(cmd_queue, out_dev, CL_TRUE, 0,
		  sizeof(uint) * no_cities, out_host, 0, NULL, NULL));
}


/**
 * MAIN entry function (CPU/HOST)
 */
int main(int argc, char** argv)
{
	INPUT_MAP input_map;
	OUTPUT_SOLUTION output_solution;

	/* Check for input and setting problems */
	DIE(argc != 4, "Usage: <kmrange> <infile> <outfile>");

	istringstream(string(argv[1])) >> input_map.kmrange;

	/* Read input map */
	read_input_map(argv[2], input_map);

	/* Process solution using OpenCL */
	solution_opencl(input_map, output_solution);

	/* Write solution to output */
	write_output_solution(argv[3], output_solution);
}
