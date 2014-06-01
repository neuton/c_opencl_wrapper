#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#define int cl_int
#define uint cl_uint
#define ushort cl_ushort
#define real cl_float

//typedef enum {CPU, GPU} cl_device_type_id;
//typedef enum {t_cl_int, t_cl_uint, t_cl_ushort, t_cl_float, t_cl_double, t_cl_mem} cl_type;
typedef struct {size_t type_size; uint n; char * val;} cl_var;

void opencl_set_nd(ushort n_dim);
void opencl_set_local_ws(ushort nd, ...);
//void opencl_set_local_ws(size_t sx, size_t sy, size_t sz);
void opencl_set_global_ws(ushort nd, ...);
//void opencl_set_global_ws(size_t sx, size_t sy, size_t sz);

//cl_var opencl_create_var(size_t type_size, uint n, cl_mem_flags flags, void * val);
//cl_var opencl_create_var(size_t type_size, uint n, ...);
cl_var opencl_create_var(size_t type_size, uint n, cl_mem_flags flags, const void * val);
void opencl_set_var(cl_var var, const void * val);
void opencl_get_var(cl_var var, void * val);
void opencl_free_var(cl_var var);

//void opencl_init(cl_device_type_id device_type_id);
void opencl_init_cpu();
void opencl_init_gpu();

cl_program opencl_create_program(const char *kernel_filename, const char *options);
//void opencl_free_program(cl_program program);

cl_kernel opencl_create_kernel(cl_program program, const char *kernel_name);
//void opencl_free_kernel(cl_kernel kernel);

void opencl_set_kernel_arg(cl_kernel kernel, ushort i, cl_var var);
void opencl_set_kernel_args(cl_kernel kernel, ...);

//void opencl_run_kernel(cl_kernel kernel, ushort nd, size_t * lws, size_t * gws);
void opencl_run_kernel(cl_kernel kernel);

void opencl_sync();
