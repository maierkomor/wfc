Wire Format Compiler (WFC)
==========================

Introduction
------------
Have you ever written code for storing data or sending it via a network?
Providing backward and forward compatibility, a stable API and independence of
the platform are the biggest challenges during this task.

WFC is a tool that lets you handle these challenges easily by translating data
structure specifications to C++ code. The generated code is especially designed
for use on embedded systems, and WFC therefore provides several options to
allow further tuning of the generated code according to project requirements.

What is WFC?
------------
Wire Format Compiler (WFC) is a tool that generates C++ code from data
structure descriptions for handling serialization, deserialization and access
to the specified data. The generated code provide an API for accessing the data
with get, set, and clear functions and serializing it to byte stream and
restoring the data structures from a byte stream in memory.

Additionally, further interfaces exist to send the data directly as a byte
stream to a device such as a UART, to generate an ASCII output, and to generate
a JSON representation. The generated code is designed to work well on embedded
systems and therefore requires no external library and relies only on standard
interfaces available on typical embedded systems. The byte streams are endian
neutral and can be used for data exchange between platforms of differing native
integer size. 

Further options allow tuning of serveral aspects of the generated code, to
allow its use on small embedded systems down to 8-bit devices. The default code
relies on C++ STL classes such as std::vector, but it can be modified to use
arrays with fixed allocated memory instead.

Features
--------
The provided features and generated code are optimized for use on embedded
systems. The serialized data can be written to FLASH, EEPROM, a serial
connection (e.g. RS232) or a network packets using multiple different ways
(i.e. directly to RAM, via a helper function, with a helper class). The
generated code has a *very small footprint*.

Additionally, serveral more features exist that provide the ability to modify
multiple aspects of the generated code. E.g.:
- setting the fundamental bitsizes of integers and variable length integers
- optimized handling of 8- and 16-bit datatypes
- use of unaligend little endian access
- use of fixed allocated arrays instead for vectors for repeated fields
- overriding of data types for strings, bytes
- generation of subclasses instead of name mangled classes
- setting a base class of message classes
- new concept for options that allows easy switching between option sets
- operator generation for comparison of objects
- JSON output of data objects
- setting of a serialization terminator byte
- suppression of code generation for unused fields
- selection of error handling concepts
- naming of most functions
- advanced enumeration naming concept

Author
------
Thomas Maier-Komor

Contact
-------
e-mail: thomas@maier-komor.de


Building WFC:
=============
Prerequisites for building WFC are:
- bison >= 3.0.4
- flex >= 2.6.4

Linux:
------
simply type "./configure" and "make" to build for Linux.

MinGW:
------
To build wfc.exe for MinGW/Windows, run "make mingw" or "make
mingw64" on either a Linux machine or Cygwin environment. Building on
MinGW directly will not work because of outdated flex and bison.

Other OSs:
----------
Other OSs have not been tested and will most like not build without
adjustments. At lease you will need flex and bison of the minimum
version.


Getting started
===============
For getting started, take a look at the examples in the examples directory and
read the (incomplete) documentation in the doc directory. Also the
documentation of Google Protocol Buffers might be helpful.  Addtionally, you
might also take a look at the test cases, and use option `-h` to list all
available flags of WFC.


Compatibility
=============
The syntax of WFC has been derived by the one of Google's Protocol Buffers
version 2 (ProtoV2). WFC syntax is also backward compatible to Google Protocol
Buffers, but does not offer all features that are provided by Protocol Buffers,
due to the focus on embedded systems.

Also, the generated data streams are compatible between both implementations,
provided that the source .wfc and .proto files are identical. The serialized
data of WFC generated code is able to deal with data coming from a 64 bit
system on a 16 bit system, provided that the receiving system has enough
memory.

As, WFC provides several extensions for embedded systems over Google Protocol
Buffers, data serialized by WFC generated code is likely to be unparsable by
Google Protocol Buffer. I.e. it might be impossible to come up with a .proto
file that is compatible with a .wfc file. 

Supported features of Google Protocol Buffers:
----------------------------------------------
- data types:
	int32, uint32, sint32, int64, uint64, sint64,
	bool, float, double, fixed32, fixed64,
	string, bytes, enums
- optional, required, repeated, and packed repeated fields
- nested messages

Extensions over Google Protocol Buffers:
----------------------------------------
- data types: fixed8, fixed16
- optimization for readability/review, speed, size
- endian specific optimization
- target specific varint\_t definition


Missing features of Google Protocol Buffers:
--------------------------------------------
- extensions
- groups
- services (cc\_generic\_services, et al.)
- target languages: C, C#, Java, Python, Go
- arena allocations
- message set wire format
- extensions keyword
- deprecated keyword
- reserved keyword
- custom options
- protoV3:
	- import keyword
	- message type any
	- oneof
	- maps
	- packages


WFC core functions and library
==============================
The core functions are generated depending on the selected options. These
functions can be generated as inline functions (`-fwfclib=inline`), as static
functions (`-wfclib=static`), or in dedicated files (`wfccore.cpp` and
`wfccore.h`) that can be used to share the core functionality among multiple
.wfc definitions within one project.

If you have a single .wfc file in your project, you can live with the defaults.
If you intend to share the core library, you must make sure that you use the
same flags for all .wfc file compilations.

In addition, the lib and include directories contain files with support
functions. These functions are independent of the flags passed to WFC and can
therefore be shared easily without any further measures. The support functions
are only needed if you intend to use ASCII or hex output generation, or make
use of C++ I/O-stream operators.


