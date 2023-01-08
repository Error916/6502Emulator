#include <stdio.h>

#include "cpu_6502.h"

void binaryprint(uint8_t n){
	int count = 0;
	while (n) {
    		if (n & 1)
        		printf("1");
    		else
        		printf("0");
		n >>= 1;
		++count;
	}
	count = 8 - count;
	for (;count; --count) printf("0");
	printf("\n");
}

int main(int argc, char **argv) {

	(void) argc;
	(void) argv;

	uint8_t program[] = { 0xa9, 0xc0, 0xaa, 0xe8, 0x00 };
	size_t len = sizeof(program) / sizeof(program[0]);
	CPU cpu;

	createCPU(&cpu);
	load_and_run(&cpu, program, len);
	printf("Length of program: %lu\n", len);
	printf("register_a: %u\n", cpu.register_a);
	printf("register_x: %u\n", cpu.register_x);
	printf("register_y: %u\n", cpu.register_y);
	printf("State Flag: ");
	binaryprint(cpu.status);
	destroyCPU(&cpu);

	return 0;
}
