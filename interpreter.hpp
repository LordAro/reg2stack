#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <functional>
#include <array>
#include <cstdint>
#include <boost/variant.hpp>
#include <vector>

using boost::variant;

enum class op_t : size_t {
	NOP,
	RET,
	END,

	INC,
	DEC,
	AND,
	OR,
	XOR,
	CP,
	SUB,
	ORG,

	LD,
	ADD,
	ADC,
	SBC,
	JP,
	JR,
	CALL,
	NUM_OPS,
};

static const std::array<std::string, (size_t)op_t::NUM_OPS> op_t_str{{
	"NOP",
	"RET",
	"END",

	"INC",
	"DEC",
	"AND",
	"OR",
	"XOR",
	"CP",
	"SUB",
	"ORG",

	"LD",
	"ADD",
	"ADC",
	"SBC",
	"JP",
	"JR",
	"CALL",
}};

class z80regblock {
public:
	uint8_t& operator[](const char r);
	uint16_t& operator[](const char *r);

private:
	std::array<uint8_t, 8> block;
	static const std::string ACTUAL_REGS;
};

class z80machine {
public:
	void nop_func();
	void ret_func();
	void end_func();

	void inc_func(const std::string &x);
	void dec_func(const std::string &x);
	void and_func(const std::string &x);
	void or_func(const std::string &x);
	void xor_func(const std::string &x);
	void cp_func(const std::string &x);
	void sub_func(const std::string &x);
	void org_func(const std::string &x);

	void ld_func(const std::string &x, const std::string &y);
	void add_func(const std::string &x, const std::string &y);
	void adc_func(const std::string &x, const std::string &y);
	void sbc_func(const std::string &x, const std::string &y);
	void jp_func(const std::string &x, const std::string &y);
	void jr_func(const std::string &x, const std::string &y);
	void call_func(const std::string &x, const std::string &y);


	z80regblock main;   // main registers
	z80regblock shadow; // shadow registers

	uint16_t ix;  // index x
	uint16_t iy;  // index y

	uint8_t i;    // interrupt vector
	uint8_t r;    // refresh counter

	uint16_t sp;  // stack pointer
	uint16_t pc;  // program counter

	std::array<uint8_t, 64*1024> mem; // 64KB mem
};

using op0func_t = std::function<void(z80machine*)>;
using op1func_t = std::function<void(z80machine*, const std::string &)>;
using op2func_t = std::function<void(z80machine*, const std::string &, const std::string &)>;
using opfunc_t = variant<op0func_t, op1func_t, op2func_t>;
using operations_map = std::array<opfunc_t, (size_t)op_t::NUM_OPS>;

static const operations_map OPERATIONS {{
	&z80machine::nop_func,
	&z80machine::ret_func,
	&z80machine::end_func,

	&z80machine::inc_func,
	&z80machine::dec_func,
	&z80machine::and_func,
	&z80machine::or_func,
	&z80machine::xor_func,
	&z80machine::cp_func,
	&z80machine::sub_func,
	&z80machine::org_func,

	&z80machine::ld_func,
	&z80machine::add_func,
	&z80machine::adc_func,
	&z80machine::sbc_func,
	&z80machine::jp_func,
	&z80machine::jr_func,
	&z80machine::call_func,
}};

struct z80instruction {
	op_t op;
	std::string operand1;
	std::string operand2;
	std::string label;
};

using z80prog = std::vector<z80instruction>;

z80prog tokeniseSource(const std::string &source);

