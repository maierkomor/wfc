WFC	= ../../bin/wfc
CXX	= g++
CXXFLAGS= -I../../include -Os

CXXSRC	= datatypes.cpp test_dt.cpp ../../lib/wfc_support.cpp
CXXOBJ	= $(CXXSRC:%.cpp=%.o)

test_dt: $(CXXOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(CXXOBJ)

datatypes.cpp: datatypes.wfc
	$(WFC) $<

