
//  main.c
//  MipsSimulator
//
//  Created by 吴未名 on 15/3/28.
//  Copyright (c) 2015年 wuwm. All rights reserved.
//
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

char memory[1024];
char dmemory[1024];
int reg[32];
int opcode,rs,rt,rd,shamt,func,immediate;
unsigned int pcAddress,numOfinstruction,numOfdata;
unsigned int cycleNum=0;

void initImemory()
{
    int i;
    unsigned char buf[4];
    cycleNum=0;
    FILE *fp;
    fp=fopen("iimage.bin", "r");
    for(i=0;i<1024;memory[i++]=0);
    for(i=0;i<32;reg[i++]=0);
    fread((void *)buf, (size_t)1, (size_t)4, fp);
    pcAddress = (buf[0] << 24) |   (buf[1] << 16) |  (buf[2] << 8) | buf[3];
    fread(&buf, (size_t)1, (size_t)4, fp);
    numOfinstruction = (buf[0] << 24) |   (buf[1] << 16) |  (buf[2] << 8) | buf[3];
    fread((void *)(memory+pcAddress), (size_t)1, (size_t)numOfinstruction*4, fp);
    fclose(fp);
    fp=fopen("dimage.bin", "r");
    for(i=0;i<1024;dmemory[i++]=0);
    fread((void *)buf, (size_t)1, (size_t)4, fp);
    reg[29] = (buf[0] << 24) |   (buf[1] << 16) |  (buf[2] << 8) | buf[3];
    fread(&buf, (size_t)1, (size_t)4, fp);
    numOfdata = (buf[0] << 24) |   (buf[1] << 16) |  (buf[2] << 8) | buf[3];
    fread((void *)dmemory, (size_t)1, (size_t)numOfdata*4, fp);
    fclose(fp);
    
    
}

void printMemOverflow(unsigned int cycle)
{
    FILE *fp;
    fp=fopen("/Users/Wu/error_dump.rpt", "a");
    fprintf(fp, "In cycle %d:Address Overflow\n",cycle+1);
    fclose(fp);
    exit(1);
}

void print0error(unsigned int cycle)
{
    FILE *fp;
    fp=fopen("/Users/Wu/error_dump.rpt", "a");
    fprintf(fp, "In cycle %d:Write $0 Error\n",cycle+1);
    fclose(fp);
}

void printMisAligned(unsigned int cycle)
{
    FILE *fp;
    fp=fopen("/Users/Wu/error_dump.rpt", "a");
    fprintf(fp, "In cycle %d:Misalignment Error\n",cycle+1);
    fclose(fp);
    exit(1);
}
unsigned int getInstruction(int address)
{
    /*
    if(address<0||address>1020)
    {
        printMemOverflow(cycleNum);
    }*/
    return (memory[address]<<24)+((memory[address+1]<<16) & 0xff0000)+((memory[address+2]<<8) & 0xff00)+(memory[address+3] & 0xff);
}

void putWord(int address,unsigned int value)
{
    if(address<0||address>1020)
    {
        printMemOverflow(cycleNum);
    }
    if(address%4!=0){
        printMisAligned(cycleNum);
    }
    dmemory[address]=(char)(value>>24);//最高位
    dmemory[address+1]=(char)((value & 0xff0000)>>16);
    dmemory[address+2]=(char)((value & 0xff00)>>8);
    dmemory[address+3]=(char)(value & 0xff);
    
}


unsigned int getWord(int address)
{
    if(address<0||address>1020)
    {
        printMemOverflow(cycleNum);
    }
    if(address%4!=0){
        printMisAligned(cycleNum);
    }
    return (dmemory[address]<<24)+((dmemory[address+1]<<16) & 0xff0000)+((dmemory[address+2]<<8) & 0xff00)+(dmemory[address+3] & 0xff);
}

void put2Byte(int address,int value)
{
    if(address<0||address>1022)
    {
        printMemOverflow(cycleNum);
    }
    if(address%2!=0){
        printMisAligned(cycleNum);
    }
    dmemory[address]=(char)((value<<16)>>24);//最高位
    dmemory[address+1]=(char)((value<<24)>>24);
    
}

int get2Byte(int address)
{
    if(address<0||address>1022)
    {
        printMemOverflow(cycleNum);
    }
    if(address%2!=0){
        printMisAligned(cycleNum);
    }
    int rv=((dmemory[address]<<8) & 0xff00)+(dmemory[address+1] & 0xff);
    return (rv<<16)>>16;
}

int get2Byteu(int address)
{
    if(address<0||address>1022)
    {
        printMemOverflow(cycleNum);
    }
    if(address%2!=0){
        printMisAligned(cycleNum);
    }
    return ((dmemory[address]<<8) & 0xff00)+(dmemory[address+1] & 0xff) & 0x0000ffff;
}


void putByte( int address, int value )
{
    if ((address < 0) || (address > 1023))
    {
        printMemOverflow(cycleNum);
    }
    dmemory[address] = (char)(value);
}

