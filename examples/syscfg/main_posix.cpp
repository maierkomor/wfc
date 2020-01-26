// example using POSIX API

//#include "syscfg.h" -- included with apropriate name via make supplied command-line

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>	// for JSON output


static char CfgName[] = "syscfg.bin";


void initDefaults(Config &cfg)
{
	printf("initializing defaults\n");
	cfg.set_hostname("host001");
	cfg.set_baudrate(115200);
	cfg.set_wifi_ssid("mywifi");
	cfg.set_wifi_pass("1234567890");
}


void saveConfig(const Config &cfg)
{
	// prepare file for writing
	int fd = open(CfgName,O_CREAT|O_TRUNC|O_RDWR,0666);
	if (-1 == fd) {
		fprintf(stderr,"failed to create syscfg.bin\n");
		return;
	}

	// here we prepare the serialized in memory presentation
	size_t s = cfg.calcSize();		// - how much mem do we need?
	uint8_t *buf = (uint8_t *) malloc(s);	// - allocate it
	cfg.toMemory(buf,s);			// - serialize the data

	// now we write it to the file
	int n = write(fd,buf,s);
	if (-1 == n)
		fprintf(stderr,"error writing to %s: %s\n",CfgName,strerror(errno));

	// cleanup
	close(fd);
	free(buf);

	// check for error
	if (n > 0)
		printf("stored config in %s\n",CfgName);
}


int readConfig(Config &cfg)
{
	// open and read the input file
	int fd = open("syscfg.bin",O_RDONLY);
	if (-1 == fd)
		return 1;
	struct stat st;
	if (-1 == fstat(fd,&st))
		return 1;
	char *buf = (char*) malloc(st.st_size);
	int n = read(fd,buf,st.st_size);
	if (n == -1)
		return 1;
	close(fd);
	
	// here we parse the buffer
	cfg.fromMemory(buf,n);
	// That's it! Ready to go.
	
	printf("conifguration restored from %s\n",CfgName);
	return 0;
}


int main()
{
	Config cfg;
	if (readConfig(cfg)) {
		fprintf(stderr,"error reading %s: %s\n",CfgName,strerror(errno));
		// Config could not be read.
		// So we initialize it with defaults.
		printf("unable to open %s, creating with defaults\n",CfgName);
		initDefaults(cfg);
	}

	// do some work here
	// e.g. output to JSON
	cfg.toJSON(std::cout);
	
	// save the config before terminating
	saveConfig(cfg);
	return 0;
}
