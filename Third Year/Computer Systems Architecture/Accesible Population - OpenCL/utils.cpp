#include "utils.h"
using namespace std;

/**
 * User/host function, check OpenCL function return code
 */
int CL_ERR(int cl_ret)
{
    if(cl_ret != CL_SUCCESS){
        cout << endl << cl_get_string_err(cl_ret) << endl;
        return 1;
    }
    return 0;
}

/**
 * User/host function, check OpenCL compilation return code
 */
int CL_COMPILE_ERR(int cl_ret, cl_program program, cl_device_id device)
{
    if(cl_ret != CL_SUCCESS){
        cout << endl << cl_get_string_err(cl_ret) << endl;
        cl_get_compiler_err_log(program, device);
        return 1;
    }
    return 0;
}

/**
* Read kernel from file
*/
void read_kernel(string file_name, string &str_kernel)
{
    ifstream in_file(file_name.c_str());
    in_file.open(file_name.c_str());
    DIE( !in_file.is_open(), "ERR OpenCL kernel file. Same directory as binary ?" );

    stringstream str_stream;
    str_stream << in_file.rdbuf();

    str_kernel = str_stream.str();
}

/**
 * OpenCL return error message, used by CL_ERR and CL_COMPILE_ERR
 */
const char* cl_get_string_err(cl_int err) {
switch (err) {
  case CL_SUCCESS:                      return  "Success!";
  case CL_DEVICE_NOT_FOUND:               return  "Device not found.";
  case CL_DEVICE_NOT_AVAILABLE:           return  "Device not available";
  case CL_COMPILER_NOT_AVAILABLE:         return  "Compiler not available";
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:  return  "Memory object alloc fail";
  case CL_OUT_OF_RESOURCES:               return  "Out of resources";
  case CL_OUT_OF_HOST_MEMORY:             return  "Out of host memory";
  case CL_PROFILING_INFO_NOT_AVAILABLE:   return  "Profiling information N/A";
  case CL_MEM_COPY_OVERLAP:               return  "Memory copy overlap";
  case CL_IMAGE_FORMAT_MISMATCH:          return  "Image format mismatch";
  case CL_IMAGE_FORMAT_NOT_SUPPORTED:     return  "Image format no support";
  case CL_BUILD_PROGRAM_FAILURE:          return  "Program build failure";
  case CL_MAP_FAILURE:                    return  "Map failure";
  case CL_INVALID_VALUE:                  return  "Invalid value";
  case CL_INVALID_DEVICE_TYPE:            return  "Invalid device type";
  case CL_INVALID_PLATFORM:               return  "Invalid platform";
  case CL_INVALID_DEVICE:                 return  "Invalid device";
  case CL_INVALID_CONTEXT:                return  "Invalid context";
  case CL_INVALID_QUEUE_PROPERTIES:       return  "Invalid queue properties";
  case CL_INVALID_COMMAND_QUEUE:          return  "Invalid command queue";
  case CL_INVALID_HOST_PTR:               return  "Invalid host pointer";
  case CL_INVALID_MEM_OBJECT:             return  "Invalid memory object";
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:return  "Invalid image format desc";
  case CL_INVALID_IMAGE_SIZE:             return  "Invalid image size";
  case CL_INVALID_SAMPLER:                return  "Invalid sampler";
  case CL_INVALID_BINARY:                 return  "Invalid binary";
  case CL_INVALID_BUILD_OPTIONS:          return  "Invalid build options";
  case CL_INVALID_PROGRAM:                return  "Invalid program";
  case CL_INVALID_PROGRAM_EXECUTABLE:     return  "Invalid program exec";
  case CL_INVALID_KERNEL_NAME:            return  "Invalid kernel name";
  case CL_INVALID_KERNEL_DEFINITION:      return  "Invalid kernel definition";
  case CL_INVALID_KERNEL:                 return  "Invalid kernel";
  case CL_INVALID_ARG_INDEX:              return  "Invalid argument index";
  case CL_INVALID_ARG_VALUE:              return  "Invalid argument value";
  case CL_INVALID_ARG_SIZE:               return  "Invalid argument size";
  case CL_INVALID_KERNEL_ARGS:            return  "Invalid kernel arguments";
  case CL_INVALID_WORK_DIMENSION:         return  "Invalid work dimension";
  case CL_INVALID_WORK_GROUP_SIZE:        return  "Invalid work group size";
  case CL_INVALID_WORK_ITEM_SIZE:         return  "Invalid work item size";
  case CL_INVALID_GLOBAL_OFFSET:          return  "Invalid global offset";
  case CL_INVALID_EVENT_WAIT_LIST:        return  "Invalid event wait list";
  case CL_INVALID_EVENT:                  return  "Invalid event";
  case CL_INVALID_OPERATION:              return  "Invalid operation";
  case CL_INVALID_GL_OBJECT:              return  "Invalid OpenGL object";
  case CL_INVALID_BUFFER_SIZE:            return  "Invalid buffer size";
  case CL_INVALID_MIP_LEVEL:              return  "Invalid mip-map level";
  default:                                return  "Unknown";
  }
}

