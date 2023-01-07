#include "cpu_6502.h"

void createCPU(CPU *cpu) {
	cpu->register_a = 0;
	cpu->register_x = 0;
	cpu->register_y = 0;
	cpu->status = 0;
	cpu->program_counter = 0;
	memset(cpu->memory, 0, 0xFFFF);
}

void destroyCPU(CPU *cpu) {
	(void) cpu;
	return;
}

uint8_t mem_read(CPU *cpu, uint16_t add) {
	return cpu->memory[add];
}

uint16_t mem_read_u16(CPU *cpu, uint16_t add) {
	uint16_t lo = (uint16_t) mem_read(cpu, add);
	uint16_t hi = (uint16_t) mem_read(cpu, add + 1);
	return (hi << 8) | ((uint16_t) lo);
}

void mem_write(CPU *cpu, uint16_t add, uint8_t data) {
	cpu->memory[add] = data;
}

void mem_write_u16(CPU *cpu, uint16_t add, uint16_t data) {
	uint8_t hi = (uint8_t) (data >> 8);
	uint8_t lo = (uint8_t) (data & 0xFF);
	mem_write(cpu, add, lo);
	mem_write(cpu, add + 1, hi);
}

uint16_t get_operand_address(CPU *cpu, AddressingMode mode) {
	switch (mode) {
		case Immediate:
			return cpu->program_counter;
		break;

		case ZeroPage:
			return (uint16_t) mem_read(cpu, cpu->program_counter);
		break;

		case Absolute:
			return mem_read_u16(cpu, cpu->program_counter);
		break;

		case ZeroPage_X:
			return (uint16_t) ((uint16_t) mem_read(cpu, cpu->program_counter) + (uint16_t) cpu->register_x);
		break;

		case ZeroPage_Y:
			return (uint16_t) ((uint16_t) mem_read(cpu, cpu->program_counter) + (uint16_t) cpu->register_y);
		break;

		case Absolute_X:
			return (uint16_t) (mem_read_u16(cpu, cpu->program_counter) + (uint16_t) cpu->register_x);
		break;

		case Absolute_Y:
			return (uint16_t) (mem_read_u16(cpu, cpu->program_counter) + (uint16_t) cpu->register_y);
		break;

		case Indirect_X:
			{
				uint8_t ptr = mem_read(cpu, cpu->program_counter) + cpu->register_x;
				uint16_t lo = mem_read(cpu, (uint16_t) ptr);
				uint16_t hi = mem_read(cpu, (uint16_t) (uint8_t) (ptr + 1));
				return ((uint16_t) hi) << 8 | (uint16_t) lo;
			}
		break;

		case Indirect_Y:
			{
				uint8_t base = mem_read(cpu, cpu->program_counter);
				uint16_t lo = mem_read(cpu, (uint16_t) base);
				uint16_t hi = mem_read(cpu, (uint16_t) (uint8_t) (base + 1));
				uint16_t deref_base = ((uint16_t) hi) << 8 | (uint16_t) lo;
				return deref_base + (uint16_t) cpu->register_y;
			}
		break;

		case NoneAddressing:
			assert(0 && "Mode not supported");
		break;
	}

	return 0;
}

void load_and_run(CPU *cpu, uint8_t *program, size_t len) {
	load(cpu, program, len);
	reset(cpu);
	run(cpu);
}

void load(CPU *cpu, uint8_t *program, size_t len) {
	for(uint16_t i = 0; i < len; ++i) {
		cpu->memory[0x8000 + i] = program[i];
	}
	mem_write_u16(cpu, 0xFFFC, 0x8000);
}

void reset(CPU *cpu) {
	cpu->register_a = 0;
	cpu->register_x = 0;
	cpu->register_y = 0;
	cpu->status = 0;

	cpu->program_counter = mem_read_u16(cpu, 0xFFFC);
}

void run(CPU *cpu) {
	while (1) {
		uint8_t code = mem_read(cpu, cpu->program_counter);
		cpu->program_counter += 1;
		uint16_t program_counter_state = cpu->program_counter;

		OPCODE opcode = opcode_lookup_table[code];

		switch (code) {
			case 0xA9:
			case 0xA5:
			case 0xB5:
			case 0xAD:
			case 0xBD:
			case 0xB9:
			case 0xA1:
			case 0xB1:
				lda(cpu, opcode.mode);
			break;

			case 0x85:
			case 0x95:
			case 0x8D:
			case 0x9D:
			case 0x99:
			case 0x81:
			case 0x91:
				sta(cpu, opcode.mode);
			break;

			case 0xAA:
				tax(cpu);
			break;

			case 0xE8:
				inx(cpu);
			break;

			case 0x00:
				return;
			break;

			default:
				assert(0 && "OPcode non supported yet");
		}

		if (program_counter_state == cpu->program_counter) {
			cpu->program_counter += (uint16_t) (opcode.len - 1);
		}
	}
}

void update_zero_and_negative_flag(CPU *cpu, uint8_t res) {
	if (res) {
		cpu->status = cpu->status & (uint8_t) 0xFD; // 0b11111101
	} else {
		cpu->status = cpu->status | (uint8_t) 0x02; // 0b00000010
	}

	if (res & (uint8_t) 0x80) {
		cpu->status = cpu->status | (uint8_t) 0x80; // 0b10000000
	} else {
		cpu->status = cpu->status & (uint8_t) 0x7F; // 0b01111111
	}
}

void lda(CPU *cpu, AddressingMode mode) {
	uint16_t addr = get_operand_address(cpu, mode);
	uint8_t value = mem_read(cpu, addr);

	cpu->register_a = value;
	update_zero_and_negative_flag(cpu, cpu->register_a);
}

void sta(CPU *cpu, AddressingMode mode) {
	uint16_t addr = get_operand_address(cpu, mode);
	mem_write(cpu, addr, cpu->register_a);
}

void tax(CPU *cpu) {
	cpu->register_x = cpu->register_a;
	update_zero_and_negative_flag(cpu, cpu->register_x);
}

void inx(CPU *cpu) {
	cpu->register_x += 1;
	update_zero_and_negative_flag(cpu, cpu->register_x);
}
