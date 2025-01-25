#include "MOS_6510.h"


MOS_6510::MOS_6510(void)//: flagsw{ 0x01,0x21,0x04,0x24,0x00,0x40,0x08,0x28 }
{
	PC = 0; 
	pPC = 0; 
	addr = 0; 
	storadd = 0;
	A = 0; 
	T = 0; 
	SP = 0xFF;
	X = 0; 
	Y = 0; 
	IR = 0; 
	ST = 0x00;  
	cycles = 0;
}

MOS_6510::MOS_6510(RAM & ram):Ram(ram)
{
	PC = 0; 
	pPC = 0; 
	addr = 0; 
	storadd = 0;
	A = 0; 
	T = 0; 
	SP = 0xFF;
	X = 0; 
	Y = 0; 
	IR = 0; 
	ST = 0x00;
	cycles = 0;
}

MOS_6510::~MOS_6510()
{

}

const byte MOS_6510::flagsw[] = { 0x01,0x21,0x04,0x24,0x00,0x40,0x08,0x28 };

const byte MOS_6510::branchflag[] = { 0x80,0x40,0x01,0x02 };

void MOS_6510::initCPU(unsigned int mempos) { PC = mempos; A = 0; X = 0; Y = 0; ST = 0; SP = 0xFF; }

//My CPU implementation is based on the instruction table by Graham at codebase64.
//After some examination of the table it was clearly seen that columns of the table (instructions' 2nd nybbles)
// mainly correspond to addressing modes, and double-rows usually have the same instructions.
//The code below is laid out like this, with some exceptions present.
//Thanks to the hardware being in my mind when coding this, the illegal instructions could be added fairly easily...
byte MOS_6510::CPU() //the CPU emulation for SID/PRG playback (ToDo: CIA/VIC-IRQ/NMI/RESET vectors, BCD-mode)
{ 
	//'IR' is the instruction-register, naming after the hardware-equivalent
	IR = Ram.memory[PC]; 
	cycles = 2; 
	storadd = 0; //'cycle': ensure smallest 6510 runtime (for implied/register instructions)
	
	if (IR & 1) {  //nybble2:  1/5/9/D:accu.instructions, 3/7/B/F:illegal opcodes
		switch (IR & 0x1F)
		{ //addressing modes (begin with more complex cases), PC wraparound not handled inside to save codespace
		case 1:
		case 3:
			PC++;
			addr = Ram.memory[Ram.memory[PC] + X] + Ram.memory[Ram.memory[PC] + X + 1] * 256;
			cycles = 6;
			break; //(zp,x)
		case 0x11: case 0x13: PC++; addr = Ram.memory[Ram.memory[PC]] + Ram.memory[Ram.memory[PC] + 1] * 256 + Y; cycles = 6; break; //(zp),y (5..6 cycles, 8 for R-M-W)
		case 0x19: case 0x1B: PC++; addr = Ram.memory[PC]; PC++; addr += Ram.memory[PC] * 256 + Y; cycles = 5; break; //abs,y //(4..5 cycles, 7 cycles for R-M-W)
		case 0x1D: PC++; addr = Ram.memory[PC]; PC++; addr += Ram.memory[PC] * 256 + X; cycles = 5; break; //abs,x //(4..5 cycles, 7 cycles for R-M-W)
		case 0xD: case 0xF: PC++; addr = Ram.memory[PC]; PC++; addr += Ram.memory[PC] * 256; cycles = 4; break; //abs
		case 0x15: PC++; addr = Ram.memory[PC] + X; cycles = 4; break; //zp,x
		case 5: case 7: PC++; addr = Ram.memory[PC]; cycles = 3; break; //zp
		case 0x17: PC++; if ((IR & 0xC0) != 0x80) { addr = Ram.memory[PC] + X; cycles = 4; } //zp,x for illegal opcodes
				 else { addr = Ram.memory[PC] + Y; cycles = 4; }  break; //zp,y for LAX/SAX illegal opcodes
		case 0x1F: PC++; if ((IR & 0xC0) != 0x80) { addr = Ram.memory[PC] + Ram.memory[++PC] * 256 + X; cycles = 5; } //abs,x for illegal opcodes
				 else { addr = Ram.memory[PC] + Ram.memory[++PC] * 256 + Y; cycles = 5; }  break; //abs,y for LAX/SAX illegal opcodes
		case 9: case 0xB: PC++; addr = PC; cycles = 2;  //immediate
		}
		addr &= 0xFFFF;
		switch (IR & 0xE0) {
		case 0x60: if ((IR & 0x1F) != 0xB) {
			if ((IR & 3) == 3) { T = (Ram.memory[addr] >> 1) + (ST & 1) * 128; ST &= 124; ST |= (T & 1); Ram.memory[addr] = T; cycles += 2; }   //ADC / RRA (ROR+ADC)
			T = A; A += Ram.memory[addr] + (ST & 1); ST &= 60; ST |= (A & 128) | (A > 255); A &= 0xFF; ST |= (!A) << 1 | (!((T ^ Ram.memory[addr]) & 0x80) & ((T ^ A) & 0x80)) >> 1;
		}
				 else {
			A &= Ram.memory[addr]; T += Ram.memory[addr] + (ST & 1); ST &= 60; ST |= (T > 255) | (!((A ^ Ram.memory[addr]) & 0x80) & ((T ^ A) & 0x80)) >> 1; //V-flag set by intermediate ADC mechanism: (A&mem)+mem
			T = A; A = (A >> 1) + (ST & 1) * 128; ST |= (A & 128) | (T > 127); ST |= (!A) << 1;
		}  break; // ARR (AND+ROR, bit0 not going to C, but C and bit7 get exchanged.)
		case 0xE0: if ((IR & 3) == 3 && (IR & 0x1F) != 0xB) { Ram.memory[addr]++; cycles += 2; }  T = A; A -= Ram.memory[addr] + !(ST & 1); //SBC / ISC(ISB)=INC+SBC
			ST &= 60; ST |= (A & 128) | (A >= 0); A &= 0xFF; ST |= (!A) << 1 | (((T ^ Ram.memory[addr]) & 0x80) & ((T ^ A) & 0x80)) >> 1; break;
		case 0xC0: if ((IR & 0x1F) != 0xB) { if ((IR & 3) == 3) { Ram.memory[addr]--; cycles += 2; }  T = A - Ram.memory[addr]; } // CMP / DCP(DEC+CMP)
				 else { X = T = (A & X) - Ram.memory[addr]; }   ST &= 124; ST |= (!(T & 0xFF)) << 1 | (T & 128) | (T >= 0);  break;  //SBX (AXS) (CMP+DEX at the same time)
		case 0x00: if ((IR & 0x1F) != 0xB) {
			if ((IR & 3) == 3) { ST &= 124; ST |= (Ram.memory[addr] > 127); Ram.memory[addr] <<= 1; cycles += 2; }
			A |= Ram.memory[addr]; ST &= 125; ST |= (!A) << 1 | (A & 128);
		} //ORA / SLO(ASO)=ASL+ORA
				 else { A &= Ram.memory[addr]; ST &= 124; ST |= (!A) << 1 | (A & 128) | (A > 127); }  break; //ANC (AND+Carry=bit7)
		case 0x20: if ((IR & 0x1F) != 0xB) {
			if ((IR & 3) == 3) { T = (Ram.memory[addr] << 1) + (ST & 1); ST &= 124; ST |= (T > 255); T &= 0xFF; Ram.memory[addr] = T; cycles += 2; }
			A &= Ram.memory[addr]; ST &= 125; ST |= (!A) << 1 | (A & 128);
		}  //AND / RLA (ROL+AND)
				 else { A &= Ram.memory[addr]; ST &= 124; ST |= (!A) << 1 | (A & 128) | (A > 127); }  break; //ANC (AND+Carry=bit7)
		case 0x40: if ((IR & 0x1F) != 0xB) {
			if ((IR & 3) == 3) { ST &= 124; ST |= (Ram.memory[addr] & 1); Ram.memory[addr] >>= 1; cycles += 2; }
			A ^= Ram.memory[addr]; ST &= 125; ST |= (!A) << 1 | (A & 128);
		} //EOR / SRE(LSE)=LSR+EOR
				 else { A &= Ram.memory[addr]; ST &= 124; ST |= (A & 1); A >>= 1; A &= 0xFF; ST |= (A & 128) | ((!A) << 1); }  break; //ALR(ASR)=(AND+LSR)
		case 0xA0: if ((IR & 0x1F) != 0x1B) { A = Ram.memory[addr]; if ((IR & 3) == 3) X = A; } //LDA / LAX (illegal, used by my 1 rasterline player) 
				 else { A = X = SP = Ram.memory[addr] & SP; }   ST &= 125; ST |= ((!A) << 1) | (A & 128); break;  // LAS (LAR)
		case 0x80: if ((IR & 0x1F) == 0xB) { A = X & Ram.memory[addr]; ST &= 125; ST |= (A & 128) | ((!A) << 1); } //XAA (TXA+AND), highly unstable on real 6502!
				 else if ((IR & 0x1F) == 0x1B) { SP = A & X; Ram.memory[addr] = SP & ((addr >> 8) + 1); } //TAS(SHS) (SP=A&X, mem=S&H} - unstable on real 6502
				 else { Ram.memory[addr] = A & (((IR & 3) == 3) ? X : 0xFF); storadd = addr; }  break; //STA / SAX (at times same as AHX/SHX/SHY) (illegal) 
		}
	}

	else if (IR & 2) {  //nybble2:  2:illegal/LDX, 6:A/X/INC/DEC, A:Accu-shift/reg.transfer/NOP, E:shift/X/INC/DEC
		switch (IR & 0x1F) { //addressing modes
		case 0x1E: PC++; addr = Ram.memory[PC]; PC++; addr += Ram.memory[PC] * 256 + (((IR & 0xC0) != 0x80) ? X : Y); cycles = 5; break; //abs,x / abs,y
		case 0xE: PC++; addr = Ram.memory[PC]; PC++; addr += Ram.memory[PC] * 256; cycles = 4; break; //abs
		case 0x16: PC++; addr = Ram.memory[PC] + (((IR & 0xC0) != 0x80) ? X : Y); cycles = 4; break; //zp,x / zp,y
		case 6: PC++; addr = Ram.memory[PC]; cycles = 3; break; //zp
		case 2: PC++; addr = PC; cycles = 2;  //imm.
		}
		addr &= 0xFFFF;
		switch (IR & 0xE0) {
		case 0x00: ST &= 0xFE; case 0x20: if ((IR & 0xF) == 0xA) { A = (A << 1) + (ST & 1); ST &= 124; ST |= (A & 128) | (A > 255); A &= 0xFF; ST |= (!A) << 1; } //ASL/ROL (Accu)
				 else { T = (Ram.memory[addr] << 1) + (ST & 1); ST &= 124; ST |= (T & 128) | (T > 255); T &= 0xFF; ST |= (!T) << 1; Ram.memory[addr] = T; cycles += 2; }  break; //RMW (Read-Write-Modify)
		case 0x40: ST &= 0xFE; case 0x60: if ((IR & 0xF) == 0xA) { T = A; A = (A >> 1) + (ST & 1) * 128; ST &= 124; ST |= (A & 128) | (T & 1); A &= 0xFF; ST |= (!A) << 1; } //LSR/ROR (Accu)
				 else { T = (Ram.memory[addr] >> 1) + (ST & 1) * 128; ST &= 124; ST |= (T & 128) | (Ram.memory[addr] & 1); T &= 0xFF; ST |= (!T) << 1; Ram.memory[addr] = T; cycles += 2; }  break; //memory (RMW)
		case 0xC0: if (IR & 4) { Ram.memory[addr]--; ST &= 125; ST |= (!Ram.memory[addr]) << 1 | (Ram.memory[addr] & 128); cycles += 2; } //DEC
				 else { X--; X &= 0xFF; ST &= 125; ST |= (!X) << 1 | (X & 128); }  break; //DEX
		case 0xA0: if ((IR & 0xF) != 0xA) X = Ram.memory[addr];  else if (IR & 0x10) { X = SP; break; }
				 else X = A;  ST &= 125; ST |= (!X) << 1 | (X & 128);  break; //LDX/TSX/TAX
		case 0x80: if (IR & 4) { Ram.memory[addr] = X; storadd = addr; }
				 else if (IR & 0x10) SP = X;  else { A = X; ST &= 125; ST |= (!A) << 1 | (A & 128); }  break; //STX/TXS/TXA
		case 0xE0: if (IR & 4) { Ram.memory[addr]++; ST &= 125; ST |= (!Ram.memory[addr]) << 1 | (Ram.memory[addr] & 128); cycles += 2; } //INC/NOP
		}
	}

	else if ((IR & 0xC) == 8) 
	{  //nybble2:  8:register/status
		switch (IR & 0xF0) 
		{
		case 0x60: 
			SP++; 
			SP &= 0xFF; 
			A = Ram.memory[0x100 + SP]; 
			ST &= 125; 
			ST |= (!A) << 1 | (A & 128); 
			cycles = 4; 
			break; //PLA
		case 0xC0: 
			Y++; Y &= 0xFF; 
			ST &= 125; ST |= (!Y) << 1 | (Y & 128); 
			break; //INY
		case 0xE0: 
			X++; 
			X &= 0xFF; 
			ST &= 125; 
			ST |= (!X) << 1 | (X & 128); 
			break; //INX
		case 0x80: 
			Y--; 
			Y &= 0xFF; 
			ST &= 125; 
			ST |= (!Y) << 1 | (Y & 128); 
			break; //DEY
		case 0x00: 
			Ram.memory[0x100 + SP] = ST; 
			SP--; 
			SP &= 0xFF; 
			cycles = 3; 
			break; //PHP
		case 0x20: 
			SP++; 
			SP &= 0xFF; 
			ST = Ram.memory[0x100 + SP]; 
			cycles = 4; 
			break; //PLP
		case 0x40: 
			Ram.memory[0x100 + SP] = A; 
			SP--; 
			SP &= 0xFF; 
			cycles = 3; 
			break; //PHA
		case 0x90: 
			A = Y; 
			ST &= 125; 
			ST |= (!A) << 1 | (A & 128); 
			break; //TYA
		case 0xA0: 
			Y = A; 
			ST &= 125; 
			ST |= (!Y) << 1 | (Y & 128); 
			break; //TAY
		default: 
			if (flagsw[IR >> 5] & 0x20) 
				ST |= (flagsw[IR >> 5] & 0xDF); 
			else 
				ST &= 255 - (flagsw[IR >> 5] & 0xDF);  //CLC/SEC/CLI/SEI/CLV/CLD/SED
		}
	}

	else {  //nybble2:  0: control/branch/Y/compare  4: Y/compare  C:Y/compare/JMP
		if ((IR & 0x1F) == 0x10) 
		{
			PC++; 
			T = Ram.memory[PC];
			
			if (T & 0x80) T -= 0x100; //BPL/BMI/BVC/BVS/BCC/BCS/BNE/BEQ  relative branch 
			
			if (IR & 0x20)
			{
				if (ST & branchflag[IR >> 6])
				{
					PC += T; cycles = 3;
				}
			}
			else 
			{ 
				if (!(ST & branchflag[IR >> 6])) 
				{ 
					PC += T; cycles = 3; 
				} 
			}
		}
		else {  //nybble2:  0:Y/control/Y/compare  4:Y/compare  C:Y/compare/JMP
			switch (IR & 0x1F) { //addressing modes
			case 0: PC++; addr = PC; cycles = 2; break; //imm. (or abs.low for JSR/BRK)
			case 0x1C: PC++; addr = Ram.memory[PC]; PC++; addr += Ram.memory[PC] * 256 + X; cycles = 5; break; //abs,x
			case 0xC: PC++; addr = Ram.memory[PC]; PC++; addr += Ram.memory[PC] * 256; cycles = 4; break; //abs
			case 0x14: PC++; addr = Ram.memory[PC] + X; cycles = 4; break; //zp,x
			case 4: PC++; addr = Ram.memory[PC]; cycles = 3;  //zp
			}
			addr &= 0xFFFF;
			switch (IR & 0xE0) {
			case 0x00: 
				Ram.memory[0x100 + SP] = PC % 256; 
				SP--; SP &= 0xFF; 
				Ram.memory[0x100 + SP] = PC / 256;  
				SP--; 
				SP &= 0xFF; 
				Ram.memory[0x100 + SP] = ST; 
				SP--; 
				SP &= 0xFF;
				PC = Ram.memory[0xFFFE] + Ram.memory[0xFFFF] * 256 - 1; 
				cycles = 7; 
				break; //BRK
			case 0x20: 
				if (IR & 0xF) 
				{ 
					ST &= 0x3D; 
					ST |= (Ram.memory[addr] & 0xC0) | (!(A & Ram.memory[addr])) << 1; 
				} //BIT 
				else 
				{ 
					Ram.memory[0x100 + SP] = (PC + 2) % 256; 
					SP--; SP &= 0xFF; 
					Ram.memory[0x100 + SP] = (PC + 2) / 256;  
					SP--; 
					SP &= 0xFF;
					PC = Ram.memory[addr] + Ram.memory[addr + 1] * 256 - 1; 
					cycles = 6; 
				}  
				break; //JSR
			case 0x40: 
				if (IR & 0xF) 
				{ 
					PC = addr - 1; 
					cycles = 3; 
				} //JMP
				else 
				{ 
					if (SP >= 0xFF) 
						return 0xFE; 
					
					SP++; 
					SP &= 0xFF; 
					ST = Ram.memory[0x100 + SP]; 
					SP++; SP &= 0xFF; 
					T = Ram.memory[0x100 + SP]; 
					SP++; SP &= 0xFF; 
					PC = Ram.memory[0x100 + SP] + T * 256 - 1;
					cycles = 6; 
				}  break; //RTI
			case 0x60: 
				if (IR & 0xF) 
				{ 
					PC = Ram.memory[addr] + Ram.memory[addr + 1] * 256 - 1; 
					cycles = 5; 
				} //JMP() (indirect)
				else 
				{ 
					if (SP >= 0xFF) 
						return 0xFF; 
					
					SP++; 
					SP &= 0xFF; 
					T = Ram.memory[0x100 + SP]; 
					SP++; 
					SP &= 0xFF; 
					PC = Ram.memory[0x100 + SP] + T * 256 - 1; 
					cycles = 6; 
				}  break; //RTS
			case 0xC0: 
				T = Y - Ram.memory[addr]; 
				ST &= 124; 
				ST |= (!(T & 0xFF)) << 1 | (T & 128) | (T >= 0); 
				break; //CPY
			case 0xE0: 
				T = X - Ram.memory[addr]; 
				ST &= 124; 
				ST |= (!(T & 0xFF)) << 1 | (T & 128) | (T >= 0); 
				break; //CPX
			case 0xA0: 
				Y = Ram.memory[addr]; 
				ST &= 125; ST |= (!Y) << 1 | (Y & 128); 
				break; //LDY
			case 0x80: 
				Ram.memory[addr] = Y; 
				storadd = addr;  //STY
			}
		}
	}

	PC++; //PC&=0xFFFF; 
	return 0;
}