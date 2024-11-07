// On my honor, I have neither given nor received any unauthorized aid on this assignment.

// disassembly output file name.
#define OUTPUTDISASSEMBLY "disassembly.txt"

// simulation output file name.
#define OUTPUTSIMULATION "simulation.txt"


#include <iostream>
#include <cstdlib>
#include <fstream>
#include <bitset>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <queue>
#include <utility>

uint32_t registers[32];
bool flaggedRegisters[32] = {};
int32_t pc = 256;
std::unordered_map<int32_t, int32_t> dataMap;

bool Busy[4];       // Busy status of functional units
int Op[4];          // Operation assigned to each functional unit
int Fi[4];          // Destination register for each functional unit
int Fj[4];          // First source register
int Fk[4];          // Second source register
int Qj[4];          // Producer of the first source register
int Qk[4];          // Producer of the second source register
bool Rj[4];         // Ready status of the first source register (1 if ready, 0 if not)
bool Rk[4];         // Ready status of the second source register (1 if ready, 0 if not)
int Result[32];

template <std::size_t N1, std::size_t N2>
std::bitset<N1 + N2> concatinateBitsets(std::bitset<N1> bitset1, std::bitset<N2> bitset2) {
    std::bitset<N1 + N2> result;
    for (std::size_t i = 0; i < N1; ++i) {
        result[i] = bitset1[i];
    }
    for (std::size_t i = 0; i < N2; ++i) {
        result[N1 + i] = bitset2[i];
    }
    return result;
}

template <std::size_t N>
int32_t twoComplement(std::bitset<N> bits) {
    return bits[N-1] ? ((~bits).to_ullong() + 1) * -1 : bits.to_ullong();
}
int32_t twoComplement(int32_t v) {
    return v < 0 ? ~(v * -1) + 1: v;
}
std::string getRegistersStr() {
    std::stringstream registersStr;
    registersStr << "Registers";
    int row = 0;
    for (int i = 0; i < 32; ++i) {
        if (i % 8 == 0) {
            registersStr << std::endl;
            registersStr << (i < 10 ? "x0" : "x") << row << ":\t" << twoComplement(registers[i]);
            row += 8;
        } else {
            registersStr << "\t";
            registersStr << twoComplement(registers[i]);
        }
    }
    return registersStr.str();
}
std::string getDataMapStr(int32_t address) {
    std::stringstream DataMapStr;
    DataMapStr << "Data";
    address = address + 4;
    for (int i = 0; i < dataMap.size(); ++i) {
        if (i % 8 == 0) {
            DataMapStr << std::endl;
            DataMapStr << address << ":\t";
            DataMapStr << dataMap.at(address);
        } else {
            DataMapStr << "\t";
            DataMapStr << dataMap.at(address);
        }
        address = address + 4;
    }
    return DataMapStr.str();
}


struct Instruction {
    std::bitset<32> instCode;
    std::bitset<5> opCode;
    std::bitset<2> catCode;
    uint32_t address;
    Instruction(const std::bitset<32>& instCode, const uint32_t& ad) {
        this->address = ad;
        this->instCode = instCode.to_ullong();
        this->catCode = instCode.to_ullong();
        this->opCode = instCode.to_ullong() >> 2;
    };
    virtual void preformOperation() const = 0;
    virtual std::string instructionToString() const = 0;
    virtual void flagRegisters() const = 0;
    virtual void unflagRegisters() const = 0;
    virtual void flagSDRegisters() const = 0;
    virtual std::vector<unsigned int> getRegisters() const = 0;
};

struct Category1 : public Instruction {
    std::bitset<5> imm1;
    std::bitset<3> func3;
    std::bitset<5> s1;
    std::bitset<5> s2;
    std::bitset<7> imm2;
    
    Category1(const std::bitset<32>& instCode, const uint32_t& ad) : Instruction(instCode.to_ullong(), ad) {
        this->imm1 = instCode.to_ullong() >> 7;
        this->func3 = instCode.to_ullong() >> 12;
        this->s1 = instCode.to_ullong() >> 15;
        this->s2 = instCode.to_ullong() >> 20;
        this->imm2 = instCode.to_ullong() >> 25;
    };

    std::string instructionToString() const override{
        std::stringstream instructionStr;
        instructionStr << "address=" <<  this->address << std::endl;
        instructionStr << "instCode=" << this->instCode << std::endl;
        instructionStr << "catCode=" << this->catCode << std::endl;
        instructionStr << "opCode=" << this->opCode << std::endl;
        instructionStr << "imm1=" << this->imm1 << std::endl;
        instructionStr << "func3=" << this->func3 << std::endl;
        instructionStr << "s1=" << this->s1 << std::endl;
        instructionStr << "s2=" << this->s2 << std::endl;
        instructionStr << "imm2=" << this->imm2 << std::endl;
        return instructionStr.str();
    }
    void flagRegisters() const override{
        flaggedRegisters[this->s1.to_ulong()] = true;
        flaggedRegisters[this->s2.to_ulong()] = true;
    }
    void unflagRegisters() const override{
        flaggedRegisters[this->s1.to_ulong()] = false;
        flaggedRegisters[this->s2.to_ulong()] = false;
    }
    std::vector<unsigned int> getRegisters() const override{
        std::vector<unsigned int> result = {1, s1.to_ulong(), s2.to_ulong()};
        return result;
    }
};
struct Category2 : public Instruction {
    std::bitset<5> rd;
    std::bitset<3> func3;
    std::bitset<5> s1;
    std::bitset<5> s2;
    std::bitset<7> func7;

