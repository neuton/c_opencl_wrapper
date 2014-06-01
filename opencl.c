#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "cl_error.h"

#include "opencl.h"

static cl_context context;
static cl_command_queue queue;
static cl_device_id device_id;
static size_t LWS[3], GWS[3];
static ushort ND;

extern void opencl_set_nd(ushort n_dim)
{
	ND = n_dim;
}

extern void opencl_set_local_ws(ushort nd, ...)
{
	ushort i;
	va_list vl;
	va_start(vl, nd);
	for (i=0; i<nd; i++)
	{
		LWS[i] = va_arg(vl, size_t);
		GWS[i] = ((GWS[i]+LWS[i]-1)/LWS[i])*LWS[i];
	}
	va_end(vl);
}

extern void opencl_set_global_ws(ushort nd, ...)
{
	ushort i;
	va_list vl;
	va_start(vl, nd);
	for (i=0; i<nd; i++)
		GWS[i] = ((va_arg(vl, size_t) + LWS[i]-1)/LWS[i])*LWS[i];
	va_end(vl);
}

extern cl_var opencl_create_var(size_t type_size, uint n, cl_mem_flags flags, const void * val)
{
	//if (n==0) clCheckError(CL_SUCCESS+1, "creating cl_var (n==0)");
	cl_var var;
	var.type_size = type_size;
	var.n = n;
	if (var.n==1)
		var.val = malloc(type_size);
	else
	{
		int err;
		var.val = malloc(sizeof(cl_mem));
		//va_list vl;
		//va_start(vl, n);
		//cl_mem_flags flags = va_arg(vl, cl_mem_flags);	// unidentified problems with this approach
		//va_end(vl);
		*((cl_mem *)var.val) = clCreateBuffer(context, flags==0 ? CL_MEM_READ_WRITE : flags, var.type_size*n, NULL, &err);
		clCheckError(err, "creating buffer");
	}
	if (val != NULL) opencl_set_var(var, val);
	return var;
}

// I may change it if I want to set var by value (not with pointer arg) (use ... in args + check var.type_size).
// so basically I may "overload" a function to receive variables with different types by using "..." (if I check for type (or type size) and use correspondent va_arg()).
extern void opencl_set_var(cl_var var, const void * val)
{
	if (var.n==1)
	{
		if (var.val == NULL)
			var.val = malloc(var.type_size);
		uint i;
		for (i=0; i<var.type_size; i++)
			var.val[i] = ((char *)val)[i];
	}
	else
	{
		if (var.val == NULL)
		{
			int err;
			var.val = malloc(sizeof(cl_mem));
			*((cl_mem *)var.val) = clCreateBuffer(context, CL_MEM_READ_WRITE, var.type_size*var.n, NULL, &err);
			clCheckError(err, "creating buffer");
		}
		clCheckError(clEnqueueWriteBuffer(queue, *((cl_mem *)var.val), CL_TRUE, 0, var.type_size*var.n, val, 0, NULL, NULL), "writing to buffer");
	}
}

extern void opencl_get_var(cl_var var, void * val)
{
	if (var.n==1)
	{
		uint i;
		for (i=0; i<var.type_size; i++)
			((char *)val)[i] = var.val[i];
	}
	else
		clCheckError(clEnqueueReadBuffer(queue, *((cl_mem *)var.val), CL_TRUE, 0, var.type_size*var.n, val, 0, NULL, NULL), "reading from buffer");
}

extern void opencl_free_var(cl_var var)
{
	if (var.n==1)
	{
		free(var.val);
		var.val = NULL;
	}
	else
	{
		clCheckError(clReleaseMemObject(*((cl_mem *)var.val)), "releasing buffer");
		free(var.val);
		var.val = NULL;
	}
}

extern void opencl_init_cpu()
{
	cl_platform_id platform_id;
	int err;
	err = clGetPlatformIDs(1, &platform_id, NULL); clCheckError(err, "getting platform id");
	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0 };
	if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL) != CL_SUCCESS)
	{
		if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL) != CL_SUCCESS)
		{
			fputs("OpenCL> Error: no capable devices found!\n", stderr);
			clCheckError(-1, "getting device id");
		}
		fputs("OpenCL> Warning: CPU device not found, using GPU instead!\n", stderr);
	}
	context = clCreateContext(cps, 1, &device_id, NULL, NULL, &err); clCheckError(err, "creating context");
	queue = clCreateCommandQueue(context, device_id, 0, &err); clCheckError(err, "creating command queue");
	GWS[0] = LWS[0] = 64;
	ND = 1;
	GWS[1] = GWS[2] = LWS[1] = LWS[2] = 0;
}

