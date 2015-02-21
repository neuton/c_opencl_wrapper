Opencl C API wrapper
==============
Simple wrapper for common OpenCL usage.

Usage example
--------------

`host.c`:

```c
#include <stdio.h>
#include "opencl.h"

main()
{
	uint i, n = 1000;
	real * v = (real *)malloc(n * sizeof(real));
	for (i=0; i<n; i++) v[i] = (real)i;
	
	// mandatory initialization
	opencl_init(0, CL_DEVICE_TYPE_GPU);
	
	// creating variables on device (and enqueing data copy)
	cl_var dn = opencl_create_var(sizeof(uint), 1, CL_MEM_READ_ONLY, &n);
	cl_var dv = opencl_create_var(sizeof(real), n, 0, v);
	
	// reading program from file
	cl_program program = opencl_create_program_from_source("kernel.cl", "");
	
	// creating kernel
	cl_kernel kernel = opencl_create_kernel(program, "test_kernel");
	
	// setting kernel args
	opencl_set_kernel_args(kernel, dn, dv);
	
	// setting workgroup sizes
	opencl_set_global_ws(1, n);
	opencl_set_local_ws(1, 64);
	
	// running the kernel
	opencl_run_kernel(kernel);
	
	// getting data back from device (blocking read)
	opencl_get_var(dv, v);
	
	// cleaning
	opencl_done();
	
	char success = 1;
	for (i=0; i<n; i++) success &= v[i] == (real)(2*i);
	puts(success ? "Success" : "Fail");
}
```

`kernel.cl`:

```c
#define real float

__kernel void test_kernel(uint n, __global real * v)
{
	v[get_global_id(0)] *= 2;
}
```

`Makefile`:

```makefile
CFLAGS = -I"$(AMDAPPSDKROOT)/include"
LDFLAGS = -L"$(AMDAPPSDKROOT)/lib/x86_64" -lOpenCL

EXEC = host

ifeq ($(OS), Windows_NT)
EXEC := $(EXEC).exe
SHELL = cmd
else
SHELL = /bin/sh
endif

$(EXEC): host.o cl_error.o opencl.o
host.o: host.c opencl.h Makefile
opencl.o: opencl.c opencl.h cl_error.h Makefile
cl_error.o: cl_error.c cl_error.h Makefile
```