/**
 * Check compiler return code, used by CL_COMPILE_ERR
 */
void cl_get_compiler_err_log(cl_program program, cl_device_id device)
{
    char* build_log;
    size_t log_size;

    /* first call to know the proper size */
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                          0, NULL, &log_size);
    build_log = new char[ log_size + 1 ];

    /* second call to get the log */
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                          log_size, build_log, NULL);
    build_log[ log_size ] = '\0';
    cout << endl << build_log << endl;
}

/**
 * Retrieve GPU device
 */
void gpu_find(cl_device_id &device,
              uint platform_select,
              uint device_select)
{
    cl_platform_id platform;
    cl_uint platform_num = 0;
    cl_platform_id* platform_list = NULL;

    cl_uint device_num = 0;
    cl_device_id* device_list = NULL;

    size_t attr_size = 0;
    cl_char* attr_data = NULL;

    /* get num of available OpenCL platforms */
    CL_ERR( clGetPlatformIDs(0, NULL, &platform_num));
    platform_list = new cl_platform_id[platform_num];
    DIE(platform_list == NULL, "alloc platform_list");

    /* get all available OpenCL platforms */
    CL_ERR( clGetPlatformIDs(platform_num, platform_list, NULL));

    /* list all platforms and VENDOR/VERSION properties */
    for(uint platf=0; platf<platform_num; platf++)
    {
        /* get attribute CL_PLATFORM_VENDOR */
        CL_ERR( clGetPlatformInfo(platform_list[platf],
            CL_PLATFORM_VENDOR, 0, NULL, &attr_size));
        attr_data = new cl_char[attr_size];
        DIE(attr_data == NULL, "alloc attr_data");

        /* get data CL_PLATFORM_VENDOR */
        CL_ERR( clGetPlatformInfo(platform_list[platf],
            CL_PLATFORM_VENDOR, attr_size, attr_data, NULL));
        delete[] attr_data;

        /* get attribute size CL_PLATFORM_VERSION */
        CL_ERR( clGetPlatformInfo(platform_list[platf],
            CL_PLATFORM_VERSION, 0, NULL, &attr_size));
        attr_data = new cl_char[attr_size];
        DIE(attr_data == NULL, "alloc attr_data");
        CL_ERR( clGetPlatformInfo(platform_list[platf],
            CL_PLATFORM_VERSION, attr_size, attr_data, NULL));
        delete[] attr_data;

        /* no valid platform found */
        platform = platform_list[platf];
        DIE(platform == 0, "platform selection");

        /* get num of available OpenCL devices type GPU on the selected platform */
        CL_ERR( clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &device_num));
        device_list = new cl_device_id[device_num];
        DIE(device_list == NULL, "alloc devices");

        /* get all available OpenCL devices type GPU on the selected platform */
        // TODO ex1 - filter and get only GPU type devices
        CL_ERR( clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL,
            device_num, device_list, NULL));

        /* list all devices and TYPE/VERSION properties */
        for(uint dev=0; dev<device_num; dev++)
            /* select device based on arguments */
            if((platf == platform_select) && (dev == device_select)){
                device = device_list[dev];
                break;
            }
    }

    delete[] platform_list;
    delete[] device_list;
}
