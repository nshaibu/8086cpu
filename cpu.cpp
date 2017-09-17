/*
 * cpu.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: nafiu
 */

#include <cstdlib>
#include <new>
#include <cmath>
#include <exception>
#include <fstream>
#include <stdio.h>

#include <stdio.h>
#include <errno.h>

#include "./cpu.h"

u16 CONVERT_INTO_WORD(u8 first, u8 second) { return (u16)( ((first)<<0x8) | (second) ); }

void _8086cpu::debug_cpu() {
	std::cout<<"The registers:" <<std::endl;
	std::cout<<"Accumulator    (AC):  "<<reg->AC<<std::endl;
	printf("[   %d   |   %d   ]\n", get_ah_reg(), get_al_reg());

	std::cout<<"Base register  (BX):   "<<reg->BX<<std::endl;
	printf("[   %d   |   %d   ]\n", get_bh_reg(), get_bl_reg());

	std::cout<<"Counter register(BX):   "<<reg->CX<<std::endl;
	printf("[   %d   |   %d   ]\n", get_ch_reg(), get_ch_reg());

	std::cout<<"Data register   (DX):   "<<reg->DX<<std::endl;
	printf("[   %d   |   %d   ]\n\n", get_dh_reg(), get_dl_reg());

	std::cout<<"Base  pointer(BP):   "<<std::hex<<ADDR_TO_20BITS(*(reg->BP), 0)<<std::endl;
	std::cout<<"Stack pointer(SP):   "<<std::hex<<ADDR_TO_20BITS(*(reg->SP), 0)<<std::endl;
	std::cout<<"Source index register(SI):   "<<reg->SI<<std::endl;
	std::cout<<"Destination index register(DI):   "<<reg->DI<<"\n\n";

	std::cout << "Code Segment:"<<std::hex<<ADDR_TO_20BITS(*(reg->CS), 0)<<std::endl;
	unsigned offset=0;
	std::cout<<"Address   "<<"Offset1   " << "offset2   " <<"offset3   "<<std::endl;
	while (true) {
		int address = ADDR_TO_20BITS(*(reg->CS), offset); ++offset;
		std::cout<<std::hex<<address;
		for (unsigned i=0; i<3; i++) std::cout<<"        "<<mem[address++];
		std::cout<<std::endl;
		if(mem[address] == '\0') break;;
	}

	std::cout << "Stack Segment:"<<std::hex<<ADDR_TO_20BITS(*(reg->SS), 0)<<std::endl;
	offset=0;
	std::cout<<"Address   "<<"Offset1   " << "offset2   " <<"offset3   "<<std::endl;
	while (true) {
		int address = ADDR_TO_20BITS(*(reg->SS), offset); ++offset;
		std::cout<<std::hex<<address;
		for (unsigned i=0; i<3; i++) std::cout<<"        "<<mem[address++];
		std::cout<<std::endl;
		if(mem[address] == '\0') break;;
	}

	std::cout << "Data Segment:"<<std::hex<<ADDR_TO_20BITS(*(reg->DS), 0)<<std::endl;
	offset=0;
	std::cout<<"Address   "<<"Offset1   " << "offset2   " <<"offset3   "<<std::endl;
	while (true) {
		int address = ADDR_TO_20BITS(*(reg->DS), offset); ++offset;
		std::cout<<std::hex<<address;
		for (unsigned i=0; i<3; i++) std::cout<<"        "<<mem[address++];
		std::cout<<std::endl;
		if(mem[address] == '\0') break;;
	}

}

_8086cpu::_8086cpu() {
		mem = (s8*)calloc(RAM_SIZE, sizeof(s8));
		reg = (REGISTER*)calloc(1, sizeof(REGISTER));

		if (reg == NULL || mem == NULL) {
			perror("bad alloc");
			exit(1);
		}

		//segment the 1M memory
		reg->CS[0] = (u16)(ADDR_TO_16BITS(0x46B00));
		reg->DS[0] = (u16)(ADDR_TO_16BITS(0x1B580));
		reg->SS[0] = (u16)(ADDR_TO_16BITS(0x72080));
		reg->ES[0] = (u16)(ADDR_TO_16BITS(0x9D600));

		reg->BP[0] = (u16)(ADDR_TO_16BITS(0x82080));
		reg->SP[0] = (u16)(ADDR_TO_16BITS(0x82080));
}

_8086cpu::~_8086cpu() { free(reg); free(mem); }

