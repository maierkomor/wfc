CXXFLAGS	= -Os -g -I../../include
CXXSRCS		= main_stream.cpp syscfg_efix.cpp syscfg_edyn.cpp
CXXOBJS		= $(CXXSRCS:%.cpp=%.o)
DEPS		= $(CXXSRCS:%.cpp=.%.d)

all: pc edyn efix

.%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MG -MT$(@:.%.d=%.o) -MM $< > $@

syscfg_pc.cpp: syscfg.wfc
	../../bin/wfc -tpc -o syscfg_pc syscfg.wfc

syscfg_edyn.cpp: syscfg.wfc
	../../bin/wfc -tembedded_dyn -o syscfg_edyn syscfg.wfc


syscfg_efix.cpp: syscfg.wfc
	../../bin/wfc -tembedded_fix -o syscfg_efix syscfg.wfc

main_edyn.o: main_posix.cpp syscfg_edyn.cpp
	$(CXX) $(CXXFLAGS) -c -include syscfg_edyn.h main_posix.cpp -o $@

main_efix.o: main_posix.cpp syscfg_efix.cpp
	$(CXX) $(CXXFLAGS) -c -include syscfg_efix.h main_posix.cpp -o $@


pc: syscfg_pc.o main_stream.o
	$(CXX) $(CXXFLAGS) -I../../include main_stream.o syscfg_pc.o -o $@

edyn: syscfg_edyn.o main_edyn.o
	$(CXX) $(CXXFLAGS) -I../../include main_edyn.o syscfg_edyn.o -o $@

efix: syscfg_efix.o main_efix.o
	$(CXX) $(CXXFLAGS) -I../../include main_efix.o syscfg_efix.o -o $@

clean:
	rm -f $(CXXOBJS) syscfg_pc.* syscfg_edy.* syscfg_efix.*

.depend: syscfg_pc.cpp syscfg_edyn.cpp syscfg_efix.cpp | $(DEPS)
	cat $(DEPS) > .depend

include .depend
