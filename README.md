# Cracker
#### A MD5 hash cracker

## Building
**Note:** There is a precompiled binary available at the [release page](https://github.com/m-lima/cracker/releases/latest)
### Dependencies
* C++ compiler
* CMake 3.2+

### Installing dependencies
To install CMake
```bash
# Mac
$ brew install cmake
```
```bash
# Linux
$ apt-get install cmake
```
```bash
# Windows
$ scoop install cmake
```

### Compiling
```bash
$ cd <repository_folder>
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```

## Running
There are two modes of operation: decoding and encoding

### Decoding
**Usage:** cracker [-t TARGETS] [-d DEVICE] [-i INPUT] [-o OUTPUT] [-r RAINBOW] [-p PREFIX] [-l LENGTH] [-n ND_RANGE] [-c THREAD_COUNT]
* TARGETS: **A list of MD5 hashes**  
  The MD5 hash is defined as a 32-digit hex value.  
  Any separators or any kind of symbol will be ignored.  
  The list may also be provided from a file or, if in a *NIX system, through piping and streaming.  
  The final list will be the concatenation of all methods.  
  If the list is piped or streamed, the "-t" argument is no longer mandatory.  
* DEVICE: **Device to execute on**  
  ***[default: AUTO]***
  * GPU - Execute in the GPU using openCL
  * CPU - Execute in the CPU using multiple threads
  * AUTO - Decide the device automatically based on the value space  
* INPUT: **A file to scan for MD5 hashes**  
  Will find all 32-digit hex values and set them as TARGETs regardless of the format
* OUTPUT: **Output file for the cracked input**  
  ***[only if INPUT is defined]***   
  All cracked hashes will be replaced in place and saved to OUTPUT
* RAINBOW: **Rainbow table**  
  File name of the rainbow table. Will be updated if new items are discovered
* PREFIX: **First digits of the value**
* LENGTH: **Length of the value**  
  ***[default: 12]***
* ND_RANGE: **Maximum size of the ND range**  
  ***[GPU only]***  
  ***[default: 10000000]***  
  Increase this in case the value is larger than 2^56
* THREAD_COUNT: **Number of threads to launch**  
  ***[CPU only]***  
  ***[default: 256]***

### Encoding
**Usage:** cracker -e VALUE
* VALUE: **The value to encode with MD5 hashing**
  
### Help
**Usage:** cracker -h