bool _8086cpu::load(std::string bin_name, unsigned int &sizeOfbin) {
	FILE *in = fopen(bin_name.c_str(), "r+b");
	if (in == NULL) {
		perror("binary not opened");
		exit(EXIT_FAILURE);
	}

	unsigned int mem_cs = ADDR_TO_20BITS(*(reg->CS), 0);
	unsigned i = 0;

	//set_ac_reg(65530);
	//set_cx_reg(8562);
	mem[mem_cs++]=0xB8;
	mem[mem_cs++]=18;
	mem[mem_cs++]=15;
	i=2;
	/*while (!feof(in)) {
		fread(&mem[mem_cs++], sizeof(s8), 1, in);
		++i;
	}
*/
	sizeOfbin = i;
	fclose(in);
	return (i < pow(2, 10))? false:true;
}

/*
 * ADC reg/mem with reg    	  000100dw    modregr/m
 * ADC immed to reg/mem       100000sw    mod010r/m
 * ADD reg/mem with reg       000000dw    modregr/m
 * ADD immed to accumulator   0000010w    data
 *
 * ADD immed to reg/mem 	  100000sw    mod000r/m
 * OR reg/mem with reg        000010dw    modregr/m
 * OR immed to reg/mem        100000sw    mod001r/m
 * OR immed to accumlator     0000110w    data
 *
 * INC reg16			      01000reg
 * INC reg/mem                1111111w    mod000r/m
 * MOV reg/mem to/from reg    100010dw    modregr/m
 * MOV reg/mem to segreg      10001110    modsegr/m
 * MOV immed to reg/mem       1100011w    mod000r/m
 * MOV immed to reg           1011wreg    data
 * MOV direct mem to/from acc 101000dw    addr
 *
 * XCHG reg/mem with reg      1000011w    modregr/m
 * XCHG reg16 with accum.     10010reg
 * CMP reg/mem with reg       001110dw    modregr/m
 * CMP immed to accumulator   0011110w    data
 * CMP immed to reg/mem       100000sw    mod111r/m
 *
 * POP reg                    01011reg
 * POP segreg                 00reg111
 * POP reg/mem                10001111    modxxxr/m
 * RCL reg/mem,CL/immediate   110100cw    mod010r/m ((not implemented yet))
 *
 * RCR reg/mem,CL/immediate   110100cw    mod011r/m ((not implemented yet))
 * STOS                       1010101w
 * CMPS                       1010011w
 * MUL reg/mem                1111011w    mod100r/m
 * IMUL Integer Multiply(Signed) 1111011w mod101r/m
 *
 * AAA  ASCII Adjust for Add 	    00110111
 * BAA  Decimal Adjust for Add 	    00100111
 * AAS  ASCII Adjust for Subtract 	00111111
 * DAS  Decimal Adjust for Subtract  00101111
 *
 * SUB Reg./Memory and Register to Either  001010dw  modregr/m
 * SUB Immediate from Register/Memory      100000sw  mod101r/m  data
 * SUB Immediate from Accumulator          0010110w  data
 *
 * LEA Load EA to Register                 10001101  modregr/m
 * LDS Load Pointer to DS                  11000101  modregr/m
 * LES Load Pointer to ES                  11000100  modregr/m
 * LAHF Load AH with Flags                 10011111
 * SAHF Store AH into Flags                10011110
 * PUSHF Push Flags                        10011100
 * POPF e Pop Flags                        10011101
 *
 */

