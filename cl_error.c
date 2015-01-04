#include "cl_error.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void clGetErrorString(short int error, char * string)
{
	switch (error)
	{
		case CL_SUCCESS: strcpy(string, "SUCCESS"); break;
		case CL_DEVICE_NOT_FOUND: strcpy(string, "DEVICE_NOT_FOUND"); break;
		case CL_DEVICE_NOT_AVAILABLE: strcpy(string, "DEVICE_NOT_AVAILABLE"); break;
		case CL_COMPILER_NOT_AVAILABLE: strcpy(string, "COMPILER_NOT_AVAILABLE"); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: strcpy(string, "MEM_OBJECT_ALLOCATION_FAILURE"); break;
		case CL_OUT_OF_RESOURCES: strcpy(string, "OUT_OF_RESOURCES"); break;
		case CL_OUT_OF_HOST_MEMORY: strcpy(string, "OUT_OF_HOST_MEMORY"); break;
		case CL_PROFILING_INFO_NOT_AVAILABLE: strcpy(string, "PROFILING_INFO_NOT_AVAILABLE"); break;
		case CL_MEM_COPY_OVERLAP: strcpy(string, "MEM_COPY_OVERLAP"); break;
		case CL_IMAGE_FORMAT_MISMATCH: strcpy(string, "IMAGE_FORMAT_MISMATCH"); break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED: strcpy(string, "IMAGE_FORMAT_NOT_SUPPORTED"); break;
		case CL_BUILD_PROGRAM_FAILURE: strcpy(string, "BUILD_PROGRAM_FAILURE"); break;
		case CL_MAP_FAILURE: strcpy(string, "MAP_FAILURE"); break;
		case CL_INVALID_VALUE: strcpy(string, "INVALID_VALUE"); break;
		case CL_INVALID_DEVICE_TYPE: strcpy(string, "INVALID_DEVICE_TYPE"); break;
		case CL_INVALID_PLATFORM: strcpy(string, "INVALID_PLATFORM"); break;
		case CL_INVALID_DEVICE: strcpy(string, "INVALID_DEVICE"); break;
		case CL_INVALID_CONTEXT: strcpy(string, "INVALID_CONTEXT"); break;
		case CL_INVALID_QUEUE_PROPERTIES: strcpy(string, "INVALID_QUEUE_PROPERTIES"); break;
		case CL_INVALID_COMMAND_QUEUE: strcpy(string, "INVALID_COMMAND_QUEUE"); break;
		case CL_INVALID_HOST_PTR: strcpy(string, "INVALID_HOST_PTR"); break;
		case CL_INVALID_MEM_OBJECT: strcpy(string, "INVALID_MEM_OBJECT"); break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: strcpy(string, "INVALID_IMAGE_FORMAT_DESCRIPTOR"); break;
		case CL_INVALID_IMAGE_SIZE: strcpy(string, "INVALID_IMAGE_SIZE"); break;
		case CL_INVALID_SAMPLER: strcpy(string, "INVALID_SAMPLER"); break;
		case CL_INVALID_BINARY: strcpy(string, "INVALID_BINARY"); break;
		case CL_INVALID_BUILD_OPTIONS: strcpy(string, "INVALID_BUILD_OPTIONS"); break;
		case CL_INVALID_PROGRAM: strcpy(string, "INVALID_PROGRAM"); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: strcpy(string, "INVALID_PROGRAM_EXECUTABLE"); break;
		case CL_INVALID_KERNEL_NAME: strcpy(string, "INVALID_KERNEL_NAME"); break;
		case CL_INVALID_KERNEL_DEFINITION: strcpy(string, "INVALID_KERNEL_DEFINITION"); break;
		case CL_INVALID_KERNEL: strcpy(string, "INVALID_KERNEL"); break;
		case CL_INVALID_ARG_INDEX: strcpy(string, "INVALID_ARG_INDEX"); break;
		case CL_INVALID_ARG_VALUE: strcpy(string, "INVALID_ARG_VALUE"); break;
		case CL_INVALID_ARG_SIZE: strcpy(string, "INVALID_ARG_SIZE"); break;
		case CL_INVALID_KERNEL_ARGS: strcpy(string, "INVALID_KERNEL_ARGS"); break;
		case CL_INVALID_WORK_DIMENSION: strcpy(string, "INVALID_WORK_DIMENSION"); break;
		case CL_INVALID_WORK_GROUP_SIZE: strcpy(string, "INVALID_WORK_GROUP_SIZE"); break;
		case CL_INVALID_WORK_ITEM_SIZE: strcpy(string, "INVALID_WORK_ITEM_SIZE"); break;
		case CL_INVALID_GLOBAL_OFFSET: strcpy(string, "INVALID_GLOBAL_OFFSET"); break;
		case CL_INVALID_EVENT_WAIT_LIST: strcpy(string, "INVALID_EVENT_WAIT_LIST"); break;
		case CL_INVALID_EVENT: strcpy(string, "INVALID_EVENT"); break;
		case CL_INVALID_OPERATION: strcpy(string, "INVALID_OPERATION"); break;
		case CL_INVALID_GL_OBJECT: strcpy(string, "INVALID_GL_OBJECT"); break;
		case CL_INVALID_BUFFER_SIZE: strcpy(string, "INVALID_BUFFER_SIZE"); break;
		case CL_INVALID_MIP_LEVEL: strcpy(string, "INVALID_MIP_LEVEL"); break;
		case CL_INVALID_GLOBAL_WORK_SIZE: strcpy(string, "CL_INVALID_GLOBAL_WORK_SIZE"); break;
		#ifdef CL_PLATFORM_NOT_FOUND_KHR
		case CL_PLATFORM_NOT_FOUND_KHR: strcpy(string, "CL_PLATFORM_NOT_FOUND_KHR"); break;
		#endif
		default: strcpy(string, "UNKNOWN");
	}
}

extern short int clSoftCheckError(short int error, const char * message)
{
	if (error != CL_SUCCESS)
	{
		char ers[42];
		clGetErrorString(error, ers);
		fprintf(stderr, "OpenCL> Error %s: code %d (%s)\n", message, error, ers);
	}
	return error;
}

extern void clCheckError(short int error, const char * message)
{
	if (clSoftCheckError(error, message) != CL_SUCCESS)
	{
		puts("Execution terminated.");
		exit(EXIT_FAILURE);
	}
}