extern void opencl_init_gpu()
{
	cl_platform_id platform_id;
	int err;
	err = clGetPlatformIDs(1, &platform_id, NULL); clCheckError(err, "getting platform id");
	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0 };
	if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL) != CL_SUCCESS)
	{
		if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL) != CL_SUCCESS)
		{
			fputs("OpenCL> Error: no capable devices found!\n", stderr);
			clCheckError(-1, "getting device id");
		}
		fputs("OpenCL> Warning: GPU device not found, using CPU instead!\n", stderr);
	}
	context = clCreateContext(cps, 1, &device_id, NULL, NULL, &err); clCheckError(err, "creating context");
	queue = clCreateCommandQueue(context, device_id, 0, &err); clCheckError(err, "creating command queue");
	GWS[0] = LWS[0] = 64;
	ND = 1;
	GWS[1] = GWS[2] = LWS[1] = LWS[2] = 0;
}

//extern cl_program opencl_create_program(const char *kernel_filename, ...)
extern cl_program opencl_create_program(const char *kernel_filename, const char *options)
{
	cl_program program;
	int err;
	char * kernel_src = malloc(11+strlen(kernel_filename)), * opts = malloc(strlen(options)+5);
	strcat(strcat(strcpy(kernel_src, "#include<"), kernel_filename), ">");
	program = clCreateProgramWithSource(context, 1, (const char **)&kernel_src, NULL, &err);
	clCheckError(err, "creating 'include' program ");
	err = clBuildProgram(program, 0, NULL, strcat(strcpy(opts, options), " -I."), NULL, NULL);
	if (err == CL_BUILD_PROGRAM_FAILURE)
	{
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		if (log_size>2)
		{
			fprintf(stderr, "OpenCL> %s build log:\n", kernel_filename);
			char * build_log = (char *)malloc(log_size);
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
			fputs(build_log, stderr);
			free(build_log); build_log = NULL;
		}
	}
	clCheckError(err, "building program");
	return program;
}

extern cl_kernel opencl_create_kernel(cl_program program, const char *kernel_name)
{
	char str[144];
	sprintf(str, "creating kernel %s", kernel_name);
	int err;
	cl_kernel kernel = clCreateKernel(program, kernel_name, &err);
	clCheckError(err, str);
	return kernel;
}

extern void opencl_set_kernel_arg(cl_kernel kernel, ushort i, cl_var var)
{
	char name[128], str[152];
	clCheckError(clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 128, name, NULL), "while getting kernel name");
	sprintf(str, "setting args for kernel %s", name);
	clCheckError(clSetKernelArg(kernel, i, var.n>1 ? sizeof(cl_mem) : var.type_size, var.val), str);
}

extern void opencl_set_kernel_args(cl_kernel kernel, ...)
{
	char name[128], str[152];
	clCheckError(clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 128, name, NULL), "while getting kernel name");
	sprintf(str, "setting args for kernel %s", name);
	uint i, n;
	clCheckError(clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(uint), &n, NULL), str);
	cl_var var;
	va_list vl;
	va_start(vl, kernel);
	for (i=0; i<n; i++)
	{
		var = va_arg(vl, cl_var);
		clCheckError(clSetKernelArg(kernel, i, var.n>1 ? sizeof(cl_mem) : var.type_size, var.val), str);
	}
	va_end(vl);
}

//extern void opencl_run_kernel(cl_kernel kernel, ushort nd = ND, size_t * local_ws = LWS, size_t * global_ws = GWS)
//  |
//  |
// \|/
//  V
//extern void opencl_run_kernel(cl_kernel kernel, ...)
extern void opencl_run_kernel(cl_kernel kernel)
{
	//clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0, NULL, &n);
	char name[128], str[145];
	clCheckError(clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 128, name, NULL), "while getting kernel name");
	sprintf(str, "launching kernel %s", name);
	clCheckError(clEnqueueNDRangeKernel(queue, kernel, ND, NULL, GWS, LWS, 0, NULL, NULL), str);
}

extern void opencl_sync()
{
	clCheckError(clFinish(queue), "finishing queue");
}
