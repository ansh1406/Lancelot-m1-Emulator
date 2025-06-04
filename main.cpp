#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <conio.h>

std::string programFileName;

using lancelot_word = unsigned short int;

class ProgramCounter
{
private:
    lancelot_word &bus;

public:
    lancelot_word programCount;

    ProgramCounter(lancelot_word &bus) : bus(bus)
    {
        programCount = 0;
    }
    void cnt_enb()
    {
        programCount++;
    }
    void cnt_out()
    {
        bus = programCount;
    }
    void jump()
    {
        programCount = bus;
    }
    void reset()
    {
        programCount = 0;
    }
};

class GeneralRegister
{
private:
    lancelot_word &bus;
    short int data;

public:
    GeneralRegister(lancelot_word &bus) : bus(bus)
    {
        data = 0;
    }

    void write()
    {
        data = bus;
    }
    void read()
    {
        bus = (lancelot_word)data;
    }
    void reset()
    {
        data = 0;
    }
    friend class ALU;
};

class FlagsRegister
{
public:
    bool equal, greater, lesser, zero;
    FlagsRegister()
    {
        equal = 0;
        greater = 0;
        lesser = 0;
        zero = 0;
    }
    void reset()
    {
        equal = 0;
        greater = 0;
        lesser = 0;
        zero = 0;
    }
};

class MemoryAddressRegister
{
private:
    lancelot_word address;
    lancelot_word &marAlwaysOut;
    lancelot_word &bus;

public:
    MemoryAddressRegister(lancelot_word &marAlwaysOut, lancelot_word &bus) : marAlwaysOut(marAlwaysOut), bus(bus)
    {
        address = 0;
    }
    void write()
    {
        address = bus;
        marAlwaysOut = address;
    }
    void read()
    {
        bus = address;
    }
    void reset()
    {
        address = 0;
        marAlwaysOut = 0;
    }
};

class InstructionRegister
{

private:
    lancelot_word &bus;

public:
    lancelot_word instruction;

    InstructionRegister(lancelot_word &bus) : bus(bus)
    {
        instruction = 0;
    }
    void write()
    {
        instruction = bus;
    }
    void readToUpper()
    {
        bus = (instruction & 0xFF00) >> 8;
    }
    void readToLower()
    {
        bus = instruction & 0x00FF;
    }
    void reset()
    {
        instruction = 0;
    }
};

class RAM
{
private:
    std::fstream *ramFile;
    lancelot_word &bus;
    lancelot_word &marAlwaysOut;

    const std::string ramTemplateFileName = "ram_template";
    const std::string ramFileName = "ram";

    void loadProgram(){
        std::ifstream programFile(programFileName,std::ios::binary);
        if (!programFile.is_open())
        {
            std::cerr << "Error opening program file!" << std::endl;
            return;
        }
        int currentAddress = 0;
        while(!programFile.eof()){
            char buffer[2];
            programFile.read(buffer, 2);
            std::string hexData;
            std::stringstream ss;
            ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (unsigned int)(unsigned char)buffer[1];
            ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (unsigned int)(unsigned char)buffer[0];
            hexData = ss.str();
            if (ramFile->is_open())
            {
                ramFile->seekp(calcCursorOffset(currentAddress), std::ios::beg);
                ramFile->write(hexData.c_str(), 4);
                currentAddress++;
            }
            else
            {
                std::cerr << "Error writing to RAM file!" << std::endl;
                return;
            }
        }
    }

    int calcCursorOffset(lancelot_word address)
    {
        unsigned int row = address / 16;
        unsigned int column = address % 16;
        unsigned int offset = row * 87 + 6 + column * 5;
        return offset;
    }

public:
    RAM(lancelot_word &bus, lancelot_word &marAlwaysOut) : bus(bus), marAlwaysOut(marAlwaysOut)
    {
        std::ifstream templateRamFile(ramTemplateFileName);
        std::ofstream ramFile(ramFileName);
        if (!ramFile.is_open())
        {
            std::cerr << "Error creating RAM file!" << std::endl;
        }
        else
        {

            templateRamFile.seekg(0, std::ios::beg);
            ramFile.seekp(0, std::ios::beg);
            ramFile << templateRamFile.rdbuf();
            ramFile.flush();
            ramFile.close();
        }
        this->ramFile = new std::fstream();
        this->ramFile->open(ramFileName, std::ios::in | std::ios::out);
        if (!this->ramFile->is_open())
        {
            std::cerr << "Error opening RAM file!" << std::endl;
        }
        loadProgram();
    }
    void read()
    {

        lancelot_word address = marAlwaysOut;
        ramFile->seekg(calcCursorOffset(address), std::ios::beg);
        char buffer[5] = {0};
        ramFile->read(buffer, 4);
        std::string data(buffer);
        bus = std::stoi(data, nullptr, 16);
    }

