CXXFLAGS=-g
WFC=../../bin/wfc
MV=mv

all: sender receiver

letter_s.cpp: letter.wfc
	$(WFC) -o letter_s $^

letter_r.cpp: letter.wfc
	$(WFC) -treceiver -o letter_r $^

sender: sender.cpp letter_s.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

receiver: receiver.cpp letter_r.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

