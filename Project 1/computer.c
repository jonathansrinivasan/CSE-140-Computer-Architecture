//Partner: Franz Anthony Varela
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "computer.h"
#undef mips			/* gcc already has a def for mips */

unsigned int endianSwap(unsigned int);

void PrintInfo (int changedReg, int changedMem);
unsigned int Fetch (int);
void Decode (unsigned int, DecodedInstr*, RegVals*);
int Execute (DecodedInstr*, RegVals*);
int Mem(DecodedInstr*, int, int *);
void RegWrite(DecodedInstr*, int, int *);
void UpdatePC(DecodedInstr*, int);
void PrintInstruction (DecodedInstr*);

/*Globally accessible Computer variable*/
Computer mips;
RegVals rVals;
int originPC;

char functs[45][8] = {

  //shamt-using functions
  "sll", //0
  "", //1
  "srl", //2
  "sra", //3

  //shift-registers
  "sllv", //4
  "", //5
  "srlv", //6
  "srav", //7

  //jumps
  "jr", //8
  "jalr", //9 (SPECIAL CASE TOO)

  "", //10
  "", //11

  //syscall
  "syscall", //12

  "", //13
  "", //14
  "", //15

  //mult-register
  "mfhi", //16
  "mthi", //17
  "mflo", //18
  "mtlo", //19

  "", //20
  "", //21
  "", //22
  "", //23

  //multiply
  "mult", //24
  "multu", //25

  //divide
  "div", //26
  "divu", //27

  "", //28
  "", //29
  "", //30
  "", //31

  //add
  "add", //32
  "addu", //33

  //sub
  "sub", //34
  "subu", //35

  //bitwise
  "and", //36
  "or", //37
  "xor", //38
  "nor", //39

  "", //40
  "", //41

  //slt
  "slt", //42
  "sltu" //43

};

char ops[45][8] = {

  //shamt-using functions
  "", //0
  "", //1
  "j", //2
  "jal", //3

  //shift-registers
  "beq", //4
  "bne", //5
  "blez", //6
  "bgtz", //7

  //jumps
  "addi", //8
  "addiu", //9 (SPECIAL CASE TOO)

  "slti", //10
  "sltiu", //11

  //syscall
  "andi", //12

  "ori", //13
  "xori", //14
  "lui", //15

  //mult-register
  "", //16
  "", //17
  "", //18
  "", //19

  "", //20
  "", //21
  "", //22
  "", //23

  //multiply
  "", //24
  "", //25

  //divide
  "", //26
  "", //27

  "", //28
  "", //29
  "", //30
  "", //31

  //add
  "lb", //32
  "lh", //33

  //sub
  "lw", //34
  "", //35

  //bitwise
  "lbu", //36
  "lhu", //37
  "", //38
  "", //39

  "sb", //40
  "sh", //41

  //slt
  "", //42
  "sw" //43

};

/*
 *  Return an initialized computer with the stack pointer set to the
 *  address of the end of data memory, the remaining registers initialized
 *  to zero, and the instructions read from the given file.
 *  The other arguments govern how the program interacts with the user.
 */
void InitComputer (FILE* filein, int printingRegisters, int printingMemory,
  int debugging, int interactive) {
    int k;
    unsigned int instr;
    originPC = 0;

    /* Initialize registers and memory */

    for (k=0; k<32; k++) { // there's registers $0 - $31 in mips
        mips.registers[k] = 0;
    }

    /* stack pointer - Initialize to highest address of data segment */
    mips.registers[29] = 0x00400000 + (MAXNUMINSTRS+MAXNUMDATA)*4;

    for (k=0; k<MAXNUMINSTRS+MAXNUMDATA; k++) {
        mips.memory[k] = 0;
    }

    k = 0;
    while (fread(&instr, 4, 1, filein)) {
	/*swap to big endian, convert to host byte order. Ignore this.*/
        mips.memory[k] = ntohl(endianSwap(instr));
        k++;
        if (k>MAXNUMINSTRS) {
            fprintf (stderr, "Program too big.\n");
            exit (1);
        }
    }

    mips.printingRegisters = printingRegisters;
    mips.printingMemory = printingMemory;
    mips.interactive = interactive;
    mips.debugging = debugging;
}

unsigned int endianSwap(unsigned int i) {
    return (i>>24)|(i>>8&0x0000ff00)|(i<<8&0x00ff0000)|(i<<24);
}

/*
 *  Run the simulation.
 */
