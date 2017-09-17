/*
 * cpu.h
 *
 *  Created on: Sep 11, 2017
 *      Author: nafiu
 */

#ifndef CPU_H_
#define CPU_H_

#include <iostream>
#include <cstdint>
#include <cstring>

#include <endian.h>


#define RAM_SIZE 0xFFFFF

typedef uint_least16_t u16;
typedef int_least16_t s16;
typedef uint_least8_t u8;
typedef int_least8_t s8;

typedef int16_t WORD;
typedef int32_t DWORD;
typedef int64_t QUADWORD;

#define SET_HIGH_BYTE(register_, value ) ( ( (register_) & 0xFF ) | ( (value)<<0x8 ) )
#define SET_LOW_BYTE(register_, value) ( (register_ & (0xff00)) | (value) )

#define GET_HIGH_BYTE(register_) ( ((register_)>>0x8) )
#define GET_LOW_BYTE(register_) ( ((register_) & 0xFF) )

/*
 * This 8086 microprocessor is a 16bit CPU with 16bit registers and thus it
 * cannot address above 2^10(64K). To solve this problem it uses main memory segmentation.
 * The macro below help generate 20bit address.
 */
#define ADDR_TO_20BITS(addr_16bit, offset) ( ((addr_16bit)<<0x4) + (offset) )
#define ADDR_TO_16BITS(addr_16bit) ( ((addr_16bit)>>4) )

class _8086cpu {
private:
	struct REGISTER {
		u16 AC; //Accumulator register
		u16 CX; //count register
		u16 BX; //base address register
		u16 DX; // data register

		u16 BP[1]; //base pointer
		u16 SP[1];//stack pointer

		u16 SI; //source index register
		u16 DI; //destination index register

		u16 EFLAGS; //flag register
		u16 IP[1]; //instruction pointer

		u16 DS[1]; //Data segment
		u16 SS[1]; //stack segment
		u16 CS[1]; //code segment
		u16 ES[1]; //Extra segment
	} *reg;

	s8 *mem; //main memory
public:
	enum FLAG { CF=0x0, PF=0x2, AF=0x4, ZF=0x6, SF=0x7, TF=0x8, \
		IF=0x9, DF=0xA, OF=0xB };

	/*
	 * 	|			ADDRESS-RANGE				|	SEGMENT NUMBER	|
	 * 	|---------------------------------------|-------------------|
	 *	|DS = 1B580(112000) -> 2B580(177536)	|		1B58		|
	 *	|CS = 46B00(289536) -> 56B00(355072)	|		46B0		|
	 *	|SS = 72080(467072) -> 82080(532608)	|		7208		|
	 *	|ES = 9D600(644608) -> AD600(710144)	|		9D60		|
	 *	|-----------------------------------------------------------|
	*/

	_8086cpu();
	~_8086cpu();

	//memory access funcs
	friend WORD read_word(const void *mem);
	friend DWORD read_dword(const void *mem);
	friend QUADWORD read_quadword(const void *mem);
	friend bool write_word( _8086cpu mem, WORD);
	friend bool write_dword(_8086cpu mem, DWORD);
	friend bool write_quadword(_8086cpu mem, QUADWORD);

	bool load(std::string bin_name, unsigned int &sizeOfbin);

	void set_ac_reg(u16 tmp) { reg->AC = tmp; }
	void set_al_reg(u8 tmp) { reg->AC = (u16)SET_LOW_BYTE(reg->AC, tmp); }
	void set_ah_reg(u8 tmp) { reg->AC = (u16)SET_HIGH_BYTE(reg->AC, tmp); }

	void set_cx_reg(u16 tmp) { reg->CX = tmp; }
	void set_cl_reg(u8 tmp) { reg->CX = (u16)SET_LOW_BYTE(reg->CX, tmp); }
	void set_ch_reg(u8 tmp) {  reg->CX = (u16)SET_HIGH_BYTE(reg->CX, tmp); }

	void set_bx_reg(u16 tmp) { reg->BX = tmp; }
	void set_bl_reg(u8 tmp) { reg->BX = (u16)SET_LOW_BYTE(reg->BX, tmp); }
	void set_bh_reg(u8 tmp) {  reg->BX = (u16)SET_HIGH_BYTE(reg->BX, tmp); }

	void set_dx_reg(u16 tmp) { reg->DX = tmp; }
	void set_dl_reg(u8 tmp) { reg->DX = (u16)SET_LOW_BYTE(reg->DX, tmp); }
	void set_dh_reg(u8 tmp) {  reg->DX = (u16)SET_HIGH_BYTE(reg->DX, tmp); }

	s16 get_ac_reg() { return reg->AC; }
	s8 get_al_reg() { return (u8)GET_LOW_BYTE(reg->AC); }
	s8 get_ah_reg() { return (u8)GET_HIGH_BYTE(reg->AC); }

	s16 get_cx_reg() { return reg->CX; }
	s8 get_cl_reg() { return (u8)GET_LOW_BYTE(reg->CX); }
	s8 get_ch_reg() { return (u8)GET_HIGH_BYTE(reg->CX); }

	s16 get_bx_reg() { return reg->BX; }
	s8 get_bl_reg() { return (u8)GET_LOW_BYTE(reg->BX); }
	s8 get_bh_reg() { return (u8)GET_HIGH_BYTE(reg->BX); }

	s16 get_dx_reg() { return reg->DX; }
	s8 get_dl_reg() { return (u8)GET_LOW_BYTE(reg->DX); }
	s8 get_dh_reg() { return (u8)GET_HIGH_BYTE(reg->DX); }

	void set_DI_reg(s16 tmp) { reg->DI = tmp; }
	void set_SI_reg(s16 tmp) { reg->SI = tmp; }
	u16 get_SI_reg() { return reg->SI; }
	u16 get_DI_reg() { return  reg->DI; }


	void set_EFLAGS_reg(FLAG flag) { reg->EFLAGS = reg->EFLAGS ^ (0x1<<flag);  }
	bool flag_is_set(FLAG flag) { return reg->EFLAGS & (0x1<<flag); }

	void running(unsigned int sizeOfBin);
	void debug_cpu();
};


#endif /* CPU_H_ */
