WFC	= ../../bin/wfc
CXX	= g++
CXXFLAGS= -g -Os

all: shared_lib conflicting_settings one_wfc


shared_lib:
	# Generate the library with settings for message_a and message_b.
	# To share the library all instance must use the same settings.
	$(WFC) -l
	# Generate wfc and use extern library.
	$(WFC) -fwfclib=extern -o message_a_x -fgenlib=false message_a.wfc
	$(WFC) -fwfclib=extern -o message_b_x -fgenlib=false message_b.wfc
	# Now go one and compile an link with the rest of your project.
	$(CXX) $(CXXFLAGS) -c message_a_x.cpp
	$(CXX) $(CXXFLAGS) -c message_b_x.cpp
	$(CXX) $(CXXFLAGS) -c wfccore.cpp
	$(CXX) $(CXXFLAGS) -c main_x.cpp
	$(CXX) $(CXXFLAGS) -o $@ message_a_x.o message_b_x.o wfccore.o main_x.o


conflicting_settings:
	# To use conflicting settings, you need make all library functions
	# module local. Therefore, set library to static mode.
	# This example conflicts wrt. VarIntBits setting.
	# The namespace is necessary to isolate conflicting definitions from eachother.
	$(WFC) -fwfclib=static -fvarintbits=32 -fnamespace=ns_a -o message_a_s message_a.wfc
	$(WFC) -fwfclib=static -fvarintbits=64 -fnamespace=ns_b -o message_b_s message_b.wfc
	# Like this conflicting settings can be linke together.
	# Now go one and compile an link with the rest of your project.
	$(CXX) $(CXXFLAGS) -c message_a_s.cpp
	$(CXX) $(CXXFLAGS) -c message_b_s.cpp
	$(CXX) $(CXXFLAGS) -c main_s.cpp
	$(CXX) $(CXXFLAGS) -o $@ message_a_s.o message_b_s.o main_s.o


one_wfc:
	# If your project has only one WFC file, you can used the best
	# optimized variant with inline library functions. 
	$(WFC) -fwfclib=inline -o message_a_i message_a.wfc
	# Now go one and compile an link with the rest of your project.
	$(CXX) $(CXXFLAGS) -c message_a_i.cpp
	$(CXX) $(CXXFLAGS) -c main_i.cpp
	$(CXX) $(CXXFLAGS) -o $@ message_a_i.o main_i.o
