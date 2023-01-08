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
	const char mnemonic[3];
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

uint16_t get_operand_address(CPU *cpu, AddressingMode mode);

void load_and_run(CPU *cpu, uint8_t *program, size_t len);
void load(CPU *cpu, uint8_t *program, size_t len);
void reset(CPU *cpu);
void run(CPU *cpu);

void update_zero_and_negative_flag(CPU *cpu, uint8_t res);
void lda(CPU *cpu, AddressingMode mode);
void sta(CPU *cpu, AddressingMode mode);
void tax(CPU *cpu);
void inx(CPU *cpu);

/* Arithmetic */
void add_to_register_a(CPU *cpu, uint8_t data);
void adc(CPU *cpu, AddressingMode mode);
void sbc(CPU *cpu, AddressingMode mode);
void and(CPU *cpu, AddressingMode mode);
void eor(CPU *cpu, AddressingMode mode);
void ora(CPU *cpu, AddressingMode mode);

static const OPCODE opcode_lookup_table[256] = {
	{ 0x00, "BRK", 1, 7, NoneAddressing },
	{ 0x01, "ORA", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x05, "ORA", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x09, "ORA", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x0D, "ORA", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x11, "ORA", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x15, "ORA", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x19, "ORA", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x1D, "ORA", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x21, "AND", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x25, "AND", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x29, "AND", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x2D, "AND", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x31, "AND", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x35, "AND", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x39, "AND", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x3D, "AND", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x41, "EOR", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x45, "EOR", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x49, "EOR", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x4D, "EOR", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x51, "EOR", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x55, "EOR", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x59, "EOR", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x5D, "EOR", 3, 4 /* +1 if page crossed */, Absolute_X},
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x61, "ADC", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x65, "ADC", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x69, "ADC", 2, 2, Immediate },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x6D, "ADC", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x71, "ADC", 2, 5, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x75, "ADC", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0 },
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
	{ 0 },
	{ 0x85, "STA", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x8D, "STA", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x91, "STA", 2, 6, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x95, "STA", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x99, "STA", 3, 5, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0x9D, "STA", 3, 5, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xA1, "LDA", 2, 6, Indirect_X},
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xA5, "LDA", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xA9, "LDA", 2, 2, Immediate },
	{ 0xAA, "TAX", 1, 2, NoneAddressing },
	{ 0 },
	{ 0 },
	{ 0xAD, "LDA", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xB1, "LDA", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xB5, "LDA", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xB9, "LDA", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xBD, "LDA", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xE1, "SBC", 2, 6, Indirect_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xE5, "SBC", 2, 3, ZeroPage },
	{ 0 },
	{ 0 },
	{ 0xE8, "INX", 1, 2, NoneAddressing },
	{ 0xE9, "SBC", 2, 2, Immediate },
	{ 0xEA, "NOP", 1, 2, NoneAddressing },
	{ 0 },
	{ 0 },
	{ 0xED, "SBC", 3, 4, Absolute },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xF1, "SBC", 2, 5 /* +1 if page crossed */, Indirect_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xF5, "SBC", 2, 4, ZeroPage_X },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xF9, "SBC", 3, 4 /* +1 if page crossed */, Absolute_Y },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0xFD, "SBC", 3, 4 /* +1 if page crossed */, Absolute_X },
	{ 0 },
	{ 0 },
};
