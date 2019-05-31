#
#  Copyright (C) 2019, Thomas Maier-Komor
#
#  This file belongs to Wire-Format-Compiler.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

WFC	=../bin/wfc
WFCFLAGS=-O2
CXX	=g++
CXXFLAGS=-g -Os -I$(ODIR) -I../include -I.
CXXOBJS	= $(CXXSRCS:%.cpp=%.o) $(WFCOBJS)


TESTCASES = \
	corruption enumtest empty_test tttest vbittest xvarint \
	cstrtest stringtest recursion json_hs lt1 skiptest reftest \
	vbittest2 tttest fixed_test

COMPILETESTS = \
	unused


$(ODIR)/%.cpp $(ODIR)/%.h: testcases/%.wfc
	$(WFC) $(WFCFLAGS) $< -o $(@:.cpp=)

$(ODIR)/%.o: testcases/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: ../lib/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: $(ODIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(COMPILETESTS:%=$(ODIR)/%.o) $(TESTCASES:%=$(ODIR)/%)
	for i in $(TESTCASES); do $(ODIR)/$$i || exit; done


$(ODIR)/corruption: $(ODIR)/corruption.o $(ODIR)/hostscope.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/enumtest: $(ODIR)/enumtest.o $(ODIR)/enumtest1.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/empty_test: $(ODIR)/empty_test.o $(ODIR)/empty_elements.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/vbittest: $(ODIR)/vbittest.o $(ODIR)/validbits.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/xvarint: $(ODIR)/xvarint.o $(ODIR)/xvarint_16.o $(ODIR)/xvarint_32.o $(ODIR)/testxvarint.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/cstrtest: $(ODIR)/cstr_test.o $(ODIR)/NodeInfo.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/stringtest: $(ODIR)/stringtest.o $(ODIR)/stringtypes.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/recursion: $(ODIR)/rtest.o $(ODIR)/recursion.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/json_hs: $(ODIR)/json_hs.o $(ODIR)/hostscope.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/lt1: $(ODIR)/libtest1.o $(ODIR)/lt1.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/skiptest: $(ODIR)/skiptest.o $(ODIR)/skip_s.o $(ODIR)/skip_r.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/reftest: $(ODIR)/reftest.o $(ODIR)/reference.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/vbittest2: $(ODIR)/vbittest2.o $(ODIR)/validbits2.o $(ODIR)/wfc_support.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/tttest: $(ODIR)/tttest.o $(ODIR)/tt.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ODIR)/fixed_test: $(ODIR)/fixed_test.o $(ODIR)/fixed_only.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