    void write()
    {
        ramFile->clear();
        lancelot_word address = marAlwaysOut;
        ramFile->seekp(calcCursorOffset(address), std::ios::beg);
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << bus;
        std::string hexData = ss.str();
        ramFile->write(hexData.c_str(), 4);
        ramFile->flush();
    }
    void reset()
    {
        ramFile->close();
        ramFile->open(ramFileName, std::ios::in | std::ios::out);
        if (!ramFile->is_open())
        {
            std::cerr << "Error opening RAM file!" << std::endl;
        }
    }
};

class Stack
{
private:
    int stackPointer;
    lancelot_word &bus;
    lancelot_word &marAlwaysOut;
    RAM &ram;

public:
    Stack(lancelot_word &bus, lancelot_word &marAlwaysOut, RAM &ram) : bus(bus), marAlwaysOut(marAlwaysOut), ram(ram)
    {
        stackPointer = 0;
    }
    void write()
    {
        lancelot_word temp = marAlwaysOut;
        marAlwaysOut = 0xffff - stackPointer;
        ram.write();
        marAlwaysOut = temp;
    }
    void read()
    {
        lancelot_word temp = marAlwaysOut;
        marAlwaysOut = 0xffff - stackPointer;
        ram.read();
        marAlwaysOut = temp;
    }
    void up()
    {
        stackPointer++;
    }
    void down()
    {
        stackPointer--;
    }
    void up_by()
    {
        stackPointer += bus;
    }
    void down_by()
    {
        stackPointer -= bus;
    }
    void stack_pointer_out()
    {
        bus = 0xffff - stackPointer;
    }
    void reset()
    {
        stackPointer = 0;
    }
};

class ALU
{
private:
    GeneralRegister &regA;
    GeneralRegister &regB;
    FlagsRegister &flags;
    short int result;
    lancelot_word &bus;

public:
    ALU(GeneralRegister &regA, GeneralRegister &regB, FlagsRegister &flags, lancelot_word &bus) : regA(regA), regB(regB), flags(flags), bus(bus)
    {
        result = 0;
    }
    void add()
    {
        result = regA.data + regB.data;
        flags.zero = (result == 0);
    }
    void sub()
    {
        result = regA.data - regB.data;
        flags.zero = (result == 0);
    }
    void mul()
    {
        result = regA.data * regB.data;
        flags.zero = (result == 0);
    }
    void div()
    {
        if (regB.data != 0)
        {
            result = regA.data / regB.data;
            flags.zero = (result == 0);
        }
        else
        {
            std::cerr << "Division by zero error!" << std::endl;
        }
    }
    void mod()
    {
        if (regB.data != 0)
        {
            result = regA.data % regB.data;
            if (result < 0)
                result = -result;
            flags.zero = (result == 0);
        }
        else
        {
            std::cerr << "Division by zero error!" << std::endl;
        }
    }
    void notOp()
    {
        result = ~regA.data;
        flags.zero = (result == 0);
    }
    void andOp()
    {
        result = regA.data & regB.data;
        flags.zero = (result == 0);
    }
    void orOp()
    {
        result = regA.data | regB.data;
        flags.zero = (result == 0);
    }
    void xorOp()
    {
        result = regA.data ^ regB.data;
        flags.zero = (result == 0);
    }
    void norOp()
    {
        result = ~(regA.data | regB.data);
        flags.zero = (result == 0);
    }
    void nandOp()
    {
        result = ~(regA.data & regB.data);
        flags.zero = (result == 0);
    }

    void compare()
    {
        if (regA.data == regB.data)
        {
            flags.equal = true;
            flags.greater = false;
            flags.lesser = false;
        }
        else if (regA.data > regB.data)
        {
            flags.equal = false;
            flags.greater = true;
            flags.lesser = false;
        }
        else
        {
            flags.equal = false;
            flags.greater = false;
            flags.lesser = true;
        }
    }
    void read()
    {
        bus = (lancelot_word)result;
    }
};

class TTY
{
private:
    std::fstream *ttyFile;
    lancelot_word &bus;

public:
    TTY(lancelot_word &bus) : bus(bus)