void Simulate () {
    char s[40];  /* used for handling interactive input */
    unsigned int instr;
    int changedReg=-1, changedMem=-1, val;
    DecodedInstr d;

    /* Initialize the PC to the start of the code section */
    mips.pc = 0x00400000;
    while (1) { // this is the while loop that continually executes instructions
        if (mips.interactive) {
            printf ("> ");
            fgets (s,sizeof(s),stdin);
            if (s[0] == 'q') {
                return;
            }
        }

        /* Fetch instr at mips.pc, returning it in instr */
        instr = Fetch (mips.pc); // the instruction in hex will be stored here

        printf ("Executing instruction at %8.8x: %8.8x\n", mips.pc, instr);

        /*
	 * Decode instr, putting decoded instr in d
	 * Note that we reuse the d struct for each instruction.
	 */
        Decode (instr, &d, &rVals);

        /*Print decoded instruction*/
        PrintInstruction(&d);

        /*
	 * Perform computation needed to execute d, returning computed value
	 * in val
	 */
        val = Execute(&d, &rVals); // this value will be used for every other instruction later

	UpdatePC(&d,val);

        /*
	 * Perform memory load or store. Place the
	 * address of any updated memory in *changedMem,
	 * otherwise put -1 in *changedMem.
	 * Return any memory value that is read, otherwise return -1.
         */
        val = Mem(&d, val, &changedMem);

        /*
	 * Write back to register. If the instruction modified a register--
	 * (including jal, which modifies $ra) --
         * put the index of the modified register in *changedReg,
         * otherwise put -1 in *changedReg.
         */
        RegWrite(&d, val, &changedReg);

        PrintInfo (changedReg, changedMem);
    }
}

/*
 *  Print relevant information about the state of the computer.
 *  changedReg is the index of the register changed by the instruction
 *  being simulated, otherwise -1.
 *  changedMem is the address of the memory location changed by the
 *  simulated instruction, otherwise -1.
 *  Previously initialized flags indicate whether to print all the
 *  registers or just the one that changed, and whether to print
 *  all the nonzero memory or just the memory location that changed.
 */
void PrintInfo ( int changedReg, int changedMem) {
    int k, addr;
    printf ("New pc = %8.8x\n", mips.pc);
    if (!mips.printingRegisters && changedReg == -1) {
        printf ("No register was updated.\n");
    } else if (!mips.printingRegisters) {
        printf ("Updated r%2.2d to %8.8x\n",
        changedReg, mips.registers[changedReg]);
    } else {
        for (k=0; k<32; k++) {
            printf ("r%2.2d: %8.8x  ", k, mips.registers[k]);
            if ((k+1)%4 == 0) {
                printf ("\n");
            }
        }
    }
    if (!mips.printingMemory && changedMem == -1) {
        printf ("No memory location was updated.\n");
    } else if (!mips.printingMemory) {
        printf ("Updated memory at address %8.8x to %8.8x\n",
        changedMem, Fetch (changedMem));
    } else {
        printf ("Nonzero memory\n");
        printf ("ADDR	  CONTENTS\n");
        for (addr = 0x00400000+4*MAXNUMINSTRS;
             addr < 0x00400000+4*(MAXNUMINSTRS+MAXNUMDATA);
             addr = addr+4) {
            if (Fetch (addr) != 0) {
                printf ("%8.8x  %8.8x\n", addr, Fetch (addr));
            }
        }
    }
}

/*
 *  Return the contents of memory at the given address. Simulates
 *  instruction fetch.
 */
unsigned int Fetch ( int addr) {
    return mips.memory[(addr-0x00400000)/4];
}

/* NEEDS TO SUPPORT:
  addi
  addiu
  subu
  sll
  srl
  and
  andi
  or
  ori
  lui
  slt
  beq
  bne
  j
  jal
  jr
  lw
  sw
*/

//remember, we also have registers[32], pc counter, and memory[] arrays

