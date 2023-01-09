#include <stdint.h>
#include <string.h>
#include <assert.h>

#define STACK		0x0100
#define STACK_RESET 	0xFD

/// # Status Register (P) http://wiki.nesdev.com/w/index.php/Status_flags
///
///  7 6 5 4 3 2 1 0
///  N V _ B D I Z C
///  | |   | | | | +--- Carry Flag
///  | |   | | | +----- Zero Flag
///  | |   | | +------- Interrupt Disable
///  | |   | +--------- Decimal Mode (not used on NES)
///  | |   +----------- Break Command
///  | +--------------- Overflow Flag
///  +----------------- Negative Flag
///
typedef enum __attribute__((__packed__)) {
	CARRY 			= 1 << 0,
	ZERO			= 1 << 1,
        INTERRUPT_DISABLE	= 1 << 2,
        DECIMAL_MODE		= 1 << 3,
        BREAK			= 1 << 4,
        BREAK2			= 1 << 5,
        OVERFLOW		= 1 << 6,
        NEGATIV			= 1 << 7,
} CPUFLAGS;

typedef struct {
	uint8_t register_a;
	uint8_t register_x;
	uint8_t register_y;
	uint8_t status;
	uint16_t program_counter;
	uint8_t stack_pointer;
	uint8_t memory[0xFFFF];
} CPU;

typedef enum {
	Immediate,
	ZeroPage,
	ZeroPage_X,
	ZeroPage_Y,
	Absolute,
	Absolute_X,
	Absolute_Y,
	Indirect_X,
	Indirect_Y,
	NoneAddressing,
} AddressingMode;

typedef struct {
	const uint8_t code;
	const char mnemonic[4];
	const uint8_t len;
	const uint8_t cycles;
	const AddressingMode mode;
} OPCODE;

void createCPU(CPU *cpu);
void destroyCPU(CPU *cpu);

uint8_t mem_read(CPU *cpu, uint16_t add);
uint16_t mem_read_u16(CPU *cpu, uint16_t add);
void mem_write(CPU *cpu, uint16_t add, uint8_t data);
void mem_write_u16(CPU *cpu, uint16_t add, uint16_t data);

uint8_t stack_pop(CPU *cpu);
uint16_t stack_pop_u16(CPU *cpu);
void stack_push(CPU *cpu, uint8_t data);
void stack_push_u16(CPU *cpu, uint16_t data);

uint16_t get_operand_address(CPU *cpu, AddressingMode mode);

void load_and_run(CPU *cpu, uint8_t *program, size_t len);
void load(CPU *cpu, uint8_t *program, size_t len);
void reset(CPU *cpu);
void run(CPU *cpu);

void update_zero_and_negative_flag(CPU *cpu, uint8_t res);

/* Arithmetic */
void add_to_register_a(CPU *cpu, uint8_t data);
void adc(CPU *cpu, AddressingMode mode);
void sbc(CPU *cpu, AddressingMode mode);
void and(CPU *cpu, AddressingMode mode);
void eor(CPU *cpu, AddressingMode mode);
void ora(CPU *cpu, AddressingMode mode);

/* Stores, Loads */
void lda(CPU *cpu, AddressingMode mode);
void ldx(CPU *cpu, AddressingMode mode);
void ldy(CPU *cpu, AddressingMode mode);
void sta(CPU *cpu, AddressingMode mode);
void stx(CPU *cpu, AddressingMode mode);
void sty(CPU *cpu, AddressingMode mode);

/* Stack */
void pha(CPU *cpu);
void pla(CPU *cpu);
void php(CPU *cpu);
void plp(CPU *cpu);

/* Flags clear */
void cld(CPU *cpu);
void cli(CPU *cpu);
void clv(CPU *cpu);
void clc(CPU *cpu);
void sec(CPU *cpu);
void sei(CPU *cpu);
void sed(CPU *cpu);

void tax(CPU *cpu);
void tay(CPU *cpu);
void tsx(CPU *cpu);
void txa(CPU *cpu);
void txs(CPU *cpu);
void tya(CPU *cpu);

/* Branching */
void branch(CPU *cpu, uint8_t cond);
void bit(CPU *cpu, AddressingMode mode);
void jmp_absolute(CPU *cpu);
void jmp_indirect(CPU *cpu);
void jsr(CPU *cpu);
void rts(CPU *cpu);
void rti(CPU *cpu);
void bne(CPU *cpu);
void bvs(CPU *cpu);
void bvc(CPU *cpu);
void bmi(CPU *cpu);
void beq(CPU *cpu);
void bcs(CPU *cpu);
void bcc(CPU *cpu);
void bpl(CPU *cpu);