    Category2(const std::bitset<32>& instCode, const uint32_t& ad) : Instruction(instCode.to_ullong(), ad) {
        this->rd = instCode.to_ullong() >> 7;
        this->func3 = instCode.to_ullong() >> 12;
        this->s1 = instCode.to_ullong() >> 15;
        this->s2 = instCode.to_ullong() >> 20;
        this->func7 = instCode.to_ullong() >> 25;
    };

    std::string instructionToString() const override{
        std::stringstream instructionStr;
        instructionStr << "address=" <<  this->address << std::endl;
        instructionStr << "instCode=" << this->instCode << std::endl;
        instructionStr << "catCode=" << this->catCode << std::endl;
        instructionStr << "opCode=" << this->opCode << std::endl;
        instructionStr << "rd=" << this->rd << std::endl;
        instructionStr << "func3=" << this->func3 << std::endl;
        instructionStr << "s1=" << this->s1 << std::endl;
        instructionStr << "s2=" << this->s2 << std::endl;
        instructionStr << "func7=" << this->func7 << std::endl;
        return instructionStr.str();
    }
    void flagRegisters() const override{
        flaggedRegisters[this->s1.to_ulong()] = true;
        flaggedRegisters[this->s2.to_ulong()] = true;
        flaggedRegisters[this->rd.to_ulong()] = true;
    }
    void unflagRegisters() const override{
        flaggedRegisters[this->s1.to_ulong()] = false;
        flaggedRegisters[this->s2.to_ulong()] = false;
        flaggedRegisters[this->rd.to_ulong()] = false;
    }
    std::vector<unsigned int> getRegisters() const override{
        std::vector<unsigned int> result = {2, s1.to_ulong(), s2.to_ulong(), rd.to_ulong()};
        return result;
    }
};
struct Category3 : public Instruction {
    std::bitset<5> rd;
    std::bitset<3> func3;
    std::bitset<5> s1;
    std::bitset<12> imm;

    Category3(const std::bitset<32>& instCode, const uint32_t& ad) : Instruction(instCode.to_ullong(), ad) {
        this->rd = instCode.to_ullong() >> 7;
        this->func3 = instCode.to_ullong() >> 12;
        this->s1 = instCode.to_ullong() >> 15;
        this->imm = instCode.to_ullong() >> 20;
    };

    std::string instructionToString() const override{
        std::stringstream instructionStr;
        instructionStr << "address=" <<  this->address << std::endl;
        instructionStr << "instCode=" << this->instCode << std::endl;
        instructionStr << "catCode=" << this->catCode << std::endl;
        instructionStr << "opCode=" << this->opCode << std::endl;
        instructionStr << "rd=" << this->rd << std::endl;
        instructionStr << "func3=" << this->func3 << std::endl;
        instructionStr << "s1=" << this->s1 << std::endl;
        instructionStr << "imm=" << this->imm << std::endl;
        return instructionStr.str();
    }
    void flagRegisters() const override{
        flaggedRegisters[this->s1.to_ulong()] = true;
        flaggedRegisters[this->rd.to_ulong()] = true;
    }
    void unflagRegisters() const override{
        flaggedRegisters[this->s1.to_ulong()] = false;
        flaggedRegisters[this->rd.to_ulong()] = false;
    }
    std::vector<unsigned int> getRegisters() const override{
        std::vector<unsigned int> result = {3, s1.to_ulong(), rd.to_ulong()};
        return result;
    }
};
struct Category4 : public Instruction {
    std::bitset<5> rd;
    std::bitset<20> imm;

    Category4(const std::bitset<32>& instCode, const uint32_t& ad) : Instruction(instCode.to_ullong(), ad) {
        this->rd = instCode.to_ullong() >> 7;
        this->imm = instCode.to_ullong() >> 12;
    };

    std::string instructionToString() const override{
        std::stringstream instructionStr;
        instructionStr << "address=" <<  this->address << std::endl;
        instructionStr << "instCode=" << this->instCode << std::endl;
        instructionStr << "catCode=" << this->catCode << std::endl;
        instructionStr << "opCode=" << this->opCode << std::endl;
        instructionStr << "rd=" << this->rd << std::endl;
        instructionStr << "imm=" << this->imm << std::endl;
        return instructionStr.str();
    }
    void flagRegisters() const override{
        flaggedRegisters[this->rd.to_ulong()] = true;
    }
    void unflagRegisters() const override{
        flaggedRegisters[this->rd.to_ulong()] = false;
    }
    std::vector<unsigned int> getRegisters() const override{
        std::vector<unsigned int> result = {4, rd.to_ulong()};
        return result;
    }
};