/* Decode instr, returning decoded instruction. */
//at this point, the instr is some hex value
void Decode ( unsigned int instr, DecodedInstr* d, RegVals* rVals) {
    /* Your code goes here */
    /* so instr will just be a number. we can find the opcode by shifting right*/
    //printf("OP: 0x%X\n", instr >> 26);
    int rs = (instr >> 21) & 0x1F;
    //if (rs < 0) exit(0);
    int rt = (instr >> 16) & 0x1F;
    //if (rt < 0) exit(0);
    int rd = (instr >> 11) & 0x1F;
    int shamt = (instr >> 6) & 0x1F; // for r-format
    int funct = instr & 0x3F; // for r-format
    int im = instr & 0xFFFF; //immediate
    int jAddr = instr << 6;
    jAddr = jAddr >> 4;

    //int tempSLL, tempSRL;
    d->op = instr >> 26; // get opcode
    //fill in instruction type
    if (d->op == 0) { // R-Format
      //  op     rs    rt    rd   shamt  func
      // xxxxxx xxxxx xxxxx xxxxx xxxxx xxxxxx
      //check for errors first
      if (funct != 0 &&
          funct != 2 &&
          funct != 8 &&
          funct != 33 &&
          funct != 35 &&
          funct != 36 &&
          funct != 37 &&
          funct != 42) exit(0); // if it's not a supported op, then dont exit


      d->type = 0; // type is R (enum)
      d->regs.r.rs = rs;
      d->regs.r.rt = rt;
      d->regs.r.rd = rd;
      d->regs.r.shamt = shamt;
      d->regs.r.funct = funct;

      //getting the register values
      rVals->R_rs = mips.registers[rs];
      rVals->R_rt = mips.registers[rt];
      rVals->R_rd = mips.registers[rd];
    }
    else if (d->op <= 3) { // J-Format
      //  op              addr
      // xxxxxx xx/xxxx/xxxx/xxxx/xxxx/xxxx/xxxx
      d->type = 2;
      d->regs.j.target = jAddr;
    } // any instruction not 0 and <= 3 are jumps
    else { // I-Format
      //  op     rs    rt    immediate
      // xxxxxx xxxxx xxxxx xxxxxxxxxxxxxxxx
      int op = d->op;
      if (op != 4 &&
          op != 5 &&
          op != 9 &&
          op != 12 &&
          op != 13 &&
          op != 15 &&
          op != 34 &&
          op != 43) exit(0); // if it's not a supported op, then dont exit

      d->type = 1; // type is I
      d->regs.i.rs = rs; // rs
      d->regs.i.rt = rt; // rt
      d->regs.i.addr_or_immed = im; // im
      if ((d->op != 0xC && d->op != 0xD) && im >> 15 == 1) {
        d->regs.i.addr_or_immed += 0xFFFF0000;
      }
      //getting the register values
      rVals->R_rs = mips.registers[rs];
      rVals->R_rt = mips.registers[rt];
    }// any instruction above 3 is immediate
}

/*
 *  Print the disassembled version of the given instruction
 *  followed by a newline.
 */
void PrintInstruction ( DecodedInstr* d) {
    /* Your code goes here */
    int op, rs, rt, rd, shamt, funct, im, jAddr;
    if (d->op == 0) { // R-Format
      op = 0x0;
      rs = d->regs.r.rs;
      rt = d->regs.r.rt;
      rd = d->regs.r.rd;
      shamt = d->regs.r.shamt;
      funct = d->regs.r.funct;

      if (funct < 3) { //sll, srl
        printf("%s\t$%d, $%d, %d\n", functs[funct], rd, rt, shamt);
      } else if (funct != 8){ // everything else
        printf("%s\t$%d, $%d, %d\n", functs[funct], rd, rs, rt);
      } else { // jr
        printf("%s\t$%d\n", functs[funct], rs);
      }

    } else if (d->op <= 3) { // J-Format
      op = d->op; // j, jal
      jAddr = d->regs.j.target;
      // op target
      printf("%s\t0x%8.8x\n", ops[op], jAddr);
    } else { // I-Format
      op = d->op;
      rs = d->regs.i.rs;
      rt = d->regs.i.rt;
      im = d->regs.i.addr_or_immed;

      if (op <=5) { // beq, bne
        // op rs, rt, im
        printf("%s\t$%d, $%d, 0x%8.8x\n", ops[op], rs, rt, ((im + 1) * 4) + mips.pc);
      } else if (op == 9) { // addi
        printf("%s\t$%d, $%d, %d\n", ops[op], rt, rs, im);
      } else if (op <= 13) { // andi, ori
        // op rt, rs, im
        printf("%s\t$%d, $%d, 0x%x\n", ops[op], rt, rs, im);
      } else if (op == 15) { //lui
        // op rt, im
        printf("%s\t$%d, 0x%x\n", ops[op], rt, im);
      } else { // lw, sw
        // op rt, im(rs)
        printf("%s\t$%d, %d($%d)", ops[op], rt, im, rs);
      }
    }
}