/* Shifts */
void asl_accumulator(CPU *cpu);
void asl(CPU *cpu, AddressingMode mode);
void lsr_accumulator(CPU *cpu);
void lsr(CPU *cpu, AddressingMode mode);
void rol_accumulator(CPU *cpu);
void rol(CPU *cpu, AddressingMode mode);
void ror_accumulator(CPU *cpu);
void ror(CPU *cpu, AddressingMode mode);

uint8_t inc(CPU *cpu, AddressingMode mode);
void inx(CPU *cpu);
void iny(CPU *cpu);
uint8_t dec(CPU *cpu, AddressingMode mode);
void dex(CPU *cpu);
void dey(CPU *cpu);

void compare(CPU *cpu, AddressingMode mode, uint8_t par);
void cmp(CPU *cpu, AddressingMode mode);
void cpx(CPU *cpu, AddressingMode mode);
void cpy(CPU *cpu, AddressingMode mode);

/* Lookup Tables */
static const OPCODE opcode_lookup_table[256] = {
	{ 0x00, "BRK", 1, 7, NoneAddressing },
	{ 0x01, "ORA", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x05, "ORA", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0x08, "PHP", 1, 3, NoneAddressing },
	{ 0x09, "ORA", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x0D, "ORA", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0x10, "BPL", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing },
	{ 0x11, "ORA", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x15, "ORA", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0x18, "CLC", 1, 2, NoneAddressing },
	{ 0x19, "ORA", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x1D, "ORA", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0x20, "JSR", 3, 6, NoneAddressing },
	{ 0x21, "AND", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0x24, "BIT", 2, 3, ZeroPage },
	{ 0x25, "AND", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0x28, "PLP", 1, 4, NoneAddressing },
	{ 0x29, "AND", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0x2C, "BIT", 3, 4, Absolute },
	{ 0x2D, "AND", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0x30, "BMI", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing },
	{ 0x31, "AND", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x35, "AND", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0x38, "SEC", 1, 2, NoneAddressing },
	{ 0x39, "AND", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x3D, "AND", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0x40, "RTI", 1, 6, NoneAddressing },
	{ 0x41, "EOR", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x45, "EOR", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0x48, "PHA", 1, 3, NoneAddressing },
	{ 0x49, "EOR", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0x4C, "JMP", 3, 3, NoneAddressing }, //AddressingMode that acts as Immediate
	{ 0x4D, "EOR", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0x50, "BVC", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing },
	{ 0x51, "EOR", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x55, "EOR", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0x58, "CLI", 1, 2, NoneAddressing },
	{ 0x59, "EOR", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x5D, "EOR", 3, 4 /* +1 if page crossed */, Absolute_X},
	{ 0 },
	{ 0 },
	{ 0x60, "RTS", 1, 6, NoneAddressing },
	{ 0x61, "ADC", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x65, "ADC", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0x68, "PLA", 1, 4, NoneAddressing },
	{ 0x69, "ADC", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0x6C, "JMP", 3, 5, NoneAddressing }, //AddressingMode:Indirect with 6502 bug
	{ 0x6D, "ADC", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0x70, "BVS", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing },
	{ 0x71, "ADC", 2, 5, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x75, "ADC", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0x78, "SEI", 1, 2, NoneAddressing },
	{ 0x79, "ADC", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x7D, "ADC", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x81, "STA", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0x84, "STY", 2, 3, ZeroPage },
	{ 0x85, "STA", 2, 3, ZeroPage },
	{ 0x86, "STX", 2, 3, ZeroPage },
	{ 0 },
	{ 0x88, "DEY", 1, 2, NoneAddressing },
	{ 0 },
	{ 0x8A, "TXA", 1, 2, NoneAddressing },
	{ 0 },
	{ 0x8C, "STY", 3, 4, Absolute },
	{ 0x8D, "STA", 3, 4, Absolute },
	{ 0x8E, "STX", 3, 4, Absolute },
	{ 0 },
	{ 0x90, "BCC", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing },
	{ 0x91, "STA", 2, 6, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0x94, "STY", 2, 4, ZeroPage_X },
	{ 0x95, "STA", 2, 4, ZeroPage_X },
	{ 0x96, "STX", 2, 4, ZeroPage_Y },
	{ 0 },
	{ 0x98, "TYA", 1, 2, NoneAddressing },
	{ 0x99, "STA", 3, 5, Absolute_Y },
	{ 0x9A, "TXS", 1, 2, NoneAddressing },
	{ 0 },
	{ 0 },
	{ 0x9D, "STA", 3, 5, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0xA0, "LDY", 2, 2, Immediate },
	{ 0xA1, "LDA", 2, 6, Indirect_X},
	{ 0xA2, "LDX", 2, 2, Immediate },
	{ 0 },
	{ 0xA4, "LDY", 2, 3, ZeroPage },
	{ 0xA5, "LDA", 2, 3, ZeroPage },
	{ 0xA6, "LDX", 2, 3, ZeroPage },
	{ 0 },
	{ 0xA8, "TAY", 1, 2, NoneAddressing },
	{ 0xA9, "LDA", 2, 2, Immediate },
	{ 0xAA, "TAX", 1, 2, NoneAddressing },
	{ 0 },
	{ 0xAC, "LDY", 3, 4, Absolute },
	{ 0xAD, "LDA", 3, 4, Absolute },
	{ 0xAE, "LDX", 3, 4, Absolute },
	{ 0 },
	{ 0xB0, "BCS", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing },
	{ 0xB1, "LDA", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0xB4, "LDY", 2, 4, ZeroPage_X },
	{ 0xB5, "LDA", 2, 4, ZeroPage_X },
	{ 0xB6, "LDX", 2, 4, ZeroPage_Y },
	{ 0 },
	{ 0xB8, "CLV", 1, 2, NoneAddressing },
	{ 0xB9, "LDA", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0xBA, "TSX", 1, 2, NoneAddressing },
	{ 0 },
	{ 0xBC, "LDY", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0xBD, "LDA", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0xBE, "LDX", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0xC0, "CPX", 2, 2, Immediate },
	{ 0xC1, "CMP", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0xC4, "CPX", 2, 3, ZeroPage },
	{ 0xC5, "CMP", 2, 3, ZeroPage },
	{ 0xC6, "DEC", 2, 5, ZeroPage },
	{ 0 },
	{ 0xC8, "INY", 1, 2, NoneAddressing },
	{ 0xC9, "CMP", 2, 2, Immediate },
	{ 0xCA, "DEX", 1, 2, NoneAddressing },
	{ 0 },
	{ 0xCC, "CPX", 3, 4, Absolute },
	{ 0xCD, "CMP", 3, 4, Absolute },
	{ 0xCE, "DEC", 3, 6, Absolute },
	{ 0 },
	{ 0xD0, "BNE", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing},
	{ 0xD1, "CMP", 2, 5 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xD5, "CMP", 2, 4, ZeroPage_X },
	{ 0xD6, "DEC", 2, 6, ZeroPage_X },
	{ 0 },
	{ 0xD8, "CLD", 1, 2, NoneAddressing },
	{ 0xD9, "CMP", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xDD, "CMP", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0xDE, "DEC", 3, 7, Absolute_X },
	{ 0 },
	{ 0xE0, "CPY", 2, 2, Immediate },
	{ 0xE1, "SBC", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0xE4, "CPY", 2, 3, ZeroPage },
	{ 0xE5, "SBC", 2, 3, ZeroPage },
	{ 0xE6, "INC", 2, 5, ZeroPage },
	{ 0 },
	{ 0xE8, "INX", 1, 2, NoneAddressing },
	{ 0xE9, "SBC", 2, 2, Immediate },
	{ 0xEA, "NOP", 1, 2, NoneAddressing },
	{ 0 },
	{ 0xEC, "CPY", 3, 4, Absolute },
	{ 0xED, "SBC", 3, 4, Absolute },
	{ 0xEE, "INC", 3, 6, Absolute },
	{ 0 },
	{ 0xF0, "BEQ", 2, 2 /* +1 if branch succeeds +2 if to a new page */, NoneAddressing },
	{ 0xF1, "SBC", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xF5, "SBC", 2, 4, ZeroPage_X },
	{ 0xF6, "INC", 2, 6, ZeroPage_X },
	{ 0 },
	{ 0xF8, "SED", 1, 2, NoneAddressing },
	{ 0xF9, "SBC", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xFD, "SBC", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0xFE, "INC", 3, 7, Absolute_X },
	{ 0 },
};
