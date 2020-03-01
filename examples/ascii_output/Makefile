WFC	= ../../bin/wfc

WFCSRC	= netconfig.wfc
CXXSRC	= $(WFCSRC:%.wfc=%.cpp) ip4_output.cpp
CXXOBJ	= $(CXXSRC:%.cpp=%.o)

.PRECIOUS: netconfig.cpp

%.cpp: %.wfc
	$(WFC) $<

ip4_output: $(CXXOBJ)
	$(CXX) -o $@ $(CXXOBJ)
