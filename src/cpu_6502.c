#include "cpu_6502.h"

void createCPU(CPU *cpu) {
	cpu->register_a = 0;
	cpu->register_x = 0;
	cpu->register_y = 0;
	cpu->status = NEGATIV | INTERRUPT_DISABLE;
	cpu->program_counter = 0;
	cpu->stack_pointer = STACK_RESET;
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

uint8_t stack_pop(CPU *cpu) {
	cpu->stack_pointer += 1;
	return mem_read(cpu, (uint16_t) STACK + (uint16_t) cpu->stack_pointer);
}

uint16_t stack_pop_u16(CPU *cpu) {
	uint16_t lo = (uint16_t) stack_pop(cpu);
	uint16_t hi = (uint16_t) stack_pop(cpu);
	return (hi << 8) | ((uint16_t) lo);
}

void stack_push(CPU *cpu, uint8_t data) {
	mem_write(cpu, (uint16_t) STACK + (uint16_t) cpu->stack_pointer, data);
	cpu->stack_pointer -= 1;
}

void stack_push_u16(CPU *cpu, uint16_t data) {
	uint8_t hi = (uint8_t) (data >> 8);
	uint8_t lo = (uint8_t) (data & 0xFF);
	stack_push(cpu, lo);
	stack_push(cpu, hi);
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
	cpu->status = NEGATIV | INTERRUPT_DISABLE;
	cpu->stack_pointer = STACK_RESET;
	cpu->program_counter = mem_read_u16(cpu, 0xFFFC);
}

void run(CPU *cpu) {
	while (1) {
		uint8_t code = mem_read(cpu, cpu->program_counter);
		cpu->program_counter += 1;
		uint16_t program_counter_state = cpu->program_counter;

		OPCODE opcode = opcode_lookup_table[code];

		switch (code) {
			/* ADC */
			case 0x69:
			case 0x65:
			case 0x75:
			case 0x6D:
			case 0x7D:
			case 0x79:
			case 0x61:
			case 0x71:
                		adc(cpu, opcode.mode);
			break;

                	/* SBC */
			case 0xE9:
			case 0xE5:
			case 0xF5:
			case 0xED:
			case 0xFD:
			case 0xF9:
			case 0xE1:
			case 0xF1:
                		sbc(cpu, opcode.mode);
			break;

                	/* AND */
			case 0x29:
			case 0x25:
			case 0x35:
			case 0x2D:
			case 0x3D:
			case 0x39:
			case 0x21:
			case 0x31:
                		and(cpu, opcode.mode);
			break;

                	/* EOR */
			case 0x49:
			case 0x45:
			case 0x55:
			case 0x4D:
			case 0x5D:
			case 0x59:
			case 0x41:
			case 0x51:
                		eor(cpu, opcode.mode);
			break;

	                /* ORA */
			case 0x09:
			case 0x05:
			case 0x15:
			case 0x0D:
			case 0x1D:
			case 0x19:
			case 0x01:
			case 0x11:
                    		ora(cpu, opcode.mode);
                	break;

			/* LDA */
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

			/* LDX */
			case 0xa2:
			case 0xa6:
			case 0xb6:
			case 0xae:
			case 0xbe:
				ldx(cpu, opcode.mode);
			break;

                	/* LDY */
			case 0xa0:
			case 0xa4:
			case 0xb4:
			case 0xac:
			case 0xbc:
				ldy(cpu, opcode.mode);
			break;

			/* STA */
			case 0x85:
			case 0x95:
			case 0x8D:
			case 0x9D:
			case 0x99:
			case 0x81:
			case 0x91:
				sta(cpu, opcode.mode);
			break;

			/* STX */
			case 0x86:
			case 0x96:
			case 0x8e:
				stx(cpu, opcode.mode);
			break;

			/* STY */
			case 0x84:
			case 0x94:
			case 0x8c:
				sty(cpu, opcode.mode);
			break;

			/* PHA */
			case 0x48:
				pha(cpu);
			break;

                	/* PLA */
			case 0x68:
                    		pla(cpu);
			break;

			/* PHP */
			case 0x08:
				php(cpu);
			break;

			/* PLP */
			case 0x28:
				plp(cpu);
			break;

                	/* CLD */
			case 0xd8:
				cld(cpu);
			break;

                	/* CLI */
			case 0x58:
				cli(cpu);
			break;

                	/* CLV */
			case 0xb8:
				clv(cpu);
			break;

                	/* CLC */
			case 0x18:
				clc(cpu);
			break;

                	/* SEC */
			case 0x38:
				sec(cpu);
			break;

                	/* SEI */
			case 0x78:
				sei(cpu);
			break;

                	/* SED */
			case 0xf8:
				sed(cpu);
			break;

			/* TAX */
			case 0xAA:
				tax(cpu);
			break;

			/* TAY */
			case 0xA8:
				tay(cpu);
			break;

			/* TSX */
			case 0xBA:
				tsx(cpu);
			break;

			/* TXA */
			case 0x8A:
				txa(cpu);
			break;

			/* TXS */
			case 0x9A:
				txs(cpu);
			break;

			/* TYA */
			case 0x98:
				tya(cpu);
			break;

			/* JMP Absolute */
			case 0x4c:
				jmp_absolute(cpu);
			break;

			/* JMP Indirect */
			case 0x6c:
				jmp_indirect(cpu);
			break;

			/* JSR */
			case 0x20:
				jsr(cpu);
			break;

			/* RTS */
			case 0x60:
				rts(cpu);
			break;

			/* RTI */
			case 0x40:
				rti(cpu);
			break;

			/* BNE */
			case 0xd0:
				bne(cpu);
			break;

			/* BVS */
			case 0x70:
				bvs(cpu);
			break;

			/* BVC */
			case 0x50:
				bvc(cpu);
			break;

			/* BPL */
			case 0x10:
				bpl(cpu);
			break;

			/* BMI */
			case 0x30:
				bmi(cpu);
			break;

			/* BEQ */
			case 0xf0:
				beq(cpu);
			break;

			/* BCS */
			case 0xb0:
				bcs(cpu);
			break;

			/* BCC */
			case 0x90:
				bcc(cpu);
			break;

			/* BIT */
			case 0x24:
			case 0x2c:
				bit(cpu, opcode.mode);
			break;

			/* INX */
			case 0xE8:
				inx(cpu);
			break;

			/* NOP */
			case 0xEA:
			break;

			/* BRK */
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
		cpu->status &= ~ZERO;
	} else {
		cpu->status |= ZERO;
	}

	if (res & NEGATIV) {
		cpu->status |= NEGATIV;
	} else {
		cpu->status &= ~NEGATIV;
	}
}

void inx(CPU *cpu) {
	cpu->register_x += 1;
	update_zero_and_negative_flag(cpu, cpu->register_x);
}

/* Arithmetic */
/// note: ignoring decimal mode
/// http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
void add_to_register_a(CPU *cpu, uint8_t data) {
	uint16_t sum = (uint16_t) cpu->register_a + (uint16_t) data + (uint16_t) (cpu->status & CARRY) ? 1 : 0;

	if (sum > 0xFF) {
		cpu->status |= CARRY;
	} else {
		cpu->status &= ~CARRY;
	}

	uint8_t res = (uint8_t) sum;
	if ((data ^ res) & (res ^ cpu->register_a) & 0x80) {
		cpu->status &= ~OVERFLOW;
	} else {
		cpu->status |= OVERFLOW;
	}

	cpu->register_a = res;
	update_zero_and_negative_flag(cpu, cpu->register_a);
}

void adc(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	add_to_register_a(cpu, value);
}

void sbc(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	add_to_register_a(cpu, (uint8_t) (int8_t) (value - 1));
}

void and(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	add_to_register_a(cpu, value & cpu->register_a);
}

void eor(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	add_to_register_a(cpu, value ^ cpu->register_a);
}

void ora(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	add_to_register_a(cpu, value | cpu->register_a);
}

/* Stores, Loads */
void lda(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	cpu->register_a = value;
	update_zero_and_negative_flag(cpu, cpu->register_a);
}

void ldx(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	cpu->register_x = value;
	update_zero_and_negative_flag(cpu, cpu->register_x);
}

void ldy(CPU *cpu, AddressingMode mode) {
	uint8_t value = mem_read(cpu, get_operand_address(cpu, mode));
	cpu->register_y = value;
	update_zero_and_negative_flag(cpu, cpu->register_y);
}

void sta(CPU *cpu, AddressingMode mode) {
	uint16_t addr = get_operand_address(cpu, mode);
	mem_write(cpu, addr, cpu->register_a);
}

void stx(CPU *cpu, AddressingMode mode) {
	uint16_t addr = get_operand_address(cpu, mode);
	mem_write(cpu, addr, cpu->register_x);
}

void sty(CPU *cpu, AddressingMode mode) {
	uint16_t addr = get_operand_address(cpu, mode);
	mem_write(cpu, addr, cpu->register_y);
}

/* Stack */
void pha(CPU *cpu) {
	stack_push(cpu, cpu->register_a);
}

void pla(CPU *cpu) {
	cpu->register_a = stack_pop(cpu);
	update_zero_and_negative_flag(cpu, cpu->register_a);
}

void php(CPU *cpu) {
	//http://wiki.nesdev.com/w/index.php/CPU_status_flag_behavior
	uint8_t status = cpu->status;
	status |= BREAK;
	status |= BREAK2;
	stack_push(cpu, status);
}

void plp(CPU *cpu) {
	cpu->status = stack_pop(cpu);
	cpu->status &= ~BREAK;
	cpu->status |= BREAK2;
}

/* Flags clear */
void cld(CPU *cpu) {
	cpu->status &= ~DECIMAL_MODE;
}

void cli(CPU *cpu) {
	cpu->status &= ~INTERRUPT_DISABLE;
}

void clv(CPU *cpu) {
	cpu->status &= ~OVERFLOW;
}

void clc(CPU *cpu) {
	cpu->status &= ~CARRY;
}

void sec(CPU *cpu) {
	cpu->status |= CARRY;
}

void sei(CPU *cpu) {
	cpu->status |= INTERRUPT_DISABLE;
}

void sed(CPU *cpu) {
	cpu->status |= DECIMAL_MODE;
}


void tax(CPU *cpu) {
	cpu->register_x = cpu->register_a;
	update_zero_and_negative_flag(cpu, cpu->register_x);
}

void tay(CPU *cpu) {
	cpu->register_y = cpu->register_a;
	update_zero_and_negative_flag(cpu, cpu->register_y);
}

void tsx(CPU *cpu) {
	cpu->register_x = cpu->stack_pointer;
	update_zero_and_negative_flag(cpu, cpu->register_x);
}

void txa(CPU *cpu) {
	cpu->register_a = cpu->register_x;
	update_zero_and_negative_flag(cpu, cpu->register_a);
}

void txs(CPU *cpu) {
	cpu->stack_pointer = cpu->register_x;
}

void tya(CPU *cpu) {
	cpu->register_a = cpu->register_y;
	update_zero_and_negative_flag(cpu, cpu->register_a);
}

/* Branching */
void jmp_absolute(CPU *cpu) {
	uint16_t addr = mem_read_u16(cpu, cpu->program_counter);
	cpu->program_counter = addr;
}

void jmp_indirect(CPU *cpu) {
	uint16_t addr = mem_read_u16(cpu, cpu->program_counter);
        // 6502 bug mode with with page boundary:
        //  if address $3000 contains $40, $30FF contains $80, and $3100 contains $50,
        //  the result of JMP ($30FF) will be a transfer of control to $4080 rather than $5080 as you intended
        //  i.e. the 6502 took the low byte of the address from $30FF and the high byte from $3000
	uint16_t ind_ref;
	if ((addr & 0x00FF) == 0x00FF) {
		uint8_t lo = mem_read(cpu, addr);
		uint8_t hi = mem_read(cpu, addr & 0xFF00);
		ind_ref = ((uint16_t) hi) << 8 | (uint16_t) lo;
	} else {
		ind_ref = mem_read_u16(cpu, addr);
	}

	cpu->program_counter = ind_ref;
}

void jsr(CPU *cpu) {
	stack_push_u16(cpu, cpu->program_counter + 2 - 1);
	uint16_t target_addr = mem_read_u16(cpu, cpu->program_counter);
	cpu->program_counter = target_addr;
}

void rts(CPU *cpu) {
	cpu->program_counter = stack_pop_u16(cpu) + 1;
}

void rti(CPU *cpu) {
	// plp call
	cpu->status = stack_pop(cpu);
	cpu->status &= ~BREAK;
	cpu->status |= BREAK2;

	cpu->program_counter = stack_pop_u16(cpu);
}

void branch(CPU *cpu, uint8_t cond) {
	if (cond) {
		int8_t jump = (int8_t) mem_read(cpu, cpu->program_counter);
		uint16_t jump_addr = cpu->program_counter + 1 + (uint16_t) jump;

		cpu->program_counter = jump_addr;
	}
}

void bne(CPU *cpu) {
	branch(cpu, !(cpu->status & ZERO));
}

void bvs(CPU *cpu) {
	branch(cpu, (cpu->status & OVERFLOW));
}

void bvc(CPU *cpu) {
	branch(cpu, !(cpu->status & OVERFLOW));
}

void bmi(CPU *cpu) {
	branch(cpu, (cpu->status & NEGATIV));
}

void beq(CPU *cpu) {
	branch(cpu, (cpu->status & ZERO));
}

void bcs(CPU *cpu) {
	branch(cpu, (cpu->status & CARRY));
}

void bcc(CPU *cpu) {
	branch(cpu, !(cpu->status & CARRY));
}

void bpl(CPU *cpu) {
	branch(cpu, !(cpu->status & NEGATIV));
}

void bit(CPU *cpu, AddressingMode mode) {
	uint16_t addr = get_operand_address(cpu, mode);
	uint8_t data = mem_read(cpu, addr);
	if (cpu->register_a & data) {
		cpu->status &= ~ZERO;
	} else {
		cpu->status |= ZERO;
	}

	if (data & NEGATIV) cpu->status |= NEGATIV;
	if (data & OVERFLOW) cpu->status |= OVERFLOW;
}
