#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#define LEN_PIM 0x100000000

int fd;
std::ifstream fm;
std::string line;
uint8_t* pim_mem;
uint8_t* buffer;
uint64_t pim_base;

uint32_t burstSize = 32;

void set_trace_file(char **argv, char option) {
	fm.open("./mem_trace/"+std::string(argv[1])+option+".txt");
}

void set_pim_device() {
	fd = open("/dev/PIM", O_RDWR|O_SYNC);
	return;
	if (fd < 0)
		std::cout << "Open /dev/PIM failed...\n";
	else
		std::cout << "Opened /dev/PIM !\n";

	pim_mem = (uint8_t*)mmap(NULL, LEN_PIM, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	pim_base = (uint64_t)pim_mem;
}

void trace_and_send() {
	while(std::getline(fm, line)) {
		std::stringstream linestream(line);
		int is_write;
		uint64_t hex_addr;

		linestream >> is_write >> hex_addr;

		if (is_write == 0) {  // read
			std::memcpy(buffer, pim_mem + hex_addr, burstSize);
		} else if (is_write == 1) {  // write
			std::memcpy(pim_mem + hex_addr, buffer, burstSize);
		} else if (is_write == 2) {  // preprocess end
			system("sudo m5 dumpstats");
		} else {
			std::cout << "This should not be done... Somethings wrong\n";
		}
	}
}

int main(int argc, char **argv) {
	char option;
	std::cout << "option : 1 / 2 / 3\nenter option :";
	std::cin >> option;
	
	if (argc <= 1) {
		std::cout << "add, mul, mac, bn, gemv, lstm\n";
		return -1;
	}

	set_trace_file(argv, option);

	system("sudo m5 checkpoint");
	system("echo CPU Switched!");

	set_pim_device();

	trace_and_send();

	system("sudo m5 dumpstats");

	return 0;
}