    {
        ttyFile = new std::fstream("tty.txt", std::ios::out | std::ios::trunc);
        if (!ttyFile->is_open())
        {
            std::cerr << "Error: Could not open tty.txt\n";
        }
        ttyFile->close();
        ttyFile = new std::fstream("tty.txt", std::ios::out | std::ios::app);
        if (!ttyFile->is_open())
        {
            std::cerr << "Error: Could not open tty.txt\n";
        }
    }
    void write()
    {
        char asciiChar = static_cast<char>(bus & 0x7F);
        if (asciiChar != 0)
        {
            *ttyFile << asciiChar;
            ttyFile->flush();
        }
    }
};

class Keyboard
{
private:
    std::fstream keyboardFile;
    int keyBoardPointer = 0;
    lancelot_word &bus;

public:
    Keyboard(lancelot_word &bus) : bus(bus)
    {
        keyboardFile.open("keyboard.txt", std::ios::in | std::ios::out | std::ios::app);
        if (!keyboardFile.is_open())
        {
            std::cerr << "Error opening keyboard file!" << std::endl;
        }
    }

    void read()
    {
        keyboardFile.clear();
        keyboardFile.seekg(keyBoardPointer, std::ios::beg);
        char ch;
        if (keyboardFile.peek() == EOF)
        {
            bus = 0;
            return;
        }
        keyboardFile.get(ch);
        bus = static_cast<lancelot_word>(ch);
    }

    void kb_reg_pop()
    {
        keyBoardPointer++;
    }
};

class ControlUnit
{
private:
    InstructionRegister &instructionReg;
    ProgramCounter &programCounter;
    MemoryAddressRegister &memoryAddressReg;
    FlagsRegister &flagsRegister;
    GeneralRegister &regA, &regB, &regC, &regD;
    RAM &ram;
    ALU alu;
    Stack &stack;
    TTY &tty;
    Keyboard &keyboard;
    bool &halted;
    lancelot_word &bus, &marAlwaysOut;

    void loadNextInstrucion()
    {
        programCounter.cnt_out();
        memoryAddressReg.write();
        ram.read();
        instructionReg.write();
        programCounter.cnt_enb();
    }

public:
    ControlUnit(InstructionRegister &instructionReg, ProgramCounter &programCounter,
                MemoryAddressRegister &memoryAddressReg, RAM &ram,
                Stack &stack, TTY &tty, Keyboard &keyboard, FlagsRegister &flagsRegister,
                GeneralRegister &regA, GeneralRegister &regB, GeneralRegister &regC, GeneralRegister &regD, bool &halted, lancelot_word &bus, lancelot_word &marAlwaysOut)
        : instructionReg(instructionReg), programCounter(programCounter),
          memoryAddressReg(memoryAddressReg), ram(ram),
          stack(stack), tty(tty), keyboard(keyboard), flagsRegister(flagsRegister),
          regA(regA), regB(regB), regC(regC), regD(regD),
          alu(regA, regB, flagsRegister, bus), halted(halted), bus(bus), marAlwaysOut(marAlwaysOut)
    {
    }

