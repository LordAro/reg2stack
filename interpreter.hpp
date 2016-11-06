
#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <cstdint>
#include <functional>

enum OP {
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

struct Instruction {
	OP op;
	std::string str;
	size_t args;
	std::function<void(void)> func;
};

struct InsBase {
	std::string str;
	size_t args;
	std::function<void(const std::string&, const std::string&)> func;
};

static const std::array<InsBase, 13> ins_bases = {{
	{"LD", 2, 0x0},
	{"INC", 1, 0x0},
	{"DEC", 1, 0x0},
	{"ADD", 2, 0x0},
	{"ADC", 2, 0x0},
	{"SUB", 1, 0x0},
	{"SBC", 2, 0x0},
	{"AND", 1, 0x0},
	{"OR", 1, 0x0},
	{"XOR", 1, 0x0},
	{"CP", 1, 0x0},
	{"JP", 2, 0x0},
	{"JR", 2, 0x0},
}};

/*
static const std::array<Instruction, 1> instructions = {
	{NOP,       "NOP",   0,   0x0},
	{LD01,      "LD",    2,   0x0},
	{LDbc_a,    "LD",    2,   0x0},
	{INCbc,     "INC",   1,   0x0},
	{INCb,      "INC",   1,   0x0},
	{DECb,      "DEC",   1,   0x0},
	{LD03,      "LD",    2,   0x0},
	{RLCA,      "RLCA",  0,   0x0},
	{EX1,       "EX",    2,   0x0},
	{ADDhl_bc,  "ADD",   2,   0x0},
	{LDa_bc,    "LD",    2,   0x0},
	{DECbc,     "DEC",   1,   0x0},
	{INCc,      "INC",      ,    0x0},  //     inc     c
	{DECc,      "DEC",      ,    0x0},  //     dec     c
	{LD05,      "LD",      ,    0x0},  //     ld      c,*
	{RRCA,      "",      ,    0x0},  //     rrca
	{DJNZ,      "",      ,    0x0},  //      djnz     *
	{LD06,      "LD",      ,    0x0},  //     ld      de,**
	{LDde_a,    "LD",      ,    0x0},  //     ld      (de),a
	{INCde,     "INC",      ,    0x0},  //     inc     de
	{INCd,      "INC",      ,    0x0},  //     inc     d
	{DECd,      "DEC",      ,    0x0},  //     dec     d
	{LD08,      "LD",      ,    0x0},  //     ld      d,*
	{RLA,       "",      ,    0x0},  //     rla
	{JR1,       "",      ,    0x0},  //     jr      *
	{ADDhl_de,  "",      ,    0x0},  //     add     hl,de
	{LDa_de,    "LD",      ,    0x0},  //     ld      a,(de)
	{DECde,     "DEC",      ,    0x0},  //     dec     de
	{INCe,      "INC",      ,    0x0},  //     inc     e
	{DECe,      "DEC",      ,    0x0},  //     dec     e
	{LD0A,      "LD",      ,    0x0},  //     ld      e,*
	{RRA,       "",      ,    0x0},  //     rra
	{JR2        ,        "",  ,      0x0},  //      jr       nz,*
	{LD0B,      "LD",      ,    0x0},  //     ld      hl,**
	{LD0C,      "LD",      ,    0x0},  //     ld      (**),hl
	{INChl,     "INC",      ,    0x0},  //     inc     hl
	{INCh,      "INC",      ,    0x0},  //     inc     h
	{DECh,      "DEC",      ,    0x0},  //     dec     h
	{LD0D,      "LD",      ,    0x0},  //     ld      h,*
	{DAA,       "",      ,    0x0},  //     daa
	{JR3,       "",      ,    0x0},  //     jr      z,*
	{ADDhl_hl,  "",      ,    0x0},  //     add     hl,hl
	{LD0E,      "LD",      ,    0x0},  //     ld      hl,(**)
	{DEChl,     "DEC",      1,    0x0},  //     dec     hl
	{INCl,      "INC",      1,    0x0},  //     inc     l
	{DECl,      "DEC",      1,    0x0},  //     dec     l
	{LD0F,      "LD",      2,    0x0},  //     ld      l,*
	{CPL,       "CPL",      0,    0x0},  //     cpl
	{JR4,        "JR",  2,      0x0},  //      jr       nc,*
	{LD10,      "LD",      2,    0x0},  //     ld      sp,**
	{LD11,      "LD",      2,    0x0},  //     ld      (**),a
	{INCsp,     "INC",      1,    0x0},  //     inc     sp
	{INChl2,    "INC",      1,    0x0},  //     inc     (hl)
	{DEChl2,    "DEC",      1,    0x0},  //     dec     (hl)
	{LD12,      "LD",      2,    0x0},  //     ld      (hl),*
	{SCF,       "SCF",      0,    0x0},  //     scf
	{JR5,       "JR",      2,    0x0},  //     jr      c,*
	{ADDhl_sp,  "ADD",      2,    0x0},  //     add     hl,sp
	{LD13,      "LD",      2,    0x0},  //     ld      a,(**)
	{DECsp,     "DEC",      1,    0x0},  //     dec     sp
	{INCa,      "INC",      1,    0x0},  //     inc     a
	{DECa,      "DEC",      1,    0x0},  //     dec     a
	{LD14,      "LD",      2,    0x0},  //     ld      a,*
	{CCF,       "CCF",      0,    0x0},  //     ccf
	{LDb_b,     "LD", 2, 0x0},  //     ld       b,b
	{LDb_c,     "LD", 2, 0x0},  //     ld      b,c
	{LDb_d,     "LD", 2, 0x0},  //     ld      b,d
	{LDb_e,     "LD", 2, 0x0},  //     ld      b,e
	{LDb_h,     "LD", 2, 0x0},  //     ld      b,h
	{LDb_l,     "LD", 2, 0x0},  //     ld      b,l
	{LDb_hl,    "LD", 2, 0x0},  //     ld      b,(hl)
	{LDb_a,     "LD", 2, 0x0},  //     ld      b,a
	{LDc_b,     "LD", 2, 0x0},  //     ld      c,b
	{LDc_c,     "LD", 2, 0x0},  //     ld      c,c
	{LDc_d,     "LD", 2, 0x0},  //     ld      c,d
	{LDc_e,     "LD", 2, 0x0},  //     ld      c,e
	{LDc_h,     "LD", 2, 0x0},  //     ld      c,h
	{LDc_l,     "LD", 2, 0x0},  //     ld      c,l
	{LDc_hl,    "LD", 2, 0x0},  //     ld      c,(hl)
	{LDc_a,     "LD", 2, 0x0},  //     ld      c,a
	{LDd_b,     "LD", 2, 0x0},  //     ld       d,b
	{LDd_c,     "LD", 2, 0x0},  //     ld      d,c
	{LDd_d,     "LD", 2, 0x0},  //     ld      d,d
	{LDd_e,     "LD", 2, 0x0},  //     ld      d,e
	{LDd_h,     "LD", 2, 0x0},  //     ld      d,h
	{LDd_l,     "LD", 2, 0x0},  //     ld      d,l
	{LDd_hl,    "LD", 2, 0x0},  //     ld      d,(hl)
	{LDd_a,     "LD", 2, 0x0},  //     ld      d,a
	{LDe_b,     "LD", 2, 0x0},  //     ld      e,b
	{LDe_c,     "LD", 2, 0x0},  //     ld      e,c
	{LDe_d,     "LD", 2, 0x0},  //     ld      e,d
	{LDe_e,     "LD", 2, 0x0},  //     ld      e,e
	{LDe_h,     "LD", 2, 0x0},  //     ld      e,h
	{LDe_l,     "LD", 2, 0x0},  //     ld      e,l
	{LDe_hl,    "LD", 2, 0x0},  //     ld      e,(hl)
	{LDe_a,     "LD", 2, 0x0},  //     ld      e,a
	{LDh_b,     "LD", 2, 0x0},  //     ld       h,b
	{LDh_c,     "LD", 2, 0x0},  //     ld      h,c
	{LDh_d,     "LD", 2, 0x0},  //     ld      h,d
	{LDh_e,     "LD", 2, 0x0},  //     ld      h,e
	{LDh_h,     "LD", 2, 0x0},  //     ld      h,h
	{LDh_l,     "LD", 2, 0x0},  //     ld      h,l
	{LDh_hl,    "LD", 2, 0x0},  //     ld      h,(hl)
	{LDh_a,     "LD", 2, 0x0},  //     ld      h,a
	{LDl_b,     "LD", 2, 0x0},  //     ld      l,b
	{LDl_c,     "LD", 2, 0x0},  //     ld      l,c
	{LDl_d,     "LD", 2, 0x0},  //     ld      l,d
	{LDl_e,     "LD", 2, 0x0},  //     ld      l,e
	{LDl_h,     "LD", 2, 0x0},  //     ld      l,h
	{LDl_l,     "LD", 2, 0x0},  //     ld      l,l
	{LDl_hl,    "LD", 2, 0x0},  //     ld      l,(hl)
	{LDl_a,     "LD", 2, 0x0},  //     ld      l,a
	{LDhl_b,    "LD", 2, 0x0},  //     ld       (hl),b
	{LDhl_c,    "LD", 2, 0x0},  //     ld      (hl),c
	{LDhl_d,    "LD", 2, 0x0},  //     ld      (hl),d
	{LDhl_e,    "LD", 2, 0x0},  //     ld      (hl),e
	{LDhl_h,    "LD", 2, 0x0},  //     ld      (hl),h
	{LDhl_l,    "LD", 2, 0x0},  //     ld      (hl),l
	{HALT,      "HALT", 0,    0x0},  //     halt
	{LDhl_a,    "LD", 2,    0x0},  //     ld      (hl),a
	{LDa_b,     "LD", 2,    0x0},  //     ld      a,b
	{LDa_c,     "LD", 2,    0x0},  //     ld      a,c
	{LDa_d,     "LD", 2,    0x0},  //     ld      a,d
	{LDa_e,     "LD", 2,    0x0},  //     ld      a,e
	{LDa_h,     "LD", 2,    0x0},  //     ld      a,h
	{LDa_l,     "LD", 2,    0x0},  //     ld      a,l
	{LDa_hl,    "LD", 2,    0x0},  //     ld      a,(hl)
	{LDa_a,     "LD", 2,    0x0},  //     ld      a,a
	{ADDa_b,    "ADD", 2,    0x0},  //      add      a,b
	{ADDa_c,    "ADD", 2,    0x0},  //     add     a,c
	{ADDa_d,    "ADD", 2,    0x0},  //     add     a,d
	{ADDa_e,    "ADD", 2,    0x0},  //     add     a,e
	{ADDa_h,    "ADD", 2,    0x0},  //     add     a,h
	{ADDa_l,    "ADD", 2,    0x0},  //     add     a,l
	{ADDa_hl,   "ADD", 2,    0x0},  //     add     a,(hl)
	{ADDa_a,    "ADD", 2,    0x0},  //     add     a,a
	{ADCa_b,    "ADC", 2,    0x0},  //     adc     a,b
	{ADCa_c,    "ADC", 2,    0x0},  //     adc     a,c
	{ADCa_d,    "ADC", 2,    0x0},  //     adc     a,d
	{ADCa_e,    "ADC", 2,    0x0},  //     adc     a,e
	{ADCa_h,    "ADC", 2,    0x0},  //     adc     a,h
	{ADCa_l,    "ADC", 2,    0x0},  //     adc     a,l
	{ADCa_hl,   "ADC", 2,    0x0},  //     adc     a,(hl)
	{ADCa_a,    "ADC", 2,    0x0},  //     adc     a,a
	{SUBb,      "SUB", 1,    0x0},  //      sub      b
	{SUBc,      "SUB", 1,    0x0},  //     sub     c
	{SUBd,      "SUB", 1,    0x0},  //     sub     d
	{SUBe,      "SUB", 1,    0x0},  //     sub     e
	{SUBh,      "SUB", 1,    0x0},  //     sub     h
	{SUBl,      "SUB", 1,    0x0},  //     sub     l
	{SUBhl,     "SUB", 1,    0x0},  //     sub     (hl)
	{SUBa,      "SUB", 1,    0x0},  //     sub     a
	{SBCa_b,    "SBC", 2,    0x0},  //     sbc     a,b
	{SBCa_c,    "SBC", 2,    0x0},  //     sbc     a,c
	{SBCa_d,    "SBC", 2,    0x0},  //     sbc     a,d
	{SBCa_e,    "SBC", 2,    0x0},  //     sbc     a,e
	{SBCa_h,    "SBC", 2,    0x0},  //     sbc     a,h
	{SBCa_l,    "SBC", 2,    0x0},  //     sbc     a,l
	{SBCa_hl,   "SBC", 2,    0x0},  //     sbc     a,(hl)
	{SBCa_a,    "SBC", 2,    0x0},  //     sbc     a,a
	{ANDb,      "AND", 1,    0x0},  //      and      b
	{ANDc,      "AND", 1,    0x0},  //     and     c
	{ANDd,      "AND", 1,    0x0},  //     and     d
	{ANDe,      "AND", 1,    0x0},  //     and     e
	{ANDh,      "AND", 1,    0x0},  //     and     h
	{ANDl,      "AND", 1,    0x0},  //     and     l
	{ANDhl,     "AND", 1,    0x0},  //     and     (hl)
	{ANDa,      "AND", 1,    0x0},  //     and     a
	{XORb,      "XOR", 1,    0x0},  //     xor     b
	{XORc,      "XOR", 1,    0x0},  //     xor     c
	{XORd,      "XOR", 1,    0x0},  //     xor     d
	{XORe,      "XOR", 1,    0x0},  //     xor     e
	{XORh,      "XOR", 1,    0x0},  //     xor     h
	{XORl,      "XOR", 1,    0x0},  //     xor     l
	{XORhl,     "XOR", 1,    0x0},  //     xor     (hl)
	{XORa,      "XOR", 1,    0x0},  //     xor     a
	{ORb,       "OR", 1,    0x0},  //      or       b
	{ORc,       "OR", 1,    0x0},  //     or      c
	{ORd,       "OR", 1,    0x0},  //     or      d
	{ORe,       "OR", 1,    0x0},  //     or      e
	{ORh,       "OR", 1,    0x0},  //     or      h
	{ORl,       "OR", 1,    0x0},  //     or      l
	{ORhl,      "OR", 1,    0x0},  //     or      (hl)
	{ORa,       "OR", 1,    0x0},  //     or      a
	{CPb,       "CP",     1,    0x0},  //     cp      b
	{CPc,       "CP",     1,    0x0},  //     cp      c
	{CPd,       "CP",     1,    0x0},  //     cp      d
	{CPe,       "CP",     1,    0x0},  //     cp      e
	{CPh,       "CP",     1,    0x0},  //     cp      h
	{CPl,       "CP",     1,    0x0},  //     cp      l
	{CPhl,      "CP",     1,    0x0},  //     cp      (hl)
	{CPa,       "CP",     1,    0x0},  //     cp      a
	{RETnz,     "",      ,    0x0},  //      ret      nz
	{POPbc,     "",      ,    0x0},  //     pop     bc
	{JPnz_,     "",      ,    0x0},  //     jp      nz,**
	{JP_,       "",      ,    0x0},  //     jp      **
	{CALLnz_,   "",      ,    0x0},  //     call    nz,**
	{PUSHbc,    "",      ,    0x0},  //     push    bc
	{ADDa_,     "",      ,    0x0},  //     add     a,*
	{RST00h,    "",      ,    0x0},  //     rst     00h
	{RETz,      "",      ,    0x0},  //     ret     z
	{RET,       "",      ,    0x0},  //     ret
	{JPz_,      "",      ,    0x0},  //     jp      z,**
	{BITS,      "",      ,    0x0},  //     #BITS#
	{CALLz_,    "",      ,    0x0},  //     call    z,**
	{CALL_,     "",      ,    0x0},  //     call    **
	{ADCa_,     "",      ,    0x0},  //     adc     a,*
	{RST08h,    "",      ,    0x0},  //     rst     08h
	{RETnc,     "",      ,    0x0},  //      ret      nc
	{POPde,     "",      ,    0x0},  //     pop     de
	{JPnc_,     "",      ,    0x0},  //     jp      nc,**
	{OUT_a,     "",      ,    0x0},  //     out     (*),a
	{CALLnc_,   "",      ,    0x0},  //     call    nc,**
	{PUSHde,    "",      ,    0x0},  //     push    de
	{SUB_,      "",      ,    0x0},  //     sub     *
	{RST10h,    "",      ,    0x0},  //     rst     10h
	{RETc,      "",      ,    0x0},  //     ret     c
	{EXX,       "",      ,    0x0},  //     exx
	{JPc_,      "",      ,    0x0},  //     jp      c,**
	{INa_,      "",      ,    0x0},  //     in      a,(*)
	{CALLc_,    "",      ,    0x0},  //     call    c,**
	{IX,        "",      ,    0x0},  //     #IX#
	{SBCa_,     "",      ,    0x0},  //     sbc     a,*
	{RST18h,    "",      ,    0x0},  //     rst     18h
	{RETpo,     "",      ,    0x0},  //      ret      po
	{POPhl,     "",      ,    0x0},  //     pop     hl
	{JPpo_,     "",      ,    0x0},  //     jp      po,**
	{EXsp_hl,   "",      ,    0x0},  //     ex      (sp),hl
	{CALLpo_,   "",      ,    0x0},  //     call    po,*
	{PUSHhl,    "",      ,    0x0},  //     push    hl
	{AND_,      "",      ,    0x0},  //     and     *
	{RST20h,    "",      ,    0x0},  //     rst     20h
	{RETpe,     "",      ,    0x0},  //     ret     pe
	{JPhl,      "",      ,    0x0},  //     jp      (hl)
	{JPpe_,     "",      ,    0x0},  //     jp      pe,**
	{EXde_hl,   "",      ,    0x0},  //     ex      de,hl
	{CALLpe_,   "",      ,    0x0},  //     call    pe,**
	{EXTD,      "",      ,    0x0},  //     #EXTD#
	{XOR_,      "",      ,    0x0},  //     xor     *
	{RST28h,    "",      ,    0x0},  //     rst     28h
	{RETp,      "",      ,    0x0},  //      ret      p
	{POPaf,     "",      ,    0x0},  //     pop     af
	{JPp_,      "",      ,    0x0},  //     jp      p,**
	{DI,        "",      ,    0x0},  //     di
	{CALLp_,    "",      ,    0x0},  //     call    p,**
	{PUSHaf,    "",      ,    0x0},  //     push    af
	{OR_,       "",      ,    0x0},  //     or      *
	{RST30h,    "",      ,    0x0},  //     rst     30h
	{RETm,      "",      ,    0x0},  //     ret     m
	{LDsp_hl,   "",      ,    0x0},  //     ld      sp,hl
	{JPm_,      "",      ,    0x0},  //     jp      m,**
	{EI,        "",      ,    0x0},  //     ei
	{CALLm_,    "",      ,    0x0},  //     call    m,**
	{IY,        "",      ,    0x0},  //     #IY#
	{CP_,       "",      ,    0x0},  //     cp      *
	{RST38h,    "",      ,    0x0},  //     rst     38h
};
*/

//static const char *instruction_str[256] {
//};

static const void (*instruction_map[16][16]) = {
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
};

struct z80reg {
	uint8_t a;
	uint8_t f; // Flags
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
};

struct z80prog {
	z80reg main; // main registers
	z80reg alt;  // shadow registers

	uint16_t ix;  // index x
	uint16_t iy;  // index u

	uint8_t i;    // interrupt vector
	uint8_t r;    // refresh counter

	uint16_t sp;  // stack pointer
	uint16_t pc;  // program counter
};

z80prog tokeniseSource(const std::string &source);

#endif /* INTERPRETER_HPP */
