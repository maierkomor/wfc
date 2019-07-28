#
#  Copyright (C) 2017-2019, Thomas Maier-Komor
#
#  This source file belongs to Wire-Format-Compiler.
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

INCDIRS		= 
LFLAGS		= -F -R --bison-locations
YFLAGS		= -Lc++

YSRC		= xproto.y
LSRC		= xproto.l
CSRC		= 
CXXSRC		= $(YSRC:.y=.cc) $(LSRC:.l=_tok.cc) Message.cc PBFile.cc \
		ProtoDriver.cc log.cc wfc.cc Enum.cc Field.cc Options.cc \
		Generator.cc CodeGenerator.cc CppGenerator.cc \
		Evaluator.cc CodeLibrary.cc io.cc FoldCompounds.cc \
		Indenter.cc XmlGenerator.cc CodeTemplate.cc Decoder.cc \
		template_lib.cc wirefuncs.cc keywords.cc
CCTSRC		=$(shell if [ -d ../.hg ]; then hg st -a -c -m -n ../share; else ls -1 ../share/*.cc*; fi)

BINDIR		= ../bin
OBJDIR		= ../obj-g
COBJ		= $(CSRC:%.c=$(OBJDIR)/%.o)
LIBCOBJ		= $(LIBCSRC:lib/%.c=$(OBJDIR)/%.o)
CXXOBJ		= $(CXXSRC:%.cc=$(OBJDIR)/%.o)
DEPS		= $(CXXSRC:%.cc=$(OBJDIR)/%.d) $(CSRC:%.c=$(OBJDIR)/%.d)

.PRECIOUS:xproto_tok.cc xproto.cc
.PHONY:tests always

default: $(BINDIR)/wfc

template_lib.cc: genlib.sh
	CCTSRC="$(CCTSRC)" bash genlib.sh

xproto.cc: xproto.y tokennames.sed
	$(YACC) $(YFLAGS) --defines=$(<:.y=.h) -o $(<:.y=.cc) $<
	$(SED) -iin -f tokennames.sed $@

%_tok.cc: %.l
	$(LEX) $(LFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.d: %.c
	$(CC) $(CCFLAGS) -MT$(@:%.d=%.o) -MM $< > $@

$(OBJDIR)/%.d: %.cc
	$(CXX) $(CXXFLAGS) -MT$(@:%.d=%.o) -MM $< > $@

$(BINDIR):
	-$(MKDIR) $(BINDIR)

$(BINDIR)/wfc: $(BINDIR) $(OBJDIR) $(CXXOBJ) $(COBJ)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(CXXOBJ) $(COBJ) -o $@ $(LIBS)

$(BINDIR)/wfc.exe: $(BINDIR) $(OBJDIR) $(CXXOBJ) $(COBJ)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(CXXOBJ) $(COBJ) -Xlinker -Map=../wfc.exe.map -o $@ $(LIBS)

%.cc %.h: %.proto
	./wfc $<

always:

version.h: always
	bash mkversion.sh

all: prerequisites tests

tests: wfc
	cd tests; $(MAKE)
	
$(OBJDIR):
	mkdir $@

clean:
	-$(RM) template_lib.cc template_lib.h xproto_tok.cc xproto.cc $(CXXOBJ) $(COBJ) reftest wfc core $(DEPS)

distclean: clean
	$(RM) $(BINDIR)/wfc $(BINDIR)/wfc.exe


.depend: $(OBJDIR) | template_lib.cc version.h $(DEPS)
	cat $(DEPS) > .depend

include .depend