    void decode()
    {
        switch (instructionReg.instruction & 0xFF00)
        {
        case 0x0000:
            break;

        case 0x0100:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regA.write();
            programCounter.cnt_enb();
            break;

        case 0x0200:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regB.write();
            programCounter.cnt_enb();
            break;

        case 0x0300:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regC.write();
            programCounter.cnt_enb();
            break;

        case 0x0400:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regD.write();
            programCounter.cnt_enb();
            break;

        case 0x0500:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            tty.write();
            programCounter.cnt_enb();
            break;

        case 0x0600:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regA.write();
            programCounter.cnt_enb();
            regA.read();
            memoryAddressReg.write();
            ram.read();
            regA.write();
            break;

        case 0x0700:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regB.write();
            programCounter.cnt_enb();
            regB.read();
            memoryAddressReg.write();
            ram.read();
            regB.write();
            break;
        case 0x0800:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regC.write();
            programCounter.cnt_enb();
            regC.read();
            memoryAddressReg.write();
            ram.read();
            regC.write();
            break;

        case 0x0900:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regD.write();
            programCounter.cnt_enb();
            regD.read();
            memoryAddressReg.write();
            ram.read();
            regD.write();
            break;

        case 0x0A00:
            programCounter.cnt_out();
            memoryAddressReg.write();
            ram.read();
            regD.write();
            programCounter.cnt_enb();
            regD.read();
            memoryAddressReg.write();
            ram.read();
            tty.write();
            break;

        case 0x0B00:
            stack.read();
            regA.write();
            stack.down();
            break;

        case 0x0C00:
            stack.read();
            regB.write();
            stack.down();
            break;

        case 0x0D00:
            stack.read();
            regC.write();
            stack.down();
            break;

        case 0x0E00:
            stack.read();
            regD.write();
            stack.down();
            break;

        case 0x0F00:
            stack.read();
            tty.write();
            stack.down();
            break;

        case 0x1000:
            programCounter.cnt_out();
            memoryAddressReg.write();
            programCounter.cnt_enb();
            ram.read();
            regD.write();
            regD.read();
            memoryAddressReg.write();
            regA.read();
            ram.write();
            break;

        case 0x1100:
            programCounter.cnt_out();
            memoryAddressReg.write();
            programCounter.cnt_enb();
            ram.read();
            regD.write();
            regD.read();
            memoryAddressReg.write();
            regB.read();
            ram.write();
            break;

        case 0x1200:
            programCounter.cnt_out();
            memoryAddressReg.write();
            programCounter.cnt_enb();
            ram.read();
            regD.write();
            regD.read();
            memoryAddressReg.write();
            regC.read();
            ram.write();
            break;

        case 0x1300:
            programCounter.cnt_out();
            memoryAddressReg.write();
            programCounter.cnt_enb();
            ram.read();
            regB.write();
            regB.read();
            memoryAddressReg.write();
            regD.read();
            ram.write();
            break;

        case 0x1400:
            programCounter.cnt_out();
            memoryAddressReg.write();
            programCounter.cnt_enb();
            ram.read();
            regD.write();
            regD.read();
            memoryAddressReg.write();
            keyboard.read();
            ram.write();
            break;

        case 0x1500:
            stack.up();
            regA.read();
            stack.write();
            break;

        case 0x1600:
            stack.up();
            regB.read();
            stack.write();
            break;

        case 0x1700:
            stack.up();
            regC.read();
            stack.write();
            break;

        case 0x1800:
            stack.up();
            regD.read();
            stack.write();
            break;

        case 0x1900:
            stack.up();
            keyboard.read();
            keyboard.kb_reg_pop();
            stack.write();
            break;

        case 0x1A00:
            regB.read();
            regA.write();
            break;

        case 0x1B00:
            regC.read();
            regA.write();
            break;

        case 0x1C00:
            regD.read();
            regA.write();
            break;

        case 0x1D00:
            regA.read();
            regB.write();
            break;

        case 0x1E00:
            regC.read();
            regB.write();
            break;

        case 0x1F00:
            regD.read();
            regB.write();
            break;

        case 0x2000:
            regA.read();
            regC.write();
            break;

        case 0x2100:
            regB.read();
            regC.write();
            break;

        case 0x2200:
            regD.read();
            regC.write();
            break;

        case 0x2300:
            regA.read();
            regD.write();
            break;

        case 0x2400:
            regB.read();
            regD.write();
            break;

        case 0x2500:
            regC.read();
            regD.write();
            break;

        case 0x2600:
            keyboard.read();
            keyboard.kb_reg_pop();
            regA.write();
            break;

        case 0x2700:
            keyboard.read();
            keyboard.kb_reg_pop();
            regB.write();
            break;

        case 0x2800:
            keyboard.read();
            keyboard.kb_reg_pop();
            regC.write();
            break;

        case 0x2900:
            keyboard.read();
            keyboard.kb_reg_pop();
            regD.write();
            break;

        case 0x2A00:
            regA.read();
            tty.write();
            break;

        case 0x2B00:
            regB.read();
            tty.write();
            break;

        case 0x2C00:
            regC.read();
            tty.write();
            break;

        case 0x2D00:
            regD.read();
            tty.write();
            break;

        case 0x2E00:
            alu.add();
            alu.read();
            regC.write();
            break;

        case 0x2F00:
            alu.sub();
            alu.read();
            regC.write();
            break;

        case 0x3000:
            alu.mul();
            alu.read();
            regC.write();
            break;

        case 0x3100:
            alu.div();
            alu.read();
            regC.write();
            break;

        case 0x3200:
            alu.mod();
            alu.read();
            regC.write();
            break;

        case 0x3300:
            alu.notOp();
            alu.read();
            regC.write();
            break;

        case 0x3400:
            alu.andOp();
            alu.read();
            regC.write();
            break;

        case 0x3500:
            alu.orOp();
            alu.read();
            regC.write();
            break;

        case 0x3600:
            alu.xorOp();
            alu.read();
            regC.write();
            break;

        case 0x3700:
            alu.nandOp();
            alu.read();
            regC.write();
            break;

        case 0x3800:
            alu.norOp();
            alu.read();
            regC.write();
            break;

        case 0x3900:
            alu.compare();
            break;

        case 0x3A00:
            programCounter.cnt_out();
            stack.up();
            stack.write();
            break;

        case 0x3B00:
            stack.read();
            stack.down();
            programCounter.jump();
            break;

        case 0x3C00:
            programCounter.cnt_out();
            memoryAddressReg.write();
            programCounter.cnt_enb();
            ram.read();
            programCounter.jump();
            break;

        case 0x3D00:
            regA.read();
            programCounter.jump();
            break;

        case 0x3E00:
            regB.read();
            programCounter.jump();
            break;

        case 0x3F00:
            regC.read();
            programCounter.jump();
            break;

        case 0x4000:
            regD.read();
            programCounter.jump();
            break;

        case 0x4100:
            programCounter.cnt_out();
            regA.write();
            break;

        case 0x4200:
            regB.read();
            memoryAddressReg.write();
            ram.read();
            regA.write();
            break;

        case 0x4300:
            regC.read();
            memoryAddressReg.write();
            ram.read();
            regA.write();
            break;

        case 0x4400:
            regD.read();
            memoryAddressReg.write();
            ram.read();
            regA.write();
            break;

        case 0x4500:
            stack.up();
            break;

        case 0x4600:
            stack.down();
            break;

        case 0x4700:
            keyboard.kb_reg_pop();
            break;

        case 0x4800:
            regB.read();
            memoryAddressReg.write();
            regA.read();
            ram.write();
            break;

        case 0x4900:
            regC.read();
            memoryAddressReg.write();
            regA.read();
            ram.write();
            break;

        case 0x4A00:
            regD.read();
            memoryAddressReg.write();
            regA.read();
            ram.write();
            break;

        case 0x4B00:
            stack.stack_pointer_out();
            regA.write();
            break;

        case 0x4C00:
            regD.read();
            stack.up_by();
            break;

        case 0x4D00:
            regD.read();
            stack.down_by();
            break;

        case 0xFB00:
            if (flagsRegister.greater)
            {
                programCounter.cnt_out();
                memoryAddressReg.write();
                programCounter.cnt_enb();
                ram.read();
                programCounter.jump();
            }
            else
            {
                programCounter.cnt_enb();
            }
            break;

        case 0xFC00:
            if (flagsRegister.equal)
            {
                programCounter.cnt_out();
                memoryAddressReg.write();
                programCounter.cnt_enb();
                ram.read();
                programCounter.jump();
            }
            else
            {
                programCounter.cnt_enb();
            }
            break;

        case 0xFD00:
            if (flagsRegister.lesser)
            {
                programCounter.cnt_out();
                memoryAddressReg.write();
                programCounter.cnt_enb();
                ram.read();
                programCounter.jump();
            }
            else
            {
                programCounter.cnt_enb();
            }
            break;

        case 0xFF00:
            halted = true;
            break;
        }
        loadNextInstrucion();
    }
};

