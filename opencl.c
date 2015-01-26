#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "cl_error.h"

#include "opencl.h"

static cl_platform_id platform;
static cl_context context;
static cl_command_queue * queues;
static cl_device_id * devices;
static uint ndevices;
static uint cid;	// current device and queue id
static size_t LWS[3], GWS[3];
static ushort ND;

extern void opencl_switch_device(uint id)
{
	cid = id;
}

extern uint opencl_get_devices_number()
{
	return ndevices;
}

extern cl_context opencl_get_context()
{
	return context;
}

extern cl_command_queue opencl_get_queue()
{
	return queues[cid];
}

extern cl_platform_id opencl_get_platform()
{
	return platform;
}

extern cl_device_id opencl_get_device()
{
	return devices[cid];
}

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
		clCheckError(clEnqueueWriteBuffer(queues[cid], *((cl_mem *)var.val), CL_TRUE, 0, var.type_size*var.n, val, 0, NULL, NULL), "writing to buffer");
	}
}

extern void opencl_get_var(const cl_var var, void * val)
{
	if (var.n==1)
	{
		uint i;
		for (i=0; i<var.type_size; i++)
			((char *)val)[i] = var.val[i];
	}
	else
		clCheckError(clEnqueueReadBuffer(queues[cid], *((cl_mem *)var.val), CL_TRUE, 0, var.type_size*var.n, val, 0, NULL, NULL), "reading from buffer");
}

uint opencl_get_platforms_number()
{
	uint nplatforms;
	clCheckError(clGetPlatformIDs(0, NULL, &nplatforms), "getting platforms number");
	return nplatforms;
}

extern void opencl_init(uint platform_id, cl_device_type device_type)
{
	cl_platform_id platforms[platform_id + 1];
	clCheckError(clGetPlatformIDs(platform_id + 1, platforms, NULL), "getting platform id");
	platform = platforms[platform_id];
	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
	clCheckError(clGetDeviceIDs(platform, device_type, 0, NULL, &ndevices), "getting devices number");
	devices = (cl_device_id *)malloc(ndevices * sizeof(cl_device_id));
	int err = clGetDeviceIDs(platform, device_type, ndevices, devices, NULL);
	if (err != CL_SUCCESS)
	{
		if (device_type == CL_DEVICE_TYPE_GPU)
			fputs("OpenCL> Error: no capable GPU device found!\n", stderr);
		else if (device_type == CL_DEVICE_TYPE_CPU)
			fputs("OpenCL> Error: no capable CPU device found!\n", stderr);
		else
			fputs("OpenCL> Error: no capable device found!\n", stderr);
		clCheckError(err, "getting devices ids");
	}
	cid = 0;
	context = clCreateContext(cps, ndevices, devices, NULL, NULL, &err); clCheckError(err, "creating context");
	queues = (cl_command_queue *)malloc(ndevices * sizeof(cl_command_queue));
	uint i;
	for (i=0; i < ndevices; i++)
		queues[i] = clCreateCommandQueue(context, devices[i], 0, &err); clCheckError(err, "creating command queue");
	GWS[0] = LWS[0] = 64;
	ND = 1;
	GWS[1] = GWS[2] = LWS[1] = LWS[2] = 0;
}

extern cl_program opencl_create_program_from_source(const char *kernel_filename, const char *options)
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
		char * build_log;
		uint i;
		for (i=0; i<ndevices; i++)
		{
			size_t log_size;
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
			if (log_size>2)
			{
				fprintf(stderr, "OpenCL> %s build log [device #%i]:\n", kernel_filename, i);
				build_log = (char *)malloc(log_size);
				clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
				fputs(build_log, stderr);
				free(build_log);
			}
		}
	}
	clCheckError(err, "building program");
	return program;
}

extern cl_program opencl_create_program_from_binary(const char *kernel_filename)
{
	cl_program program;
	int err;
	FILE * kf = fopen(kernel_filename, "r"); if (kf==NULL) clCheckError(CL_SUCCESS+1, "opening kernel file");
	size_t kfs = 0; while (fgetc(kf)!=EOF) kfs++; rewind(kf);
	char * kernel = (char *)malloc(kfs);
	uint i; for (i=0; i<kfs; i++) kernel[i] = fgetc(kf); fclose(kf);
	for (i=0; kernel_filename[i]!='.' && kernel_filename[i]!='\0'; i++);
	program = clCreateProgramWithBinary(context, ndevices,  devices, (const size_t *)&kfs, (const unsigned char **)&kernel, NULL, &err);
	clCheckError(err, "creating program from binary");
	free(kernel);
	return program;
}

extern void opencl_write_program_to_file(const cl_program program, const char * output_filename)
{
	size_t binary_size;
	clCheckError(clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &binary_size, NULL), "getting binary size");
	unsigned char * binary = (unsigned char *)malloc(binary_size);
	clCheckError(clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(char *), &binary, NULL), "getting binary");
	FILE * f = fopen(output_filename, "w");
	if (f==NULL) { fprintf(stderr, "Error opening \"%s\"!\n", output_filename); exit(EXIT_FAILURE); }
	uint i;
	for (i=0; i<binary_size; i++) fputc((int)binary[i], f);
	fclose(f);
}

extern cl_kernel opencl_create_kernel(cl_program program, const char * kernel_name)
{
	char * str = (char *)malloc(strlen(kernel_name) + 17);
	sprintf(str, "creating kernel %s", kernel_name);
	int err;
	cl_kernel kernel = clCreateKernel(program, kernel_name, &err);
	clCheckError(err, str);
	return kernel;
}

//extern cl_kernel * opencl_create_kernels(const cl_program program)
//{
//	uint n;
//	clCheckError(clCreateKernelsInProgram(program, 0, NULL, &n), "getting kernels number");
//	cl_kernel * kernels = (cl_kernel *)malloc(n * sizeof(cl_kernel));
//	clCheckError(clCreateKernelsInProgram(program, n, kernels, NULL), "creating kernels");
//	return kernels;
//}

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

extern void opencl_run_kernel(cl_kernel kernel)
{
	//clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0, NULL, &n);
	char name[128], str[145];
	clCheckError(clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 128, name, NULL), "while getting kernel name");
	sprintf(str, "launching kernel %s", name);
	clCheckError(clEnqueueNDRangeKernel(queues[cid], kernel, ND, NULL, GWS, LWS, 0, NULL, NULL), str);
}

extern void opencl_sync()
{
	clCheckError(clFinish(queues[cid]), "finishing queue");
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

extern void opencl_free_program(cl_program program)
{
	clCheckError(clReleaseProgram(program), "releasing program");
}

extern void opencl_free_kernel(cl_kernel kernel)
{
	clCheckError(clReleaseKernel(kernel), "releasing kernel");
}

//extern void opencl_free_kernels(cl_kernel * kernels, uint n)
//{
//	uint i;
//	for (i=0; i<n; i++)
//		clCheckError(clReleaseKernel(kernels[i]), "releasing kernels");
//	free(kernels);
//}

extern void opencl_done()
{
	clCheckError(clReleaseContext(context), "releasing context");
	uint i;
	for (i=0; i<ndevices; i++)
	{
		clCheckError(clReleaseCommandQueue(queues[i]), "releasing queues");
		clCheckError(clReleaseDevice(devices[i]), "releasing devices");
	}
	free(queues);
	free(devices);
}