void _8086cpu::running(unsigned int sizeOfBin) {
	reg->IP[0] = *(reg->CS);

	int counter = 0;
	u8 addr_mode, opcode, tmp=0;
	unsigned offset = 0, ExitProgram = 0;

	for (;;){
		opcode = mem[ADDR_TO_20BITS(*(reg->IP), offset)];
		offset++;
		ExitProgram++;
		switch(opcode) {
		    case 0x28: break;//SUB reg/mem to reg to either in byte
		    case 0x29: break;//SUB reg/mem to reg to either in word
		    case 0x2A: break;//SUB reg/mem to reg  to either in byte
		    case 0x2B: break;//SUB reg/mem to reg to either in word
		    case 0x2D: break;//SUB immed from accumulator in word
		    case 0x2C: break;//SUB immed from acc in byte

			case 0x37: break;//AAA
			case 0x27: break;//BAA
			case 0x3F: break;//AAS
			case 0x2F: break;//DAS

			case 0xF6: break;//MUL reg/mem in byte -->Also IMUL
			case 0xF7: break;//MUL reg/mem in word -->Also IMUL

			case 0xAA: break;//STOS in byte
			case 0xAB: break;//STOS in word

			case 0xA6: break;//CMPS in byte
			case 0xA7: break;//CMPS in word

			case 0x8F: break;//POP reg/mem
			case 0x07: break;//POP ES
			case 0x0F: break;//POP CS
			case 0x17: break;//POP SS
			case 0x1F: break;//POP DS
			case 0x58: break;//POP AX
			case 0x59: break;//POP CX
			case 0x5A: break;//POP DX
			case 0x5B: break;//POP BX
			case 0x5C: break;//POP SP
			case 0x5D: break;//POP BP
			case 0x5E: break;//POP SI
			case 0x5F: break;//POP DI

			case 0x86: break;//XCHG reg/mem with reg in byte
			case 0x87: break;//XCHG reg/mem with reg in word
			case 0x90: break;//XCHG reg16 with AC ie AC with itself redundant
			case 0x91: break;//XCHG reg16 with CX
			case 0x92: break;//XCHG reg16 with DX
			case 0x93: break;//XCHG reg16 with BX

			case 0x38: break;//CMP reg/mem with reg in byte
			case 0x39: break;//CMP reg/mem with reg in word
			case 0x3A: break;//CMP reg/mem with reg in byte
			case 0x3B: break;//CMP reg/mem with reg in word
			case 0x3C: break;//CMP immed to AC in byte
			case 0x3D: break;//CMP immed to AC in word

			case 0x10: break;//ADC reg to reg/mem in byte
			case 0x11: break;//ADC reg to reg/mem in word
			case 0x12: break;//ADC reg/mem to reg in byte
			case 0x13: break;//ADC reg/mem to reg in word

			case 0x00:  //ADD reg/mem with reg in byte
			case 0x01:  //ADD reg/mem with reg in word
			case 0x02:  //ADD reg/mem to reg in byte
			case 0x03:  //ADD reg/mem to reg in word
			case 0x04:  //ADD immed to accumulator in byte
			case 0x05:  //ADD immed to accumulator in word
			case 0x82: //ADD immed to reg/mem in byte --|__signed extended -------------------------|-->CMP immed to reg/mem in byte->SUB Immed from Reg/Mem in byte
			case 0x83: //ADD immed to reg/mem in word --|->CMP immed to reg/mem in word->SUB Immed from Reg/Mem in word									|--->OR immed to reg/mem
			case 0x80: //ADD immed to reg/mem in byte ----|___no signed extension of byte operand---|->CMP immed to reg/mem in byte->SUB immed from Reg/Mem in byte
			case 0x81: //ADD immed to reg/mem in word ----|-->CMP immed to reg/mem in word->SUB Immed from Regi/Mem in word

			case 0x08:  //OR reg to reg/mem in byte
			case 0x09:  //OR reg to reg/mem in word
			case 0x0A:  //OR reg/mem to reg in byte
			case 0x0B:  //OR reg/mem to reg in word
			case 0x0C:  //OR immed to accumulator in byte
			case 0x0D:  //OR immed to accumulator in word

			case 0x40: break;//INC AL
			case 0x41: break;//INC CX
			case 0x42: break;//INC DX
			case 0x43: break;//INC BX
			case 0x44: break;//INC SP
			case 0x45: break;//INC BP
			case 0x46: break;//INC SI
			case 0x47: break;//INC DI
			case 0xFE: break;//INC reg/mem in byte
			case 0xFF: break;//INC reg/mem in word

			case 0x88: //MOV reg to/from reg in byte
				addr_mode = mem[ADDR_TO_20BITS(*(reg->IP), offset)];
				++offset; ExitProgram++;

				switch(addr_mode) {
					case 0xC1: /*AL to CL */tmp = get_al_reg(); set_cl_reg(tmp); break;
					case 0xC2: /*AL to DL*/ tmp = get_al_reg(); set_dl_reg(tmp); break;
					case 0xC3: /*AL to BL*/ tmp = get_al_reg(); set_bl_reg(tmp); break;
					case 0xC4: /*AL to AH*/ tmp = get_al_reg(); set_ah_reg(tmp); break;
					case 0xC5: /*AL to CH*/ tmp = get_al_reg(); set_ch_reg(tmp); break;
					case 0xC6: /*AL to DH*/ tmp = get_al_reg(); set_dh_reg(tmp); break;
					case 0xC7: /*AL to BH*/ tmp = get_al_reg(); set_bh_reg(tmp); break;
					case 0xC8: /*CL to AL*/ tmp = get_cl_reg(); set_al_reg(tmp); break;
					case 0xCA: /*CL to DL*/ tmp = get_cl_reg(); set_dl_reg(tmp); break;
					case 0xCB: /*CL to BL*/ tmp = get_cl_reg(); set_bl_reg(tmp); break;
					case 0xCC: /*CL to AH*/ tmp = get_cl_reg(); set_ah_reg(tmp); break;
					case 0xCD: /*CL to CH*/ tmp = get_cl_reg(); set_ch_reg(tmp); break;
					case 0xCE: /*CL to DH*/ tmp = get_cl_reg(); set_dh_reg(tmp); break;
					case 0xCF: /*CL to BH*/ tmp = get_cl_reg(); set_bh_reg(tmp); break;
					case 0xD0: /*DL to AL*/ tmp = get_dl_reg(); set_al_reg(tmp); break;
					case 0xD1: /*DL to CL*/ tmp = get_dl_reg(); set_cl_reg(tmp); break;
					case 0xD3: /*DL to BL*/ tmp = get_dl_reg(); set_bl_reg(tmp); break;
					case 0xD4: /*DL to AH*/ tmp = get_dl_reg(); set_ah_reg(tmp); break;
					case 0xD5: /*DL to CH*/ tmp = get_dl_reg(); set_ch_reg(tmp); break;
					case 0xD6: /*dl to dh*/ tmp = get_dl_reg(); set_dh_reg(tmp); break;
					case 0xD7: /*dl to bh*/ tmp = get_dl_reg(); set_bh_reg(tmp); break;
					case 0xD8: /*bl to al*/ tmp = get_bl_reg(); set_al_reg(tmp); break;
					case 0xD9: /*bl to cl*/ tmp = get_bl_reg(); set_cl_reg(tmp); break;
					case 0xDA: /*bl to dl*/ tmp = get_bl_reg(); set_dl_reg(tmp); break;
					case 0xDC: /*bl to ah*/ tmp = get_bl_reg(); set_ah_reg(tmp); break;
					case 0xDD: /*bl to ch*/ tmp = get_bl_reg(); set_ch_reg(tmp); break;
					case 0xDE: /*bl to dh*/ tmp = get_bl_reg(); set_dh_reg(tmp); break;
					case 0xDF: /*bl to bh*/ tmp = get_bl_reg(); set_bh_reg(tmp); break;

					case 0xE0: /*AH to al */tmp = get_ah_reg(); set_al_reg(tmp); break;
					case 0xE1: /*Ah to cL*/ tmp = get_ah_reg(); set_cl_reg(tmp); break;
					case 0xE2: /*Ah to dl*/ tmp = get_ah_reg(); set_dl_reg(tmp); break;
					case 0xE3: /*Ah to bl*/ tmp = get_ah_reg(); set_bl_reg(tmp); break;
					case 0xE7: /*Ah to bH*/ tmp = get_ah_reg(); set_bh_reg(tmp); break;
					case 0xE5: /*Ah to cH*/ tmp = get_ah_reg(); set_ch_reg(tmp); break;
					case 0xE6: /*Ah to dH*/ tmp = get_ah_reg(); set_dh_reg(tmp); break;
					case 0xE8: /*Ch to AL*/ tmp = get_ch_reg(); set_al_reg(tmp); break;
					case 0xE9: /*Ch to cL*/ tmp = get_ch_reg(); set_cl_reg(tmp); break;
					case 0xEA: /*Ch to dL*/ tmp = get_ch_reg(); set_dl_reg(tmp); break;
					case 0xEB: /*Ch to bl*/ tmp = get_ch_reg(); set_bl_reg(tmp); break;
					case 0xEC: /*Ch to aH*/ tmp = get_ch_reg(); set_ah_reg(tmp); break;
					case 0xEE: /*Ch to DH*/ tmp = get_ch_reg(); set_dh_reg(tmp); break;
					case 0xFF: /*Ch to BH*/ tmp = get_ch_reg(); set_bh_reg(tmp); break;
					case 0xF0: /*Dh to AL*/ tmp = get_dh_reg(); set_al_reg(tmp); break;
					case 0xF1: /*Dh to CL*/ tmp = get_dh_reg(); set_cl_reg(tmp); break;
					case 0xF2: /*Dh to dL*/ tmp = get_dh_reg(); set_dl_reg(tmp); break;
					case 0xF3: /*Dh to bl*/ tmp = get_dh_reg(); set_bl_reg(tmp); break;
					case 0xF4: /*Dh to aH*/ tmp = get_dh_reg(); set_ah_reg(tmp); break;
					case 0xF5: /*dh to ch*/ tmp = get_dh_reg(); set_ch_reg(tmp); break;
					case 0xF7: /*dh to bh*/ tmp = get_dh_reg(); set_bh_reg(tmp); break;
					case 0xF8: /*bh to al*/ tmp = get_bh_reg(); set_al_reg(tmp); break;
					case 0xF9: /*bh to cl*/ tmp = get_bh_reg(); set_cl_reg(tmp); break;
					case 0xFA: /*bh to dl*/ tmp = get_bh_reg(); set_dl_reg(tmp); break;
					case 0xFB: /*bh to bl*/ tmp = get_bh_reg(); set_bl_reg(tmp); break;
					case 0xFC: /*bh to ah*/ tmp = get_bh_reg(); set_ah_reg(tmp); break;
					case 0xFD: /*bh to ch*/ tmp = get_bh_reg(); set_ch_reg(tmp); break;
					case 0xFE: /*bh to dh*/ tmp = get_bh_reg(); set_dh_reg(tmp); break;
				}
				break;
			case 0x89: break;//MOV reg to/from reg in word
			case 0x8A: break;//MOV reg/mem to/from reg in byte
			case 0x8B: break;//MOV reg/mem to/from reg in word
			case 0x8E: break;//MOV reg/mem to/from segreg
			case 0xC6: break;//MOV immed to reg/mem in byte
			case 0xC7: break;//MOV immed to reg/mem in word
			case 0xB0: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_al_reg(tmp); break;//MOV immed to AL register
			case 0xB1: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_cl_reg(tmp); break;//MOV immed to CL register
			case 0xB2: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_dl_reg(tmp); break;//MOV immed to DL register
			case 0xB3: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_bl_reg(tmp); break;//MOV immed to BL register
			case 0xB4: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_ah_reg(tmp); break;//MOV immed to AH register
			case 0xB5: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_ch_reg(tmp); break;//MOV immed to CH register
			case 0xB6: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_dh_reg(tmp); break;//MOV immed to DH register
			case 0xB7: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; ++ExitProgram; set_bh_reg(tmp); break;//MOV immed to BH register
			case 0xB8: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; set_ac_reg(CONVERT_INTO_WORD(mem[ADDR_TO_20BITS(*(reg->IP), offset)], tmp)); offset++; ExitProgram+=2; break;//MOV immed to AC register
			case 0xB9: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; set_cx_reg(CONVERT_INTO_WORD(mem[ADDR_TO_20BITS(*(reg->IP), offset)], tmp)); offset++; ExitProgram+=2; break;//MOV immed to CX register
			case 0xBA: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; set_dx_reg(CONVERT_INTO_WORD(mem[ADDR_TO_20BITS(*(reg->IP), offset)], tmp)); offset++; ExitProgram+=2; break;//MOV immed to DX
			case 0xBB: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; set_bx_reg(CONVERT_INTO_WORD(mem[ADDR_TO_20BITS(*(reg->IP), offset)], tmp)); offset++; ExitProgram+=2; break;//MOV immed to BX
			case 0xBC: //MOV immed to SP--->Also for MOV segreg to reg/mem
					addr_mode = mem[ADDR_TO_20BITS(*(reg->IP), offset)];
					++offset;
					++ExitProgram;
					switch(addr_mode) {

					}
				break;
			case 0xBD: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; reg->BP[0]=CONVERT_INTO_WORD(mem[ADDR_TO_20BITS(*(reg->IP), offset)], tmp); offset++; ExitProgram+=2; break;//MOV immed to BP
			case 0xBE: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; set_SI_reg(CONVERT_INTO_WORD(mem[ADDR_TO_20BITS(*(reg->IP), offset)], tmp)); offset++; ExitProgram+=2; break;//MOV immed to SI
			case 0xBF: tmp=mem[ADDR_TO_20BITS(*(reg->IP), offset)]; ++offset; set_DI_reg(CONVERT_INTO_WORD(mem[ADDR_TO_20BITS(*(reg->IP), offset)], tmp)); offset++; ExitProgram+=2; break;//MOV immed to DI
			case 0xA0: break;//MOV direct mem to/from AC in byte
			case 0xA1: break;//MOV direct mem to/from AC in word
			case 0xA2: break;//MOV direct mem to/from AC in byte
			case 0xA3: break;//MOV direct mem to/from AC in word
		}

		if (ExitProgram >= sizeOfBin) break;
		if (counter <= 0) {
			/*TODO hardware interrupts, check system timers, refresh screen
			 * exit program reset virtual registers
			*/
		}
	}
}

