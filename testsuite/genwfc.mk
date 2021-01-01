#
#  Copyright (C) 2019-2020, Thomas Maier-Komor
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

WFC	= $(WFCROOT)/bin/wfc

WFCSRCS = testcases/empty_elements.wfc testcases/enumtest.wfc testcases/validbits.wfc \
	  testcases/NodeInfo.wfc testcases/stringtypes.wfc testcases/recursion.wfc \
	  testcases/libtest1.wfc testcases/hostscope.wfc testcases/xvarint.wfc \
	  testcases/reference.wfc testcases/validbits2.wfc testcases/tt.wfc \
	  testcases/unused.wfc testcases/fixed_only.wfc testcases/novarint.wfc \
	  testcases/packed.wfc testcases/virtual.wfc testcases/byname.wfc \
	  testcases/inv_def.wfc testcases/arraycheck.wfc


CXXSRCS	= $(WFCSRCS:testcases/%.wfc=$(ODIR)/%.cpp) $(ODIR)/referencev2.cpp

$(ODIR)/%.cpp: testcases/%.wfc
	"$(WFC)" $(WFCFLAGS) $< -o $(@:.cpp=)


all: $(CXXSRCS) $(ODIR)/skip_s.cpp $(ODIR)/comp_v1.cpp $(ODIR)/comp_v2.cpp



$(ODIR)/hostscope.cpp: testcases/hostscope.wfc
	"$(WFC)" $(WFCFLAGS) -fno-asserts $< -o $(@:.cpp=)

$(ODIR)/xvarint.cpp: testcases/xvarint.wfc
	"$(WFC)" $(WFCFLAGS) -tvi16 -fwfclib=static $< -o $(ODIR)/xvarint_16
	"$(WFC)" $(WFCFLAGS) -tvi32 -fwfclib=static $< -o $(ODIR)/xvarint_32
	"$(WFC)" $(WFCFLAGS) -tvi64 -fwfclib=static $< -o $(ODIR)/xvarint

$(ODIR)/skip_s.cpp: testcases/skip.wfc
	"$(WFC)" $(WFCFLAGS) -tsender testcases/skip.wfc -o $(ODIR)/skip_s
	"$(WFC)" $(WFCFLAGS) -treceiver testcases/skip.wfc -o $(ODIR)/skip_r

$(ODIR)/comp_v1.cpp: testcases/compatibility.wfc
	"$(WFC)" $(WFCFLAGS) -tV1 -o $(ODIR)/comp_v1 testcases/compatibility.wfc

$(ODIR)/comp_v2.cpp: testcases/compatibility.wfc
	"$(WFC)" $(WFCFLAGS) -tV2 -o $(ODIR)/comp_v2 testcases/compatibility.wfc

$(ODIR)/referencev2.cpp: testcases/reference.wfc
	"$(WFC)" $(WFCFLAGS) -trev2 -o $(ODIR)/referencev2 $^