struct BEQ : public Category1 {
    BEQ(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        if (registers[this->s1.to_ullong()] == registers[this->s2.to_ullong()]) {
            pc = pc + (twoComplement(concatinateBitsets(this->imm1, this->imm2)) << 1);
        }
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tbeq x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << std::endl;
        return str.str();
    }
};
struct BNE : public Category1 {
    BNE(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        if (registers[this->s1.to_ullong()] != registers[this->s2.to_ullong()]) {
            pc = pc + (twoComplement(concatinateBitsets(this->imm1, this->imm2)) << 1);
        }
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tbne x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << std::endl;
        return str.str();
    }
};
struct BLT : public Category1 {
    BLT(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        int32_t v2 = twoComplement(registers[this->s2.to_ullong()]);
        int32_t v1 = twoComplement(registers[this->s1.to_ullong()]);

        if (v1 < v2) {
            pc = pc + (twoComplement(concatinateBitsets(this->imm1, this->imm2)) << 1);
        }
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tblt x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << std::endl;
        return str.str();
    }
};
struct SW : public Category1 {
    int32_t immV;

    SW(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {
        this->immV = twoComplement(concatinateBitsets(this->imm1, this->imm2));
    }
    void preformOperation() const override {
        dataMap[immV + registers[this->s2.to_ullong()]] = registers[this->s1.to_ullong()];
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tsw x" << twoComplement(this->s1.to_ullong()) << ", " << this->immV << "(x" << twoComplement(this->s2.to_ullong()) << ")" << std::endl;
        return str.str();
    }
};

struct ADD : public Category2 {
    ADD(const std::bitset<32>& instCode, const uint32_t& ad) : Category2(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = twoComplement(twoComplement(registers[this->s1.to_ullong()]) + twoComplement(registers[this->s2.to_ullong()]));
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tadd x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
        return str.str();
    }
};
struct SUB : public Category2 {
    SUB(const std::bitset<32>& instCode, const uint32_t& ad) : Category2(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = twoComplement(twoComplement(registers[this->s1.to_ullong()]) - twoComplement(registers[this->s2.to_ullong()]));
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tsub x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
        return str.str();
    }
};
struct AND : public Category2 {
    AND(const std::bitset<32>& instCode, const uint32_t& ad) : Category2(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = registers[this->s1.to_ullong()] & registers[this->s2.to_ullong()];
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tand x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
        return str.str();
    }
};
struct OR : public Category2 {
    OR(const std::bitset<32>& instCode, const uint32_t& ad) : Category2(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = registers[this->s1.to_ullong()] | registers[this->s2.to_ullong()];
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tor x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
        return str.str();
    }
};

struct ADDI : public Category3 {
    ADDI(const std::bitset<32>& instCode, const uint32_t& ad) : Category3(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = twoComplement(twoComplement(registers[this->s1.to_ullong()]) + twoComplement(this->imm.to_ullong()));
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\taddi x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
        return str.str();
    }
};
struct ANDI : public Category3 {
    ANDI(const std::bitset<32>& instCode, const uint32_t& ad) : Category3(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = registers[this->s1.to_ullong()] & this->imm.to_ullong();
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tandi x" << twoComplement(this->rd.to_ullong()) << ", x"<< twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
        return str.str();
    }
};
struct ORI : public Category3 {
    ORI(const std::bitset<32>& instCode, const uint32_t& ad) : Category3(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = registers[this->s1.to_ullong()] | this->imm.to_ullong();
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tori x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
        return str.str();
    }
};
struct SLL : public Category3 {
    SLL(const std::bitset<32>& instCode, const uint32_t& ad) : Category3(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = registers[this->s1.to_ullong()] << twoComplement(this->imm.to_ullong());
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tsll x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
        return str.str();
    }
};
struct SRA : public Category3 {
    SRA(const std::bitset<32>& instCode, const uint32_t& ad) : Category3(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = registers[this->s1.to_ullong()] >> twoComplement(this->imm.to_ullong());
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tsra x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
        return str.str();
    }
};
struct LW : public Category3 {
    LW(const std::bitset<32>& instCode, const uint32_t& ad) : Category3(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = twoComplement(dataMap[twoComplement(this->imm.to_ullong()) + twoComplement(registers[this->s1.to_ullong()])]);
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tlw x" << twoComplement(this->rd.to_ullong()) << ", " << twoComplement(this->imm.to_ullong()) << "(x" << this->s1.to_ullong() << ")" << std::endl;
        return str.str();
    }
};

struct JAL : public Category4 {
    JAL(const std::bitset<32>& instCode, const uint32_t& ad) : Category4(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = pc + 4;
        pc = pc + (twoComplement(this->imm) << 1);
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tjal x" << twoComplement(this->rd.to_ullong()) << ", #" << twoComplement(this->imm) << std::endl;
        return str.str();
    }
};
struct BREAK : public Category4 {
    BREAK(const std::bitset<32>& instCode, const uint32_t& ad) : Category4(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        pc = -4;
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\tbreak" << std::endl;
        return str.str();
    }
};

std::string simulateInstructions(std::vector<std::unique_ptr<Instruction>>& instructionList) {
    std::stringstream tableStr;
    int cycleCounter = 0;
    while(pc) {
        tableStr << "--------------------" << std::endl;
        tableStr << "Cycle " << ++cycleCounter << ":\t" << instructionList[(pc - 256)/4]->instructionToString();
        instructionList[(pc - 256)/4]->preformOperation();
        tableStr << getRegistersStr() << std::endl;
        tableStr << getDataMapStr(instructionList.back()->address) << std::endl;
        pc = pc + 4;
    }
    return tableStr.str();
}

int32_t loadData(int32_t address, std::vector<std::bitset<32>>& codes) {
    if (codes[(address-256)/4].to_ullong() & 0b1111111 != 124) {
        return 0;
    }
    int32_t breakAddress = address;
    address = address + 4;
    for (int i = (address-256)/4; i < codes.size(); ++i) {
        dataMap[address] = twoComplement(codes[i]);
        address = address + 4;
    }

    return (address - breakAddress)/4;
}
int32_t createInstructions(std::vector<std::unique_ptr<Instruction>>& instructionList, std::vector<std::bitset<32>>& codes) {
    int address = 256;
    for (std::bitset<32> c : codes) {
        int opcode = c.to_ullong() & 0b1111111;
        if (opcode == 0) {
            instructionList.push_back(std::make_unique<JAL>(c, address));
        } else if (opcode == 1) {
            instructionList.push_back(std::make_unique<ADD>(c, address));
        } else if (opcode == 2) {
            instructionList.push_back(std::make_unique<ADDI>(c, address));
        } else if (opcode == 3) {
            instructionList.push_back(std::make_unique<BEQ>(c, address));
        } else if (opcode == 5) {
            instructionList.push_back(std::make_unique<SUB>(c, address));
        } else if (opcode == 6) {
            instructionList.push_back(std::make_unique<ANDI>(c, address));
        } else if (opcode == 7) {
            instructionList.push_back(std::make_unique<BNE>(c, address));
        } else if (opcode == 9) {
            instructionList.push_back(std::make_unique<AND>(c, address));
        } else if (opcode == 10) {
            instructionList.push_back(std::make_unique<ORI>(c, address));
        } else if (opcode == 11) {
            instructionList.push_back(std::make_unique<BLT>(c, address));
        } else if (opcode == 13) {
            instructionList.push_back(std::make_unique<OR>(c, address));
        } else if (opcode == 14) {
            instructionList.push_back(std::make_unique<SLL>(c, address));
        } else if (opcode == 15) {
            instructionList.push_back(std::make_unique<SW>(c, address));
        } else if (opcode == 18) {
            instructionList.push_back(std::make_unique<SRA>(c, address));
        } else if (opcode == 22) {
            instructionList.push_back(std::make_unique<LW>(c, address));
        } else if (opcode == 124) {
            instructionList.push_back(std::make_unique<BREAK>(c, address));
            return address;
        } else {
            std::cerr << "INVALID INSTRUCTION: " << c.to_ullong() << std::endl;
            std::exit(EXIT_FAILURE);
        }
        address = address + 4;
    }
    std::cerr << "BREAK NOT FOUND: " << address << std::endl;
    std::exit(EXIT_FAILURE);
}
bool getCodes(std::vector<std::bitset<32>>& codes, std::string fileName) {
    std::ifstream inputFile(fileName);
    if (!inputFile) {
        std::cerr << "Error: Failed Opening Input File." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    while (!inputFile.eof()) {
        std::string codeLine;
        std::getline(inputFile, codeLine);
        codes.push_back(std::bitset<32>(codeLine));
    }

    inputFile.close();
    return true;
}
std::string getDisassembly(std::vector<std::bitset<32>>& codes, std::vector<std::unique_ptr<Instruction>>& instructions) {
    std::stringstream disassemblyStr;
    for (int i = 0; i < codes.size(); ++i) {
        disassemblyStr << codes[i] << "\t" << ((i < instructions.size()) ? instructions[i]->instructionToString() : std::to_string(instructions.back()->address+4*(i-instructions.size()+1)) + "\t" + std::to_string(dataMap[instructions.back()->address+4*(i-instructions.size()+1)]) + "\n");
    }
    return disassemblyStr.str();
}

bool writeDisassembly(const std::string& fileName, const std::string& disassemblyStr) {
    std::ofstream outputFile(fileName);
    if (!outputFile) {
        std::cerr << "Error: Failed Opening Output File." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    outputFile << disassemblyStr << std::endl;

    outputFile.close();
    return true;
}
bool writeSim(const std::string& fileName, const std::string& instructionStr) {
    std::ofstream outputFile(fileName);
    if (!outputFile) {
        std::cerr << "Error: Failed Opening Output File." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    outputFile << instructionStr;

    outputFile.close();
    return true;
}

struct IF
{
    bool stalled = false;
    bool preIssueFull = false;
    int instr1 = (pc-256)/4;
    int instr2 = (pc-256)/4;
    int waiting = -1;
    int executing = -1;
    void fetch(std::vector<std::unique_ptr<Instruction>> &instructionList) {
        if (preIssueFull) {
            return;
        }

        if (nextInstructionBranch(instructionList, executing)) {
            instructionList[executing]->preformOperation();
            executing = -1;
        }

        if (nextInstructionBreak(instructionList, (pc-256)/4)) {
            executing = (pc-256)/4;
            return;
        }

        if (stalled) {
            if (!nextBranchUsesFlagged(instructionList, waiting)) {
                stalled = false;
                executing = waiting;
                waiting = -1;
            }
            return;
        }

        instr1 = (pc-256)/4;
        pc = pc + 4;
        instr2 = (pc-256)/4;

        if (nextInstructionBranch(instructionList, instr1)) {
            instr2 = -1;
            waiting = instr1;
            stalled = true;
        }
        if (nextInstructionBranch(instructionList, instr2)) {
            instr1 = -1;
            waiting = instr2;
            stalled = true;
        }
    }
    bool nextBranchUsesFlagged(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentpc) {
        for (int i = 0; i < instructionList[currentpc]->getRegisters().size(); ++i) {
            if (flaggedRegisters[instructionList[currentpc]->getRegisters()[i]]) {
                return true;
            }
        }
        return false;
    }
    bool nextInstructionBranch(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentpc) {
        if (currentpc < 0) {
            return false;
        }
        unsigned long code = (std::bitset<7>(instructionList[currentpc]->opCode.to_ulong()) << 2 | std::bitset<7>(instructionList[currentpc]->catCode.to_ulong())).to_ulong();
        return code == 3 || code == 7 || code == 11 || code == 0;
    }
    bool nextInstructionBreak(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentpc) {
        unsigned long code = (std::bitset<7>(instructionList[currentpc]->opCode.to_ulong()) << 2 | std::bitset<7>(instructionList[currentpc]->catCode.to_ulong())).to_ulong();
        return code == 124;
    }
    std::pair<int, int> decode(std::vector<std::unique_ptr<Instruction>> &instructionList) {
        if (!stalled && instr2 < 0) {
            return std::make_pair(instr1, -1);
        }
        if (!stalled) {
            return std::make_pair(instr1, instr2);
        }
        return std::make_pair(-1, -1);
    }
    void cycle(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> &preIssue) {
        preIssueFull = preIssue.size() > 3;
        fetch(instructionList);
        if (!preIssueFull && decode(instructionList).first > -1 && !nextInstructionBranch(instructionList, decode(instructionList).first)) {
            preIssue.push(decode(instructionList).first);
            instructionList[decode(instructionList).first]->flagRegisters();
        }
        preIssueFull = preIssue.size() > 3;
        if (!preIssueFull && decode(instructionList).second > -1 && !nextInstructionBranch(instructionList, decode(instructionList).second)) {
            preIssue.push(decode(instructionList).second);
            instructionList[decode(instructionList).second]->flagRegisters();
        }
    }
    std::string IFtoString(std::vector<std::unique_ptr<Instruction>> &instructionList) {
        std::stringstream outputstring;
        outputstring << "IF Unit:\n\tWaiting:";
        if (waiting != -1) {
            outputstring << " " << instructionList[waiting]->instructionToString();
        } else {
            outputstring << "\n";
        }
        outputstring << "\tExecuted:";
        if (executing != -1) {
            outputstring << " " << instructionList[executing]->instructionToString();
        } else {
            outputstring << "\n";
        }
        return outputstring.str();
    }
};

void readFlagged() {
    for (int i = 0; i < 32; i++) {
        std::cout << flaggedRegisters[i];
        if (i != 31) {
            std::cout << ",";
        }
    }
}

std::string writePREISSUEQ(std::queue<int> preIssue, std::vector<std::unique_ptr<Instruction>> &instructionList) {
    std::stringstream resultstr;
    resultstr << "Pre-Issue Queue:\n\tEntry 0:";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    resultstr << "\n\tEntry 1:";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    resultstr << "\n\tEntry 2:";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    resultstr << "\n\tEntry 3:";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    return resultstr.str();
}
bool issueInst(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst, int FU) {
    if (!Busy[FU] && instructionList[inst]->getRegisters()[0] == 1 ? true : Result[instructionList[inst]->getRegisters().back()] == -1) {  // Check if FU is available and no instruction writes to dst
        // Mark the functional unit as busy
        Busy[FU] = true;
        
        // Assign the operation and registers to the functional unit
        Op[FU] = instructionList[inst]->address;
        Fi[FU] = instructionList[inst]->getRegisters()[0] == 1 ? -1 : instructionList[inst]->getRegisters().back();
        Fj[FU] = instructionList[inst]->getRegisters()[0] == 4 ? -1 : instructionList[inst]->getRegisters()[1];
        Fk[FU] = instructionList[inst]->getRegisters()[0] > 2 ? -1 : instructionList[inst]->getRegisters()[2];
        
        // Set the producer registers for the source registers
        Qj[FU] = instructionList[inst]->getRegisters()[0] == 4 ? -1 : Result[instructionList[inst]->getRegisters()[1]];
        Qk[FU] = instructionList[inst]->getRegisters()[0] > 2 ? -1 : Result[instructionList[inst]->getRegisters()[2]];
        
        // Set ready flags for source registers (1 if ready, 0 if not)
        Rj[FU] = (Qj[FU] == -1);  // If Qj is -1, the register is ready
        Rk[FU] = (Qk[FU] == -1);  // If Qk is -1, the register is ready
        
        // Update the Result array to show which FU will produce the result for the destination register
        if (instructionList[inst]->getRegisters()[0] != 1){
             Result[instructionList[inst]->getRegisters().back()] = FU;
        }
        return true;
    }
    return false;
}
struct issue {
    bool checkScoreboard(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst1, int FU) {
        if (instructionList[inst1]->getRegisters()[0] == 1 ? false : Result[instructionList[inst1]->getRegisters().back()] != -1) {  // Check if FU is available and no instruction writes to dst
            return false;
        }
        if (instructionList[inst1]->getRegisters()[0] > 2 ? -1 : Result[instructionList[inst1]->getRegisters()[2]] != -1 || instructionList[inst1]->getRegisters()[0] == 4 ? -1 : Result[instructionList[inst1]->getRegisters()[1]] != -1) {
            return false;
        }
        return true;
    }
    bool checkScoreboard(std::vector<std::unique_ptr<Instruction>> &instructionList, std::vector<int> previousInstructions, int inst, int FU) {
        bool BusyTest[4];       // Busy status of functional units
        int OpTest[4];          // Operation assigned to each functional unit
        int FiTest[4];          // Destination register for each functional unit
        int FjTest[4];          // First source register
        int FkTest[4];          // Second source register
        int QjTest[4];          // Producer of the first source register
        int QkTest[4];          // Producer of the second source register
        bool RjTest[4];         // Ready status of the first source register (1 if ready, 0 if not)
        bool RkTest[4];         // Ready status of the second source register (1 if ready, 0 if not)
        int ResultTest[32];
        for (int i = 0; i < 4; i++) {
            BusyTest[i] = false;
            OpTest[i] = -1;
            FiTest[i] = -1;
            FjTest[i] = -1;
            FkTest[i] = -1;
            QjTest[i] = -1;
            QkTest[i] = -1;
            RjTest[i] = false;
            RkTest[i] = false;
        }
        for (int i = 0; i < 32; i++) {
            ResultTest[i] = -1;  // -1 means the register is not currently being written by any FU
        }

        if (instructionList[inst]->getRegisters()[0] == 1 ? true : Result[instructionList[inst]->getRegisters().back()] == -1) {  // Check if FU is available and no instruction writes to dst
            // Mark the functional unit as busy

            // Assign the operation and registers to the functional unit
            OpTest[FU] = instructionList[inst]->address;
            FiTest[FU] = instructionList[inst]->getRegisters()[0] == 1 ? -1 : instructionList[inst]->getRegisters().back();
            FjTest[FU] = instructionList[inst]->getRegisters()[0] == 4 ? -1 : instructionList[inst]->getRegisters()[1];
            FkTest[FU] = instructionList[inst]->getRegisters()[0] > 2 ? -1 : instructionList[inst]->getRegisters()[2];

            // Set the producer registers for the source registers
            QjTest[FU] = instructionList[inst]->getRegisters()[0] == 4 ? -1 : ResultTest[instructionList[inst]->getRegisters()[1]];
            QkTest[FU] = instructionList[inst]->getRegisters()[0] > 2 ? -1 : ResultTest[instructionList[inst]->getRegisters()[2]];

            // Set ready flags for source registers (1 if ready, 0 if not)
            RjTest[FU] = (QjTest[FU] == -1);  // If Qj is -1, the register is ready
            RkTest[FU] = (QkTest[FU] == -1);  // If Qk is -1, the register is ready

            // Update the Result array to show which FU will produce the result for the destination register
            if (instructionList[inst]->getRegisters()[0] != 1){
                 ResultTest[instructionList[inst]->getRegisters().back()] = FU;
            }
        }

        if (instructionList[inst]->getRegisters()[0] == 1 ? false : ResultTest[instructionList[inst]->getRegisters().back()] != -1) {  // Check if FU is available and no instruction writes to dst
            return false;
        }
        if (instructionList[inst]->getRegisters()[0] > 2 ? -1 : ResultTest[instructionList[inst]->getRegisters()[2]] != -1 || instructionList[inst]->getRegisters()[0] == 4 ? -1 : ResultTest[instructionList[inst]->getRegisters()[1]] != -1) {
            return false;
        }
        return true;
    }

    void cycle(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> preIssue, std::queue<int>& alu1, std::queue<int>& alu2) {
        for (int i = 0; i < 4; i++) {
            if (!preIssue.empty()) {
                int inst = preIssue.front();
                preIssue.pop();

                if (isLoadorStore(instructionList, inst)) {
                    if (!openALU(alu1)) {
                        continue;
                    }
                    if (!checkScoreboard(instructionList, inst, 1)) {
                        continue;
                    }
                    issueInst(instructionList, inst, 1);
                } else {
                    if (!openALU(alu2)) {
                        continue;
                    }
                    if (!checkScoreboard(instructionList, inst, 2)) {
                        continue;
                    }
                    issueInst(instructionList, inst, 2);
                }
            }
        }
    }

    bool openALU(std::queue<int> preALU) {
        return preALU.size() < 3;
    }
    bool isLoadorStore(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentinst) {
        unsigned long code = (std::bitset<7>(instructionList[currentinst]->opCode.to_ulong()) << 2 | std::bitset<7>(instructionList[currentinst]->catCode.to_ulong())).to_ulong();
        return  code == 15 || code == 22;
    }
    bool RAW(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentinst) {
        std::vector<unsigned int> effectedRegisters = instructionList[currentinst]->getRegisters();
        switch (effectedRegisters[0]) {
            case 1:
                for (int i = 0; i < 32; ++i) {
                    if (destinationRegisters[i] == effectedRegisters[1] || destinationRegisters[i] == effectedRegisters[2]) {
                        return true;
                    }
                }
                break;
            case 2:
                for (int i = 0; i < 32; ++i) {
                    if (destinationRegisters[i] == effectedRegisters[1] || destinationRegisters[i] == effectedRegisters[2]) {
                        return true;
                    }
                }
                break;
            case 3:
                for (int i = 0; i < 32; ++i) {
                    if (destinationRegisters[i] == effectedRegisters[1]) {
                        return true;
                    }
                }
                break;
            case 4:
                break;
        }
        return false;
    }
    bool WAW(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentinst) {
        std::vector<unsigned int> effectedRegisters = instructionList[currentinst]->getRegisters();
        switch (effectedRegisters[0]) {
            case 1:
                break;
            case 2:
                for (int i = 0; i < 32; ++i) {
                    if (destinationRegisters[i] == effectedRegisters[3]) {
                        return true;
                    }
                }
                break;
            case 3:
                for (int i = 0; i < 32; ++i) {
                    if (destinationRegisters[i] == effectedRegisters[2]) {
                        return true;
                    }
                }
                break;
            case 4:
                for (int i = 0; i < 32; ++i) {
                    if (destinationRegisters[i] == effectedRegisters[1]) {
                        return true;
                    }
                }
                break;
        }
        return false;
    }
    bool WAR(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentinst) {
        std::vector<unsigned int> effectedRegisters = instructionList[currentinst]->getRegisters();
        switch (effectedRegisters[0]) {
            case 1:
                break;
            case 2:
                for (int i = 0; i < 32; ++i) {
                    if (sourceRegisters[i] == effectedRegisters[3]) {
                        return true;
                    }
                }
                break;
            case 3:
                for (int i = 0; i < 32; ++i) {
                    if (sourceRegisters[i] == effectedRegisters[2]) {
                        return true;
                    }
                }
                break;
            case 4:
                for (int i = 0; i < 32; ++i) {
                    if (sourceRegisters[i] == effectedRegisters[1]) {
                        return true;
                    }
                }
                break;
        }
        return false;
    }
    bool instructionsIndependent(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentinst1, int currentinst2) {
        std::vector<unsigned int> effectedRegisters1 = instructionList[currentinst1]->getRegisters();
        std::vector<unsigned int> effectedRegisters2 = instructionList[currentinst2]->getRegisters();
        switch(effectedRegisters1[0]) {
            case 1: //s1 s2
                if (effectedRegisters2[0] == 1) { //s1 s2
                    return true;
                }
                if (effectedRegisters2[0] == 2) { //s1 s2 rd
                    if (effectedRegisters2[3] == effectedRegisters1[1] || effectedRegisters2[3] == effectedRegisters1[2]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 3) { //s1 rd
                    if (effectedRegisters2[2] == effectedRegisters1[1] || effectedRegisters2[2] == effectedRegisters1[2]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 4) { //rd
                    if (effectedRegisters2[1] == effectedRegisters1[1] || effectedRegisters2[1] == effectedRegisters1[2]) {
                        return false;
                    }
                    return true;
                }
                break;
            case 2: //s1 s2 rd
                if (effectedRegisters2[0] == 1) { //s1 s2
                    if (effectedRegisters2[1] == effectedRegisters1[3] || effectedRegisters2[2] == effectedRegisters1[3]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 2) { //s1 s2 rd
                    if (effectedRegisters2[1] == effectedRegisters1[3] || effectedRegisters2[2] == effectedRegisters1[3] || effectedRegisters2[3] == effectedRegisters1[3] || effectedRegisters2[3] == effectedRegisters1[1] || effectedRegisters2[3] == effectedRegisters1[2]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 3) { //s1 rd
                    if (effectedRegisters2[1] == effectedRegisters1[3] || effectedRegisters2[2] == effectedRegisters1[3] || effectedRegisters2[2] == effectedRegisters1[1] || effectedRegisters2[2] == effectedRegisters1[2]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 4) { //rd
                    if (effectedRegisters2[1] == effectedRegisters1[3] || effectedRegisters2[1] == effectedRegisters1[1] || effectedRegisters2[1] == effectedRegisters1[2]) {
                        return false;
                    }
                    return true;
                }
                break;
            case 3: //s1 rd
                if (effectedRegisters2[0] == 1) { //s1 s2
                    if (effectedRegisters2[1] == effectedRegisters1[2] || effectedRegisters2[2] == effectedRegisters1[2]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 2) { //s1 s2 rd
                    if (effectedRegisters2[1] == effectedRegisters1[2] || effectedRegisters2[2] == effectedRegisters1[2] || effectedRegisters2[3] == effectedRegisters1[2] || effectedRegisters2[3] == effectedRegisters1[1]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 3) { //s1 rd
                    if (effectedRegisters2[1] == effectedRegisters1[2] || effectedRegisters2[2] == effectedRegisters1[2] || effectedRegisters2[2] == effectedRegisters1[1]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 4) { //rd
                    if (effectedRegisters2[1] == effectedRegisters1[2] || effectedRegisters2[1] == effectedRegisters1[1]) {
                        return false;
                    }
                    return true;
                }
                break;
            case 4: //rd
                if (effectedRegisters2[0] == 1) { //s1 s2
                    if (effectedRegisters2[1] == effectedRegisters1[1] || effectedRegisters2[2] == effectedRegisters1[1]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 2) { //s1 s2 rd
                    if (effectedRegisters2[1] == effectedRegisters1[1] || effectedRegisters2[2] == effectedRegisters1[1] || effectedRegisters2[3] == effectedRegisters1[1]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 3) { //s1 rd
                    if (effectedRegisters2[1] == effectedRegisters1[1] || effectedRegisters2[2] == effectedRegisters1[1]) {
                        return false;
                    }
                    return true;
                }
                if (effectedRegisters2[0] == 4) { //rd
                    if (effectedRegisters2[1] == effectedRegisters1[1]) {
                        return false;
                    }
                    return true;
                }
                break;
        }
        return false;
    }

};

int main(int argc, char *argv[]) {
    //if (argc != 2) {
    //    std::cerr << "Error: Input File is Required." << std::endl;
    //    std::exit(EXIT_FAILURE);
    //}



    std::vector<std::bitset<32>> codes;
    std::vector<std::unique_ptr<Instruction>> instructionList;

    //getCodes(codes, argv[1]);
    getCodes(codes, "sample.txt");
    uint32_t breakAddress = createInstructions(instructionList, codes);
    loadData(breakAddress, codes);

    IF instructionDecoder;
    std::queue<int> preIssue;
    std::queue<int> preALU1;
    std::queue<int> preALU2;
    std::queue<int> postALU2;
    std::queue<int> preMem;
    std::queue<int> postMem;

    for (int i = 0; i < 4; i++) {
        Busy[i] = false;
        Op[i] = -1;
        Fi[i] = -1;
        Fj[i] = -1;
        Fk[i] = -1;
        Qj[i] = -1;
        Qk[i] = -1;
        Rj[i] = false;
        Rk[i] = false;
    }
    for (int i = 0; i < 32; i++) {
        Result[i] = -1;  // -1 means the register is not currently being written by any FU
    }

    instructionDecoder.cycle(instructionList, preIssue);
    std::cout << instructionDecoder.IFtoString(instructionList);
    std::cout << writePREISSUEQ(preIssue, instructionList);
    instructionDecoder.cycle(instructionList, preIssue);
    std::cout << instructionDecoder.IFtoString(instructionList);
    std::cout << writePREISSUEQ(preIssue, instructionList);
    instructionDecoder.cycle(instructionList, preIssue);
    std::cout << instructionDecoder.IFtoString(instructionList);
    std::cout << writePREISSUEQ(preIssue, instructionList);
    flaggedRegisters[1] = false;
    flaggedRegisters[2] = false;
    instructionDecoder.cycle(instructionList, preIssue);
    std::cout << instructionDecoder.IFtoString(instructionList);
    std::cout << writePREISSUEQ(preIssue, instructionList);
    instructionDecoder.cycle(instructionList, preIssue);
    std::cout << instructionDecoder.IFtoString(instructionList);
    std::cout << writePREISSUEQ(preIssue, instructionList);
    instructionDecoder.cycle(instructionList, preIssue);
    std::cout << instructionDecoder.IFtoString(instructionList);
    std::cout << writePREISSUEQ(preIssue, instructionList);

    //writeDisassembly(OUTPUTDISASSEMBLY, getDisassembly(codes, instructionList));
    //writeSim(OUTPUTSIMULATION, simulateInstructions(instructionList));

    return 0;
}