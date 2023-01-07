#include <stdio.h>

#include "cpu_6502.h"

int main(int argc, char **argv) {

	(void) argc;
	(void) argv;

	uint8_t program[] = {0xa5, 0x10, 0x00};
	size_t len = sizeof(program) / sizeof(program[0]);
	CPU cpu;

	createCPU(&cpu);
	mem_write(&cpu, 0x10, 0x55);
	load_and_run(&cpu, program, len);
	printf("%lu\n%d\n%d\n%x\n", len, cpu.register_a, cpu.register_x, cpu.status);

	return 0;
}