/* Perform computation needed to execute d, returning computed value */
int Execute ( DecodedInstr* d, RegVals* rVals) {
    /* Your code goes here */
    int op = d->op;
    int rs = rVals->R_rs;
    int rt = rVals->R_rt;
    int jAddr = d->regs.j.target;
    if (op == 0) { // R-format
      int funct = d->regs.r.funct;
      switch (funct) {
        case 0x0: {return rs << rt;} //break;//sll
        case 0x2: {return rs >> rt;} // break;//srl
        case 0x8: {return rs;} //break;//jr
        case 0x21: {return rs + rt;} //break;//addu
        case 0x23: {return rs - rt;}  //break;//subu
        case 0x24: {return rs & rt;}  //break;//and
        case 0x25: {return rs | rt;} //break;//or
        case 0x2A: {return (rs < rt) ? 1: 0;}  //break;//slt
      }
    } else { //I & J format
      int im = d->regs.i.addr_or_immed;
      switch (op) {
        case 0x2: {return jAddr;} //break;//j
        case 0x3: {return jAddr;} //break;//jal
        case 0x4: {if (rs == rt) {return mips.pc + ((im + 1) * 4);} else return mips.pc+4;}  //break;//beq
        case 0x5: {if (rs != rt) return mips.pc + ((im + 1) * 4); else return mips.pc+4;}  //break;//bne
        case 0x9: {return rs + im;}  //break;//addiu
        case 0xC: {return rs & im;}  //break;//andi
        case 0xF: {return im << 16;}  //break;// lui
        case 0x23: {return rs + im;}  //break;// lw
        case 0x2B: {return rs + im;}  //break;// sw
      }
    }
    return 0;
}

/*
 * Update the program counter based on the current instruction. For
 * instructions other than branches and jumps, for example, the PC
 * increments by 4 (which we have provided).
 */
void UpdatePC ( DecodedInstr* d, int val) {
    mips.pc+=4;
    /* Your code goes here */
    //the only instructions that update PC are beq, bne, jump, jal, and jr
    int op = d->op;
    if (op == 0) {
      if (d->regs.r.funct == 0x8) mips.pc = val; // jr
    } else {
      if (op <= 0x5) {
        //printf("MERP\n");
        originPC = mips.pc;
        mips.pc = val;
      } //bne, beq, jump, jal
    }
}

/*
 * Perform memory load or store. Place the address of any updated memory
 * in *changedMem, otherwise put -1 in *changedMem. Return any memory value
 * that is read, otherwise return -1.
 *
 * Remember that we're mapping MIPS addresses to indices in the mips.memory
 * array. mips.memory[0] corresponds with address 0x00400000, mips.memory[1]
 * with address 0x00400004, and so forth.
 *
 */
int Mem( DecodedInstr* d, int val, int *changedMem) {
    /* Your code goes here */
    //the only instructions that access memory are sw and lw
    int op = d->op;
    if (op == 0x23 || op == 0x2b) { //sw and lw
      if (val * 4 + 0x00400000 <= 0x004010000 || val * 4 + 0x00400000 >= 0x00403FFF) { // check if youre outside the memory bounds
        printf("Memory Access Exception at 0x%8.8X, address 0x%8.8X\n", mips.pc, val);
        exit(0);
      }
      *changedMem = -1; // just put this here first in case you dont change anything
      if (op == 0x23) return mips.memory[(val-0x00400000) / 4]; //lw
    } else if (op == 0x2B) { //sw
      *changedMem = val;
      mips.memory[(val-0x00400000) / 4] = rVals.R_rt;
      return -1; // you only return any memory value that is read, otherwise -1
    }
  return val; // otherwise, your value is going to be used for the WB
}

/*
 * Write back to register. If the instruction modified a register--
 * (including jal, which modifies $ra) --
 * put the index of the modified register in *changedReg,
 * otherwise put -1 in *changedReg.
 */
void RegWrite( DecodedInstr* d, int val, int *changedReg) {
    /* Your code goes here */
    int type = d->type;
    if (type == R) { // R-Format
      //the only instructions that wouldn't write back are jr
      int funct = d->regs.r.funct;
      if (funct != 0x8) {
        int rd = d->regs.r.rd;
        mips.registers[rd] = val;
        *changedReg = rd;
      } else *changedReg = -1;
    } else if (type == I) { // I-Format
      int op = d->op;
      // the only three instructions that dont write back are beq, bne, and sw
      int rt = d->regs.i.rt;
      if (op != 0x2B && op != 0x4 && op != 0x5) {
        mips.registers[rt] = val;
        *changedReg = d->regs.i.rt;
      } else *changedReg = -1;
    } else { // J-Format
      if (d->op == 0x3) {
        mips.registers[31] = originPC;
        *changedReg = 31;
      } else *changedReg = -1;
    }
}
