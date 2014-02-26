c_opencl_wrapper
==============
Simple wrapper for common OpenCL usage.



NOTES
==============

interesting error encountered:
"UNREACHABLE executed!"
(compiler bug??? (http://devgurus.amd.com/thread/159335 https://gist.github.com/ddemidov/5574346))
no, it's not... my mistake, as always =|

TO DO:
-> deal with mess with "opencl_sync()" usage!
-> cl_var should be set with the value of scalar variable, not a pointer!
-> ...
-> PROFIT
