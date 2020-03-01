/*
 *  Copyright (C) 2017-2020, Thomas Maier-Komor
 *
 *  This source file belongs to Wire-Format-Compiler.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Options.h"
#include "CodeGenerator.h"
#include "XmlGenerator.h"
#include "CodeLibrary.h"
#include "ProtoDriver.h"
#include "io.h"
#include "log.h"
#include "template_lib.h"
#include "version.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

using namespace std;

extern void decodeFile(const char *fn);

const char *InstallDir = 0;


void printVersion(ostream &out)
{
	out << "Wire Format Compiler (WFC), Version " VERSION "\nCopyright 2015-2020, Thomas Maier-Komor\n";
}


void printUsage(ostream &out)
{
	out <<	"synopsis: wfc [options] <filename.wfc>\n\n"
		"options:\n"
		"-d <file>   : decode file <file>\n"
		"-f <flag>   : set flag or option <flag> as key=value pair\n"
		"-fwith-<o>  : enable option <o>\n"
		"-fno-<o>    : disable option <o>\n"
		"-f <o=v>    : set option <o> to value <v>\n"
		"-O2         : optimize for speed\n"
		"-Os         : optimize for size\n"
		"-Or         : optimize for review\n"
		"-o <name>   : set output basename to <name>\n"
		"-g          : add comments\n"
		"-l          : generate wfc library only\n"
		"-m <msg>    : generate message <msg>\n"
		"-s          : generate subclasses\n"
		"-t <target> : enable options <target> of .wfc file\n"
		"-p <f>.cct  : postprocess .cct file to .cc for testing\n"
		"-v          : increase verbosity\n"
		"-h          : print help message\n"
		"-V          : print version\n"
		"-I<lib>     : include .cct library or directory\n"
		"\n"
		"flags:\n";
	Options::printHelp(out);
}


#if defined __MINGW32__ || defined __MINGW64__
#define resolve_symlink(b)
#else
void resolve_symlink(char *buf)
{
	struct stat st;
	if (-1 == stat(buf,&st))
		return;
	while ((st.st_mode & S_IFMT) == S_IFLNK) {
		char tmp[PATH_MAX];
		if (-1 == readlink(buf,tmp,sizeof(tmp))) {
			error("unable to resolve link %s: %s",buf,strerror(errno));
			return;
		}
		strcpy(buf,tmp);
		if (-1 == stat(buf,&st))
			return;
	}
}
#endif


static void resolveFromPath(char *fn, const char *arg)
// insecure function!!
#if defined __MINGW32__ || defined __MINGW64__
#define ENVSEP ';'
#else
#define ENVSEP ':'
#endif
{
	struct stat st;
	const char *path = getenv("PATH");
	if (path == 0) {
		strcpy(fn,arg);
		return;
	}
	const char *c = strchr(path,ENVSEP);
	while (c) {
		size_t l = c-path;
		memcpy(fn,path,l);
		if (fn[l-1] != FILESEP) {
			fn[l] = FILESEP;
			++l;
		}
		strcpy(fn+l,arg);
		//diag("looking for %s in %s",arg,fn);
		if (0 == stat(fn,&st)) {
			diag("found %s in %s",arg,fn);
			return;
		}
		path = c+1;
		c = strchr(path,ENVSEP);
	}
	size_t pl = strlen(path);
	memcpy(fn,path,pl);
	if (fn[pl-1] != FILESEP)
		fn[pl++] = FILESEP;
	strcpy(fn+pl,arg);
	//diag("looking for %s in %s",arg,fn);
	if (0 == stat(fn,&st)) {
		diag("found %s in %s",arg,fn);
		return;
	}
#if defined __MINGW32__ || defined __MINGW64__
	//diag("looking for %s in %s",arg,fn);
	if (0 == stat(arg,&st)) {
		diag("found %s in .",arg);
		fn[0] = '.';
		fn[1] = 0;
		return;
	}
#endif
	fn[0] = 0;
}


static void setInstallDir(const char *arg0)
{
	char buf[PATH_MAX];
	size_t n = 0;
#if defined __MINGW32__ || defined __MINGW64__
	if (arg0[1] == ':') {	// we expect an absolute path
#else
	if (arg0[0] == FILESEP) {
#endif
		// absolute path - do nothing
	} else if ((arg0[0] == '.') || strchr(arg0,FILESEP)) {
		// relative path: prefill with cwd
		if (0 == getcwd(buf,sizeof(buf))) {
			buf[0] = '.';
			buf[1] = '/';
			buf[2] = 0;
		} else {
			n = strlen(buf);
			if (buf[n-1] != FILESEP)
				buf[n++] = FILESEP;
		}
	} else {
		// resolve location via path
#if defined __MINGW32__ || defined __MINGW64__
		size_t l = strlen(arg0);
		if (memcmp(arg0+l-4,".exe",4)) {
			char tmp[l+5];
			memcpy(tmp,arg0,l);
			strcpy(tmp+l,".exe");
			resolveFromPath(buf,tmp);
		} else {
			resolveFromPath(buf,arg0);
		}
#else
		resolveFromPath(buf,arg0);
#endif
		n = strlen(buf);
		if (n == 0)
			fatal("unable to resolve %s from PATH",arg0);
	}
	strncpy(buf+n,arg0,sizeof(buf)-n);
	if (char *fs = strrchr(buf,FILESEP)) {
		*fs = 0;
	} else {
		char *wd = getcwd(buf,sizeof(buf));
		assert(wd);
		size_t wl = strlen(wd);
		if (wd[wl-1] != FILESEP) {
			wd[wl] = FILESEP;
			wd[wl+1] = 0;
		}
		InstallDir = strdup(wd);
		diag("InstalDir: %s",InstallDir);
		return;
	}
	char *lfs = strrchr(buf,FILESEP);
	assert(lfs);
	lfs[1] = 0;
	resolve_symlink(buf);
	foldPath(buf);
	InstallDir = strdup(buf);
	diag("installdir: %s",InstallDir);
}


int main(int argc, char *argv[])
{
	vector<string> genMsgs;
	for (int i = 1; i < argc; ++i) {
		if (0 == strcmp("-v",argv[i])) {
#ifdef DEBUG
			if (Diags && !Debug) {
				Debug = true;
				diag("debug messages enabled");
			}
#endif
			if (!Diags) {
				Diags = true;
				diag("diag messages enabled");
			}
		} else if (0 == strcmp("-V",argv[i])) {
			printVersion(cout);
		}
	}
	setInstallDir(argv[0]);
	Options options("commandline",Options::getDefaults());
	bool yydebug = false, hadPostProcess = false, genLib = false;
	const char *target = 0;
	const char *outname = 0;	// output basename to use instead of input file name
	char *fn = 0;
#if defined __MINGW32__ || defined __MINGW64__
#define PCORRECT "+"
#else
#define PCORRECT
	setenv("POSIXLY_CORRECT","1",1);
#endif
	while (optind < argc) {
		int opt = getopt(argc,argv,PCORRECT "d:f:ghI:lm:O:o:p:st:Vvx:y");
		if (opt == -1) {
			if (optind == argc) 
				break;
			if (fn)
				fatal("multiple input files");
			fn = strdup(argv[optind]);
			++optind;
			continue;
		}
		switch (opt) {
		case 'd':
			decodeFile(optarg);
			break;
		case 'f':
			options.addOption(optarg,true);
			break;
		case 'g':
			options.addOption("comments",true);
			break;
		case 'h':
			printUsage(cout);
			exit(EXIT_SUCCESS);
			break;
		case 'I':
			Lib.add(optarg);
			break;
		case 'l':
			genLib = true;
			options.addOption("wfclib","extern",true);
			break;
		case 'm':
			genMsgs.push_back(optarg);
			break;
		case 'O':
			if (0 == strcmp(optarg,"r"))
				options.addOption("Optimize","review",true);
			else if (0 == strcmp(optarg,"2"))
				options.addOption("Optimize","speed",true);
			else if (0 == strcmp(optarg,"s"))
				options.addOption("Optimize","size",true);
			else
				error("invalid argument %s for option -O",optarg);
			break;
		case 'o':
			outname = optarg;
			break;
		case 'p':
			PostProcess(optarg,&options);
			hadPostProcess = true;
			break;
		case 's':
			options.addOption("subclasses",true);
			break;
		case 't':
			target = optarg;
			break;
		case 'V':
			exit(EXIT_SUCCESS);
			break;
		case 'v':
			/*
			 * done at startup
			 * just to prevent "unknown option" messages
			 */
			break;
		case 'x':
			options.addOption(optarg,false);
			break;
		case 'y':
			yydebug = true;
			break;
		default:
			fatal("unknown option -%c",opt);
		}
	}
	string sharedir0 = InstallDir;
	sharedir0 += "share";
	string sharedir1 = sharedir0;
	sharedir1 += FILESEP;
	sharedir1 += "wfc";
	if (0 == access(sharedir1.c_str(),X_OK|R_OK))
		Lib.add(sharedir1.c_str());
	else if (0 == access(sharedir0.c_str(),X_OK|R_OK))
		Lib.add(sharedir0.c_str());
	else
		Lib.addBuf(TemplateLib,"<internal>");
	ProtoDriver drv(yydebug);
	if (fn == 0) {
		if (genLib) {
			CodeGenerator c(0,&options);
			c.writeLib(outname);
			exit(hadError() ? EXIT_FAILURE : EXIT_SUCCESS);
		}
		if (hadPostProcess)
			exit(EXIT_SUCCESS);
		fatal("missing input file");
	}
	msg("parsing %s...",fn);
	drv.parse(fn);
	if (drv.hadError() || hadError())
		exit(EXIT_FAILURE);
	PBFile *f = drv.getFile();
	char *dot = strrchr(fn,'.');
	assert(dot);
	*dot = 0;

	CodeGenerator cg(f,&options);
	if (genLib) {
		cg.writeLib();
		exit(hadError() ? EXIT_FAILURE : EXIT_SUCCESS);
	}
	if (target) {
		cg.setTarget(target);
		if (hadError())
			exit(EXIT_FAILURE);
	}
	cg.init(genMsgs);
	if (hadError())
		exit(EXIT_FAILURE);

	cg.writeFiles(outname ? outname : fn);
	if (NumWarnings)
		msg("Finished with %u warnings.",NumWarnings);
	return 0;
}