enum class OPALL : uint8_t {
	NOP = 0x00,    // nop
	LD01,          // ld bc,**
	LDbc_a,        // ld (bc),a
	INCbc,         // inc bc
	INCb,          // inc b
	DECb,          // dec b
	LD03,          // ld b,*
	RLCA,          // rlca
	EX1,           // ex af,af'
	ADDhl_bc,      // add hl,bc
	LDa_bc,        // ld a,(bc)
	DECbc,         // dec bc
	INCc,          // inc c
	DECc,          // dec c
	LD05,          // ld c,*
	RRCA,          // rrca
	DJNZ = 0x10,   // djnz *
	LD06,          // ld de,**
	LDde_a,        // ld (de),a
	INCde,         // inc de
	INCd,          // inc d
	DECd,          // dec d
	LD08,          // ld d,*
	RLA,           // rla
	JR1,           // jr *
	ADDhl_de,      // add hl,de
	LDa_de,        // ld a,(de)
	DECde,         // dec de
	INCe,          // inc e
	DECe,          // dec e
	LD0A,          // ld e,*
	RRA,           // rra
	JR2 = 0x20,    // jr nz,*
	LD0B,          // ld hl,**
	LD0C,          // ld (**),hl
	INChl,         // inc hl
	INCh,          // inc h
	DECh,          // dec h
	LD0D,          // ld h,*
	DAA,           // daa
	JR3,           // jr z,*
	ADDhl_hl,      // add hl,hl
	LD0E,          // ld hl,(**)
	DEChl,         // dec hl
	INCl,          // inc l
	DECl,          // dec l
	LD0F,          // ld l,*
	CPL,           // cpl
	JR4 = 0x30,    // jr nc,*
	LD10,          // ld sp,**
	LD11,          // ld (**),a
	INCsp,         // inc sp
	INChl2,        // inc (hl)
	DEChl2,        // dec (hl)
	LD12,          // ld (hl),*
	SCF,           // scf
	JR5,           // jr c,*
	ADDhl_sp,      // add hl,sp
	LD13,          // ld a,(**)
	DECsp,         // dec sp
	INCa,          // inc a
	DECa,          // dec a
	LD14,          // ld a,*
	CCF,           // ccf
	LDb_b = 0x40,  // ld b,b
	LDb_c,         // ld b,c
	LDb_d,         // ld b,d
	LDb_e,         // ld b,e
	LDb_h,         // ld b,h
	LDb_l,         // ld b,l
	LDb_hl,        // ld b,(hl)
	LDb_a,         // ld b,a
	LDc_b,         // ld c,b
	LDc_c,         // ld c,c
	LDc_d,         // ld c,d
	LDc_e,         // ld c,e
	LDc_h,         // ld c,h
	LDc_l,         // ld c,l
	LDc_hl,        // ld c,(hl)
	LDc_a,         // ld c,a
	LDd_b = 0x50,  // ld d,b
	LDd_c,         // ld d,c
	LDd_d,         // ld d,d
	LDd_e,         // ld d,e
	LDd_h,         // ld d,h
	LDd_l,         // ld d,l
	LDd_hl,        // ld d,(hl)
	LDd_a,         // ld d,a
	LDe_b,         // ld e,b
	LDe_c,         // ld e,c
	LDe_d,         // ld e,d
	LDe_e,         // ld e,e
	LDe_h,         // ld e,h
	LDe_l,         // ld e,l
	LDe_hl,        // ld e,(hl)
	LDe_a,         // ld e,a
	LDh_b = 0x60,  // ld h,b
	LDh_c,         // ld h,c
	LDh_d,         // ld h,d
	LDh_e,         // ld h,e
	LDh_h,         // ld h,h
	LDh_l,         // ld h,l
	LDh_hl,        // ld h,(hl)
	LDh_a,         // ld h,a
	LDl_b,         // ld l,b
	LDl_c,         // ld l,c
	LDl_d,         // ld l,d
	LDl_e,         // ld l,e
	LDl_h,         // ld l,h
	LDl_l,         // ld l,l
	LDl_hl,        // ld l,(hl)
	LDl_a,         // ld l,a
	LDhl_b = 0x70, // ld (hl),b
	LDhl_c,        // ld (hl),c
	LDhl_d,        // ld (hl),d
	LDhl_e,        // ld (hl),e
	LDhl_h,        // ld (hl),h
	LDhl_l,        // ld (hl),l
	HALT,          // halt
	LDhl_a,        // ld (hl),a
	LDa_b,         // ld a,b
	LDa_c,         // ld a,c
	LDa_d,         // ld a,d
	LDa_e,         // ld a,e
	LDa_h,         // ld a,h
	LDa_l,         // ld a,l
	LDa_hl,        // ld a,(hl)
	LDa_a,         // ld a,a
	ADDa_b = 0x80, // add a,b
	ADDa_c,        // add a,c
	ADDa_d,        // add a,d
	ADDa_e,        // add a,e
	ADDa_h,        // add a,h
	ADDa_l,        // add a,l
	ADDa_hl,       // add a,(hl)
	ADDa_a,        // add a,a
	ADCa_b,        // adc a,b
	ADCa_c,        // adc a,c
	ADCa_d,        // adc a,d
	ADCa_e,        // adc a,e
	ADCa_h,        // adc a,h
	ADCa_l,        // adc a,l
	ADCa_hl,       // adc a,(hl)
	ADCa_a,        // adc a,a
	SUBb = 0x90,   // sub b
	SUBc,          // sub c
	SUBd,          // sub d
	SUBe,          // sub e
	SUBh,          // sub h
	SUBl,          // sub l
	SUBhl,         // sub (hl)
	SUBa,          // sub a
	SBCa_b,        // sbc a,b
	SBCa_c,        // sbc a,c
	SBCa_d,        // sbc a,d
	SBCa_e,        // sbc a,e
	SBCa_h,        // sbc a,h
	SBCa_l,        // sbc a,l
	SBCa_hl,       // sbc a,(hl)
	SBCa_a,        // sbc a,a
	ANDb = 0xA0,   // and b
	ANDc,          // and c
	ANDd,          // and d
	ANDe,          // and e
	ANDh,          // and h
	ANDl,          // and l
	ANDhl,         // and (hl)
	ANDa,          // and a
	XORb,          // xor b
	XORc,          // xor c
	XORd,          // xor d
	XORe,          // xor e
	XORh,          // xor h
	XORl,          // xor l
	XORhl,         // xor (hl)
	XORa,          // xor a
	ORb = 0xB0,    // or b
	ORc,           // or c
	ORd,           // or d
	ORe,           // or e
	ORh,           // or h
	ORl,           // or l
	ORhl,          // or (hl)
	ORa,           // or a
	CPb,           // cp b
	CPc,           // cp c
	CPd,           // cp d
	CPe,           // cp e
	CPh,           // cp h
	CPl,           // cp l
	CPhl,          // cp (hl)
	CPa,           // cp a
	RETnz = 0xC0,  // ret nz
	POPbc,         // pop bc
	JPnz_,         // jp nz,**
	JP_,           // jp **
	CALLnz_,       // call nz,**
	PUSHbc,        // push bc
	ADDa_,         // add a,*
	RST00h,        // rst 00h
	RETz,          // ret z
	RET,           // ret
	JPz_,          // jp z,**
	BITS,          // #BITS#
	CALLz_,        // call z,**
	CALL_,         // call **
	ADCa_,         // adc a,*
	RST08h,        // rst 08h
	RETnc = 0xD0,  // ret nc
	POPde,         // pop de
	JPnc_,         // jp nc,**
	OUT_a,         // out (*),a
	CALLnc_,       // call nc,**
	PUSHde,        // push de
	SUB_,          // sub *
	RST10h,        // rst 10h
	RETc,          // ret c
	EXX,           // exx
	JPc_,          // jp c,**
	INa_,          // in a,(*)
	CALLc_,        // call c,**
	IX,            // #IX#
	SBCa_,         // sbc a,*
	RST18h,        // rst 18h
	RETpo = 0xE0,  // ret po
	POPhl,         // pop hl
	JPpo_,         // jp po,**
	EXsp_hl,       // ex (sp),hl
	CALLpo_,       // call po,*
	PUSHhl,        // push hl
	AND_,          // and *
	RST20h,        // rst 20h
	RETpe,         // ret pe
	JPhl,          // jp (hl)
	JPpe_,         // jp pe,**
	EXde_hl,       // ex de,hl
	CALLpe_,       // call pe,**
	EXTD,          // #EXTD#
	XOR_,          // xor *
	RST28h,        // rst 28h
	RETp = 0xF0,   // ret p
	POPaf,         // pop af
	JPp_,          // jp p,**
	DI,            // di
	CALLp_,        // call p,**
	PUSHaf,        // push af
	OR_,           // or *
	RST30h,        // rst 30h
	RETm,          // ret m
	LDsp_hl,       // ld sp,hl
	JPm_,          // jp m,**
	EI,            // ei
	CALLm_,        // call m,**
	IY,            // #IY#
	CP_,           // cp *
	RST38h,        // rst 38h
};


#endif /* INTERPRETER_HPP */
