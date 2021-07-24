#
#  Copyright (C) 2019-2021, Thomas Maier-Komor
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
#CXXSRCS	+=$(ODIR)/comp_v1.cpp $(ODIR)/comp_v2.cpp
CXXOBJS	= $(CXXSRCS:%.cpp=%.o) $(WFCOBJS)
LIBJSON	= -L/usr/pkg/lib -ljson-c -Wl,-R/usr/pkg/lib


TESTCASES = \
	corruption enumtest empty_test tttest vbittest xvarint \
	cstrtest stringtest recursion json_hs lt1 skiptest reftest \
	vbittest2 tttest fixed_test novi_test pack_test comp_test \
	ref_byname byname_test inv_def_test arraycheck reftestv2

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
	for i in $(TESTCASES); do echo $$i; "$(ODIR)/$$i" || exit; done


$(ODIR)/corruption: $(ODIR)/corruption.o $(ODIR)/hostscope.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/enumtest: $(ODIR)/enumtest.o $(ODIR)/enumtest1.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/empty_test: $(ODIR)/empty_test.o $(ODIR)/empty_elements.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/vbittest: $(ODIR)/vbittest.o $(ODIR)/validbits.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/xvarint: $(ODIR)/xvarint.o $(ODIR)/xvarint_16.o $(ODIR)/xvarint_32.o $(ODIR)/testxvarint.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/cstrtest: $(ODIR)/cstr_test.o $(ODIR)/NodeInfo.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/stringtest: $(ODIR)/stringtest.o $(ODIR)/stringtypes.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/recursion: $(ODIR)/rtest.o $(ODIR)/recursion.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/json_hs: $(ODIR)/json_hs.o $(ODIR)/hostscope.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/lt1: $(ODIR)/libtest1.o $(ODIR)/lt1.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/skiptest: $(ODIR)/skiptest.o $(ODIR)/skip_s.o $(ODIR)/skip_r.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/reftest: $(ODIR)/reftest.o $(ODIR)/reference.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/reftestv2: $(ODIR)/reftestv2.o $(ODIR)/referencev2.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/vbittest2: $(ODIR)/vbittest2.o $(ODIR)/validbits2.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/tttest: $(ODIR)/tttest.o $(ODIR)/tt.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/fixed_test: $(ODIR)/fixed_test.o $(ODIR)/fixed_only.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/novi_test: $(ODIR)/test_novi.o $(ODIR)/novarint.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/pack_test: $(ODIR)/pack_test.o $(ODIR)/packed.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/astr_test: $(ODIR)/astr_test.o $(ODIR)/astr.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/comp_test: $(ODIR)/comp_v1.o $(ODIR)/comp_v2.o $(ODIR)/comp_test.o  $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/ref_byname: $(ODIR)/bynametest.o $(ODIR)/reference.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/byname_test: $(ODIR)/test_byname.o $(ODIR)/byname.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/inv_def_test: $(ODIR)/inv_def_test.o $(ODIR)/inv_def.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)

$(ODIR)/arraycheck: $(ODIR)/checkarray.o $(ODIR)/arraycheck.o $(WFCOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBJSON)