int getByte (int address)
{
    int n ;
    if ((address < 0) || (address > 1023))
    {
        printMemOverflow(cycleNum);
    }
    
    n = dmemory[address] ;//有符号
    return (n<<24)>>24 ;
}

unsigned int getByteu (int address)
{
    unsigned int n ;
    if ((address < 0) || (address > 1023))
    {
        printMemOverflow(cycleNum);
    }
    
    n = (unsigned int)dmemory[address] &0x000000ff;//无符号
    return n ;
}

void regSet(int address,int value)
{
    if(address==0){
        print0error(cycleNum);
        reg[0]=0;
        return;
    }
    reg[address]=value;
}

void classify(int ins)
{
    opcode=(ins>>26 & 0x3f);//将数据存入段间寄存器
    rs=(ins>>21 & 0x1f);//5bit
    rt=(ins>>16 & 0x1f);//5bit
    rd=(ins>>11 & 0x1f);//5bit
    shamt=(ins>>6 & 0x1f);//5bit
    func=(ins & 0x3f);//6bit
    immediate=(ins<<16)>>16;//16bit
}

void exec()
{
    int instruction;
    instruction=getInstruction(pcAddress);
    pcAddress+=4;
    classify(instruction);
    switch (opcode) {
        case 0://R-Format
            switch (func) {
                case 0x20:
                    regSet(rd, reg[rs]+reg[rt]);
                    break;
                case 0x22:
                    regSet(rd, reg[rs]-reg[rd]);
                    break;
                case 0x24:
                    regSet(rd, reg[rs] & reg[rt]);
                    break;
                case 0x25:
                    regSet(rd, reg[rs]|reg[rt]);
                    break;
                case 0x26:
                    regSet(rd, reg[rs]^reg[rt]);
                case 0x27:
                    regSet(rd, ~(reg[rs]|reg[rt]));
                    break;
                case 0x28:
                    regSet(rd, ~(reg[rs]&reg[rt]));
                    break;
                case 0x2A:
                    if(reg[rs]<reg[rt]){
                        regSet(rd, 1);
                    }else{
                        regSet(rd, 0);
                    }
                    break;
                case 0x00:
                    regSet(rd, reg[rt]<<shamt);
                    break;
                case 0x02:
                    regSet(rd, (unsigned int)reg[rt]>>shamt);
                    break;
                case 0x03:
                    regSet(rd, reg[rt]>>shamt);
                    break;
                case 0x08:
                    pcAddress=reg[rs];
                    break;
                default:
                    break;
            }
            break;
        case 0x08:
            regSet(rt, reg[rs]+immediate);
            break;
        case 0x23:
            regSet(rt, getWord(reg[rs]+immediate));
            break;
        case 0x21:
            regSet(rt, get2Byte(reg[rs]+immediate));
            break;
        case 0x25:
            //lhu
            regSet(rt, get2Byteu(reg[rs]+immediate));
            break;
        case 0x20:
            regSet(rt, getByte(reg[rs]+immediate));
            break;
        case 0x24:
            //lbu
            regSet(rt, getByteu(reg[rs]+immediate));
            break;
        case 0x2B:
            putWord(reg[rs]+immediate, reg[rt]);
            break;
        case 0x29:
            put2Byte(reg[rs]+immediate, reg[rt]);
            break;
        case 0x28:
            putByte(reg[rs]+immediate, reg[rt]);
            break;
        case 0x0F:
            regSet(rt, immediate<<16);
            break;
        case 0x0C:
            regSet(rt, reg[rs] & (unsigned int)immediate);
            break;
        case 0x0D:
            regSet(rt, reg[rs] | (unsigned int)immediate);
            break;
        case 0x0E:
            regSet(rt, ~(reg[rs]|(unsigned int)immediate));
            break;
        case 0x0A:
            if (reg[rs]<immediate) {
                regSet(rt, 1);
            }else{
                regSet(rt, 0);
            }
            break;
        case 0x04:
            if(reg[rs]==reg[rt]){
                pcAddress+=4*immediate;
            }
            break;
        case 0x05:
            if(reg[rs]!=reg[rt]){
                pcAddress+=4*immediate;
            }
            break;
        case 0x02:
            pcAddress=((pcAddress) & 0xf0000000) | (immediate*4);
            break;
        case 0x03:
            regSet(31, pcAddress);
            pcAddress=((pcAddress) & 0xf0000000) | (immediate*4);
            break;
        case 0x3F:
            exit(1);
        default:
            break;
    }
    cycleNum++;

}

void printReg()
{
    int i;
    FILE *fp;
    fp=fopen("/Users/Wu/snapshot.rpt", "a");
    fprintf(fp, "%s%d\n","cycle ",cycleNum);
    
    for (i=0; i<32; ++i) {
        fprintf(fp, "%s%02d%s%08X\n","$",i,": 0x",reg[i]);
    }
    fprintf(fp, "%s%08X\n\n\n","PC: 0x",pcAddress);
    fclose(fp);
}

int main(int argc, const char * argv[])
{
    initImemory();
    while (1) {
        printReg();
        exec();
    }

}
