#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#ifndef int
	#define int cl_int
#endif

#ifndef uint
	#define uint cl_uint
#endif

#ifndef ushort
	#define ushort cl_ushort
#endif

#ifndef real
	#define real cl_float
#endif

typedef struct {size_t type_size; uint n; char * val;} cl_var;

uint opencl_get_platforms_number();
void opencl_init(uint platform_id, cl_device_type device_type);

void opencl_switch_device(uint id);
uint opencl_get_devices_number();

cl_context opencl_get_context();
cl_command_queue opencl_get_queue();
cl_platform_id opencl_get_platform();
cl_device_id opencl_get_device();

void opencl_set_nd(ushort n_dim);
void opencl_set_local_ws(ushort nd, ...);
void opencl_set_global_ws(ushort nd, ...);

cl_var opencl_create_var(size_t type_size, uint n, cl_mem_flags flags, const void * val);
void opencl_set_var(cl_var var, const void * val);
void opencl_get_var(const cl_var var, void * val);

cl_program opencl_create_program_from_source(const char * kernel_filename, const char *options);
cl_program opencl_create_program_from_binary(const char * kernel_filename, const char *options);
void opencl_write_program_to_file(const cl_program program, const char * output_filename);

cl_kernel opencl_create_kernel(const cl_program program, const char * kernel_name);
//cl_kernel * opencl_create_kernels(const cl_program program);
void opencl_set_kernel_arg(cl_kernel kernel, ushort i, cl_var var);
void opencl_set_kernel_args(cl_kernel kernel, ...);
void opencl_run_kernel(const cl_kernel kernel);

void opencl_sync();

void opencl_free_var(cl_var var);
void opencl_free_program(cl_program program);
void opencl_free_kernel(cl_kernel kernel);
//void opencl_free_kernels(cl_kernel * kernels, uint n);	// bad thing

void opencl_done();