class CPU
{
private:
    ControlUnit controlUnit;
    InstructionRegister instructionReg;
    ProgramCounter programCounter;
    MemoryAddressRegister memoryAddressReg;
    FlagsRegister flagsRegister;
    GeneralRegister regA, regB, regC, regD;
    RAM ram;
    ALU alu;
    Stack stack;
    TTY tty;
    Keyboard keyboard;
    lancelot_word bus;
    lancelot_word marAlwaysOut;
    bool halted = false;

    void reset()
    {
        instructionReg.reset();
        programCounter.reset();
        memoryAddressReg.reset();
        flagsRegister.reset();
        regA.reset();
        regB.reset();
        regC.reset();
        regD.reset();
        ram.reset();
        stack.reset();
    }

    void execute()
    {
        controlUnit.decode();
    }

    void halt()
    {
        std::cout << "\nCPU halted. Exiting...\n";
        exit(0);
    }

public:
    CPU() : controlUnit(instructionReg, programCounter, memoryAddressReg, ram, stack, tty, keyboard, flagsRegister, regA, regB, regC, regD, halted, bus, marAlwaysOut),
            instructionReg(bus), programCounter(bus), memoryAddressReg(marAlwaysOut, bus), flagsRegister(), regA(bus), regB(bus), regC(bus), regD(bus),
            ram(bus, marAlwaysOut), alu(regA, regB, flagsRegister, bus), stack(bus, marAlwaysOut, ram), tty(bus), keyboard(bus)
    {
        std::cout << "\nCPU initialized.\n";
    }
    void start()
    {
        reset();
        while (!halted)
        {
            execute();
        }
        halt();
    }
};

int main(int argc , char *argv[])
{
    if(argc < 2){
        std::cerr << "No program file specified. Usage: " << argv[0] << " <program_file>\n";
        return 1;
    }
    programFileName = argv[1]; 
    CPU lancelot_m1;
    lancelot_m1.start();
    return 0;
}
