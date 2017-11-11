
# papi-wrapper version 3.0.0

© 2017 Sean Chester

_C++ library to simplify multi-core profiling of software with PAPI._

[![license](https://img.shields.io/github/license/mashape/apistatus.svg?style=plastic)](LICENSE)

------------------------------------


## Table of Contents 

  * [Introduction](#introduction)
  * [Requirements](#requirements)
  * [Installation](#installation)
  * [Documentation](#documentation)
  * [License](#license)
  * [Contact](#contact)
  * [Acknowledgements](#acknowledgements)
  

------------------------------------
## Introduction

[PAPI](http://icl.cs.utk.edu/papi/) is an API for counting hardware 
events (e.g., cache misses and branch mispredictions) from inside C++ 
software. With PAPI, one can determine _why_ a bottleneck/hot spot 
exists in the application. 
The [Intel developer's guide](http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-optimization-manual.html) 
provides a very good overview of how profiling the hardware events 
on an Intel machine can improve one's understanding of why a particular 
application performs the way that it does.

PAPI supports multi-core profiling, but there is some programmatic 
overhead. This library adds another layer of abstraction to PAPI 
to simplify profiling of OpenMP code. The sections below describe 
how to use this library, and there is a minimum working example 
[src/mwe.cpp](src/mwe.cpp) included. 

------------------------------------
## Requirements

This library has the following dependencies:

 * OpenMP for parallelization
 * PAPI for the production of performance metrics
 * C++ 11 for newer programming features such as templates and auto types


------------------------------------
## Usage

There are two ways to use this library:
 
 * you can either start from the working example (mwe.cpp) or 

 * you can include [src/papi_wrapper.h](src/papi_wrapper.h) and 
 compile and link [src/papi_wrapper.cpp](src/papi_wrapper.cpp). 


### Starting from the working example

Compile and link the working example with the makefile by navigating 
to this directory and executing the command:

> make all

This will create a binary at [bin/papi-wrapper-mwe](bin/papi-wrapper-mwe), 
assuming that you have PAPI installed and `papi.h` on your include path and 
that you are using a `C++` compiler that supports OpenMP (such as the latest 
GNU compiler). You can get usage instructions for the mwe by executing: 

> ./bin/papi-wrapper-mwe

The mwe simply sums a bunch of integers in parallel but tracks a set 
of hardware counters that are specified on the command line. For example, 
the following command would sum the integers with 4 concurrent threads and 
track the three comma-separated counters:

> ./bin/papi-wrapper-mwe -t 4 -p "custom" 
> -c "UOPS_EXECUTED:PORT_0 UOPS_EXECUTED:PORT_1 UOPS_EXECUTED:PORT_2"

The output will be a tab-separated list of long integers, echoed to 
`stdout` with one integer corresponding to each counter (in the same 
order that they were specified on the command line). The number 
corresponds to the sum of the values from every thread.

The library imposes no limitations on the number of counters that are 
tracked, but note that the hardware does. You can verify the limits of 
your machine with the commands `papi_avail` and `papi_native_avail`, which, 
respectively, show the preset events and the native events that are 
possible to track on the current architecture. 

Alternatively, you can profile a pre-selected set of hardware events: 

 * **branch**: # of branch hits, # of branch misses, # of branch 
instructions, and branch misprediction ratio (percentage of branch 
instructions that are mispredicted).

 * **throughput**: # of instructions, # of cycles, cycles per 
 instruction (CPI)

 * **cache**: # L2 misses, # L2 accesses, L2 miss ratio, # 
 L3 misses, # L3 accesses, L3 miss ratio

 * **tlb**: # Data TLB misses, Instruction TLB misses

To measure throughput, for example, execute the command:

> ./bin/papi-wrapper-mwe -t 4 -p "throughput"

To get up and running with your own code, simply replace lines 96-104 
(which perform the parallel sum of random integers) with the code that you 
wish to monitor. Then recompile and relink (using the makefile again). 

### Including the library in your own code

To add the papi-wrapper to an existing code base, include 
[src/papi_wrapper.h](src/papi_wrapper.h) in any file that invokes papi-wrapper 
library calls and link [src/papi_wrapper.cpp](src/papi_wrapper.cpp) in the 
executable. Ensure that the regular PAPI header file, `papi.h`, is also on your 
include path. Follow the following steps:

 * Be sure to call `papi_init( num_threads );` **once** in the application prior 
 to any profiling, indicating how many threads will be used. This will perform 
 the initialisation of PAPI for every thread.
 
 * Create an array of papi objects, one for each thread. The papi objects can 
 be of any of the predefined types (`papi_branch`, `papi_cache`, `papi_cycles`, 
 `papi_instructions_` or `papi_tlbs`). Alternatively, it can be a `papi_custom` 
 object which takes a space-delimited string of the hardware events that should 
 be tracked. Using pointers of the type `papi_base*` permits polymorphic treatment 
 of the different subclasses.
 
 * Any code that should be profiled should be nested within the static library 
 methods `start_papi_array` and `stop_papi_array`. It is possible to start and 
 stop the counters more than once.
 
 * You can append any of the papi objects to a stream with the (overloaded) 
 stream operator `<<`. If you have an array of papi objects (as in the case 
 of multi-threaded code), it will copy the results of the first object in 
 the array. Calling the static library method `sum_papi_array` will first 
 sum all the papi objects (from each thread/element of the array). Thus, 
 you will probably want to call this method first before appending the objects 
 to an output stream.
 
 * For more details, including alternate library methods not discussed here, 
 consult the documentation or [src/papi_wrapper.h](src/papi_wrapper.h) header file.

------------------------------------
## Documentation

The code has been documented for `doxygen`. If the `doc/html/` 
directory is empty or stale, you can regenerate the documentation 
by running `doxygen Doxyfile` from within the `doc/` subdirectory.
The `doxygen` settings are included in [doc/Doxyfile](doc/Doxyfile) 
and can be freely modified to suit your preferences.


------------------------------------
## License

Copyright (c) 2017 Sean Chester

papi-wrapper, version 3.0.0, is distributed freely under the *MIT License*:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


------------------------------------
## Contact

Please do not hesitate to contact me if 
you have comments, questions, or bugs to report, or just to mention 
that you found this software useful. The easiest way to contact me is 
at [papi-wrapper on GitHub](https://github.com/sean-chester/papi-wrapper). 

------------------------------------
## Acknowledgements

*papi-wrapper* is based on an initial wrapper library developed by 
Kenneth S. Bøgh, Matthias Rav, and Manuel R. Ciosici at Aarhus University.

------------------------------------
