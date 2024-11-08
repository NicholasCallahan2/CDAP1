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
#include <stdexcept>

uint32_t registers[32];
bool flaggedRegisters[32] = {};
int32_t pc = 256;
std::unordered_map<int32_t, int32_t> dataMap;

bool Busy[4];
int Op[4];
int Fi[4];
int Fj[4];
int Fk[4];
int Qj[4];
int Qk[4];
bool Rj[4];
bool Rk[4];
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
    return std::bitset<32>(v)[31] ? ~(v * -1) + 1: v;
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
        str << "[beq x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << "]";
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
        str << "[bne x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << "]";
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
        str << "[blt x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << "]";
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
        str << "[sw x" << twoComplement(this->s1.to_ullong()) << ", " << this->immV << "(x" << twoComplement(this->s2.to_ullong()) << ")" << "]";
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
        str << "[add x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << "]";
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
        str << "[sub x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << "]";
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
        str << "[and x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << "]";
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
        str << "[or x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << "]";
        return str.str();
    }
};

struct ADDI : public Category3 {
    ADDI(const std::bitset<32>& instCode, const uint32_t& ad) : Category3(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = twoComplement(twoComplement(registers[this->s1.to_ullong()]) + twoComplement(this->imm));
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << "[addi x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm) << "]";
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
        str << "[andi x" << twoComplement(this->rd.to_ullong()) << ", x"<< twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << "]";
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
        str << "[ori x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << "]";
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
        str << "[sll x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << "]";
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
        str << "[sra x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << "]";
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
        str << "[lw x" << twoComplement(this->rd.to_ullong()) << ", " << twoComplement(this->imm.to_ullong()) << "(x" << this->s1.to_ullong() << ")" << "]";
        return str.str();
    }
};

struct JAL : public Category4 {
    JAL(const std::bitset<32>& instCode, const uint32_t& ad) : Category4(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        registers[this->rd.to_ullong()] = pc + 4;
        pc = pc + ((twoComplement(this->imm)) << 1);
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << "[jal x" << twoComplement(this->rd.to_ullong()) << ", #" << twoComplement(this->imm) << "]";
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
        str << "[break" << "]";
        return str.str();
    }
};

bool checkScoreboard(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst1) {
    if (instructionList[inst1]->getRegisters()[0] == 1 ? false : Result[instructionList[inst1]->getRegisters().back()] != -1) {
        return false;
    }
    if ((instructionList[inst1]->getRegisters()[0] > 2 ? false : Result[instructionList[inst1]->getRegisters()[2]] != -1) || (instructionList[inst1]->getRegisters()[0] == 4 ? false : Result[instructionList[inst1]->getRegisters()[1]] != -1)) {
        return false;
    }
    return true;
}

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
        if (nextInstructionBreak(instructionList, (pc-256)/4)) {
            waiting = -1;
            executing = (pc-256)/4;
            throw std::runtime_error("break");
        }

        if (nextInstructionJAL(instructionList, executing)) {
            executing = -1;
        }

        if (nextInstructionBranch(instructionList, executing) && !nextInstructionJAL(instructionList, executing)) {
            int temp = pc;
            instructionList[executing]->preformOperation();
            if (pc == temp) {
                pc = pc + 4;
            }
            if (nextInstructionBreak(instructionList, (pc-256)/4)) {
                waiting = -1;
                executing = (pc-256)/4;
                throw std::runtime_error("break");
            }
            executing = -1;
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
        pc = pc + 4;
        
        if (nextInstructionBranch(instructionList, instr1)) {
            instr2 = -1;
            pc = pc - 8;
            if (nextInstructionJAL(instructionList, instr1)) {
                executing = instr1;
                instructionList[executing]->preformOperation();
            } else {
                waiting = instr1;
                stalled = true;
            }
        }
        if (nextInstructionBranch(instructionList, instr2)) {
            pc = pc - 4;
            if (nextInstructionJAL(instructionList, instr2)) {
                executing = instr2;
                instructionList[executing]->preformOperation();
            } else {
                waiting = instr2;
                stalled = true;
            }
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
    bool nextInstructionJAL(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentpc) {
        if (currentpc < 0) {
            return false;
        }
        unsigned long code = (std::bitset<7>(instructionList[currentpc]->opCode.to_ulong()) << 2 | std::bitset<7>(instructionList[currentpc]->catCode.to_ulong())).to_ulong();
        return code == 0;
    }
    bool nextInstructionBreak(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentpc) {
        if (currentpc < 0) {
            return false;
        }
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
            outputstring << " " << instructionList[waiting]->instructionToString() << std::endl;
        } else {
            outputstring << "\n";
        }
        outputstring << "\tExecuted:";
        if (executing != -1) {
            outputstring << " " << instructionList[executing]->instructionToString() << std::endl;
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
    resultstr << "Pre-Issue Queue:\n\tEntry 0: ";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    resultstr << "\n\tEntry 1: ";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    resultstr << "\n\tEntry 2: ";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    resultstr << "\n\tEntry 3: ";
    if (!preIssue.empty()) {
        resultstr << instructionList[preIssue.front()]->instructionToString();
        preIssue.pop();
    }
    return resultstr.str();
}
std::string writePREALU1(std::queue<int> alu1, std::vector<std::unique_ptr<Instruction>> &instructionList) {
    std::stringstream resultstr;
    resultstr << "\nPre-ALU1 Queue:\n\tEntry 0: ";
    if (!alu1.empty()) {
        resultstr << instructionList[alu1.front()]->instructionToString();
        alu1.pop();
    }
    resultstr << "\n\tEntry 1: ";
    if (!alu1.empty()) {
        resultstr << instructionList[alu1.front()]->instructionToString();
        alu1.pop();
    }
    return resultstr.str();
}
std::string writePREALU2(std::queue<int> alu2, std::vector<std::unique_ptr<Instruction>> &instructionList) {
    std::stringstream resultstr;
    resultstr << "\nPre-ALU2 Queue:\n\tEntry 0: ";
    if (!alu2.empty()) {
        resultstr << instructionList[alu2.front()]->instructionToString();
        alu2.pop();
    }
    resultstr << "\n\tEntry 1: ";
    if (!alu2.empty()) {
        resultstr << instructionList[alu2.front()]->instructionToString();
        alu2.pop();
    }
    return resultstr.str();
}
std::string writePreMemQueue(std::queue<int> memQ, std::vector<std::unique_ptr<Instruction>> &instructionList) {
    std::stringstream resultstr;
    resultstr << "\nPre-MEM Queue: ";
    if (!memQ.empty()) {
        resultstr << instructionList[memQ.front()]->instructionToString();
        memQ.pop();
    }
    return resultstr.str();
}
std::string writePostMemQueue(std::queue<int> memQ, std::vector<std::unique_ptr<Instruction>> &instructionList) {
    std::stringstream resultstr;
    resultstr << "\nPost-MEM Queue: ";
    if (!memQ.empty()) {
        resultstr << instructionList[memQ.front()]->instructionToString();
        memQ.pop();
    }
    return resultstr.str();
}
std::string writePostALU2(std::queue<int> alu2, std::vector<std::unique_ptr<Instruction>> &instructionList) {
    std::stringstream resultstr;
    resultstr << "\nPost-ALU2 Queue: ";
    if (!alu2.empty()) {
        resultstr << instructionList[alu2.front()]->instructionToString();
        alu2.pop();
    }
    return resultstr.str();
}
bool issueInst(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst, int FU) {
    if (!Busy[FU] && instructionList[inst]->getRegisters()[0] == 1 ? true : Result[instructionList[inst]->getRegisters().back()] == -1) {
        Busy[FU] = true;
        
        Op[FU] = instructionList[inst]->address;
        Fi[FU] = instructionList[inst]->getRegisters()[0] == 1 ? -1 : instructionList[inst]->getRegisters().back();
        Fj[FU] = instructionList[inst]->getRegisters()[0] == 4 ? -1 : instructionList[inst]->getRegisters()[1];
        Fk[FU] = instructionList[inst]->getRegisters()[0] > 2 ? -1 : instructionList[inst]->getRegisters()[2];
        
        Qj[FU] = instructionList[inst]->getRegisters()[0] == 4 ? -1 : Result[instructionList[inst]->getRegisters()[1]];
        Qk[FU] = instructionList[inst]->getRegisters()[0] > 2 ? -1 : Result[instructionList[inst]->getRegisters()[2]];
        
        Rj[FU] = (Qj[FU] == -1);
        Rk[FU] = (Qk[FU] == -1);
        
        if (instructionList[inst]->getRegisters()[0] != 1){
             Result[instructionList[inst]->getRegisters().back()] = FU;
        }
        return true;
    }
    return false;
}
bool unIssueInst(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst, int FU) {
    if (Busy[FU] || !(instructionList[inst]->getRegisters()[0] == 1 ? true : Result[instructionList[inst]->getRegisters().back()] == -1)) {
        Busy[FU] = false;
        
        Op[FU] = -1;
        Fi[FU] = -1;
        Fj[FU] = -1;
        Fk[FU] = -1;
        
        Qj[FU] = -1;
        Qk[FU] = -1;
        
        Rj[FU] = (Qj[FU] == -1);
        Rk[FU] = (Qk[FU] == -1);
        
        if (instructionList[inst]->getRegisters()[0] != 1){
             Result[instructionList[inst]->getRegisters().back()] = -1;
        }
        return true;
    }
    return false;
}
struct issue {
    bool checkMem(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst) {
        for (int i = 1; i < instructionList[inst]->getRegisters().size()-1; ++i) {
            if (Fi[1] == instructionList[inst]->getRegisters()[i]) {
                return false;
            }
        }
        for (int i = 1; i < instructionList[inst]->getRegisters().size()-1; ++i) {
            if (Fi[3] == instructionList[inst]->getRegisters()[i]) {
                return false;
            }
        }
        return true;
    }
    bool checkWAWorWAR(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst1, int inst2) {
        if (instructionList[inst1]->getRegisters()[0] != 1 && instructionList[inst2]->getRegisters()[0] != 1) {
            return true;
        }

        // waw
        if (instructionList[inst1]->getRegisters().back() == instructionList[inst2]->getRegisters().back()) {
            return false;
        }

        // war
        for (int i = 1; i < instructionList[inst2]->getRegisters().size()-1; ++i){
            if (instructionList[inst1]->getRegisters().back() == instructionList[inst2]->getRegisters()[i]) {
                return false;
            }
        }
        return true;
    }
    bool checkRAW(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst1, int inst2) {
        // raw
        for (int i = 1; i < instructionList[inst1]->getRegisters().size()-1; ++i){
            if (instructionList[inst2]->getRegisters().back() == instructionList[inst1]->getRegisters()[i]) {
                return false;
            }
        }
        return true;
    }

    bool checkDataHazard(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst1, std::vector<int> ealierInstructions) {
        if (!checkWAR(instructionList, inst1, ealierInstructions)) {
            return false;
        }

        for (int i = 0; i < ealierInstructions.size(); ++i) {
            int inst2 = ealierInstructions[i];
            for (int i = 1; i < instructionList[inst2]->getRegisters().size()-1; ++i){
                if (instructionList[inst1]->getRegisters()[0] != 1 && instructionList[inst2]->getRegisters()[0] != 1) {
                    if (instructionList[inst1]->getRegisters().back() == instructionList[inst2]->getRegisters().back()) {
                        return false;
                    }
                    for (int i = 1; i < instructionList[inst2]->getRegisters().size()-1; ++i){
                        if (instructionList[inst1]->getRegisters().back() == instructionList[inst2]->getRegisters()[i]) {
                            return false;
                        }
                    }
                    for (int i = 1; i < instructionList[inst1]->getRegisters().size()-1; ++i){
                        if (instructionList[inst2]->getRegisters().back() == instructionList[inst1]->getRegisters()[i]) {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }

    bool checkWAR(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst1, std::vector<int> ealierInstructions) {
        for (int i = 0; i < ealierInstructions.size(); ++i) {
            int inst2 = ealierInstructions[i];
            for (int i = 1; i < instructionList[inst2]->getRegisters().size()-1; ++i){
                if (instructionList[inst1]->getRegisters().back() == instructionList[inst2]->getRegisters()[i]) {
                return false;
                }
            }
        }
        return true;
    }
    bool checkLoadStoreOrder(std::vector<std::unique_ptr<Instruction>> &instructionList, int inst1, std::vector<int> ealierInstructions) {
        if (!isLoadStore(instructionList, inst1)) {
            return true;
        }
        if (isStore(instructionList, inst1)) {
            return true;
        }
        for (int i = 0; i < ealierInstructions.size(); ++i) {
            if (isStore(instructionList, ealierInstructions[i])) {
                return false;
            }
        }
        return true;
    }
    
    bool checkCanIssue(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> preIssue, int inst0, int inst1, std::queue<int>& alu1, std::queue<int>& alu2) {
        if (inst1 != -1 && !checkWAWorWAR(instructionList, inst0, inst1)) {
            return false;
        }
        if (inst1 != -1 && isLoadStore(instructionList, inst0) == isLoadStore(instructionList, inst1)) {
            return false;
        }
        if (!checkMem(instructionList, inst0)) {
            return false;
        }
        if (isLoadStore(instructionList, inst0)) {
            if (!openALU(alu1)) {
                return false;
            }
            if (!checkScoreboard(instructionList, inst0)) {
                return false;
            }
            return true;
        } else {
            if (!openALU(alu2)) {
                return false;
            }

            if (!checkScoreboard(instructionList, inst0)) {
                return false;
            }
            return true;
        }
    }

    std::vector<int> findValidInstructions(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> preIssue, std::queue<int>& alu1, std::queue<int>& alu2) {
        std::vector<int> skippedInstructions;
        std::vector<int> validInstructions;
        if (preIssue.empty()) {
            return std::vector<int> {};
        }
        int inst0 = preIssue.front();
        preIssue.pop();
        if (!checkCanIssue(instructionList, preIssue, inst0, -1, alu1, alu2)) {
            skippedInstructions.push_back(inst0);
            if (!preIssue.empty() && !isStore(instructionList, inst0)) {
                int inst1 = preIssue.front();
                preIssue.pop();
                if (!checkLoadStoreOrder(instructionList, inst1, skippedInstructions) || !checkDataHazard(instructionList, inst1, skippedInstructions) || !checkCanIssue(instructionList, preIssue, inst0, inst1, alu1, alu2)) {
                    skippedInstructions.push_back(inst1);
                    if (!preIssue.empty() && !isStore(instructionList, inst1)) {
                        int inst2 = preIssue.front();
                        preIssue.pop();
                        if (!checkLoadStoreOrder(instructionList, inst2, skippedInstructions) || !checkDataHazard(instructionList, inst2, skippedInstructions) || !checkCanIssue(instructionList, preIssue, inst1, inst2, alu1, alu2) || !checkCanIssue(instructionList, preIssue, inst0, inst2, alu1, alu2)) {
                            skippedInstructions.push_back(inst2);
                            if (!preIssue.empty() && !isStore(instructionList, inst2)) {
                                int inst3 = preIssue.front();
                                preIssue.pop();
                                if (!checkLoadStoreOrder(instructionList, inst3, skippedInstructions) || !checkDataHazard(instructionList, inst3, skippedInstructions) || !checkCanIssue(instructionList, preIssue, inst0, inst3, alu1, alu2) || !checkCanIssue(instructionList, preIssue, inst1, inst3, alu1, alu2) || !checkCanIssue(instructionList, preIssue, inst2, inst3, alu1, alu2)) {
                                    skippedInstructions.push_back(inst3);
                                } else {
                                    validInstructions.push_back(inst3);
                                }
                            }
                        } else {
                            validInstructions.push_back(inst2);
                        }
                    }
                } else {
                    validInstructions.push_back(inst1);
                }
            }
        } else {
            validInstructions.push_back(inst0);
            if (!preIssue.empty()) {
                int inst1 = preIssue.front();
                preIssue.pop();
                if (!checkLoadStoreOrder(instructionList, inst1, validInstructions) || !checkDataHazard(instructionList, inst1, validInstructions) || !checkCanIssue(instructionList, preIssue, inst0, inst1, alu1, alu2)) {
                    skippedInstructions.push_back(inst1);
                    if (!preIssue.empty() && !isStore(instructionList, inst1)) {
                        int inst2 = preIssue.front();
                        preIssue.pop();
                        if (!checkLoadStoreOrder(instructionList, inst2, skippedInstructions) || !checkLoadStoreOrder(instructionList, inst2, validInstructions) || !checkDataHazard(instructionList, inst2, validInstructions) || !checkDataHazard(instructionList, inst2, skippedInstructions) || !checkCanIssue(instructionList, preIssue, inst1, inst2, alu1, alu2) || !checkCanIssue(instructionList, preIssue, inst0, inst2, alu1, alu2)) {
                            skippedInstructions.push_back(inst2);
                            if (!preIssue.empty() && !isStore(instructionList, inst2)) {
                                int inst3 = preIssue.front();
                                preIssue.pop();
                                if (!checkLoadStoreOrder(instructionList, inst3, skippedInstructions) || !checkLoadStoreOrder(instructionList, inst3, validInstructions) || !checkDataHazard(instructionList, inst3, validInstructions) || !checkDataHazard(instructionList, inst3, skippedInstructions) || !checkCanIssue(instructionList, preIssue, inst0, inst3, alu1, alu2) || !checkCanIssue(instructionList, preIssue, inst1, inst3, alu1, alu2) || !checkCanIssue(instructionList, preIssue, inst2, inst3, alu1, alu2)) {
                                    skippedInstructions.push_back(inst3);
                                } else {
                                    validInstructions.push_back(inst3);
                                }
                            }
                        } else {
                            validInstructions.push_back(inst2);
                        }
                    }
                } else {
                    validInstructions.push_back(inst1);
                }
            }
        }
        return validInstructions;
    }

    void cycle(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> &preIssue, std::queue<int>& alu1, std::queue<int>& alu2) {
        std::vector<int> validInstructions = findValidInstructions(instructionList, preIssue, alu1, alu2);
        std::queue<int> temp;
        while(!preIssue.empty()) {
            for (int i = 0; i < validInstructions.size(); ++i) {
                if (preIssue.front() == validInstructions[i]) {
                    preIssue.pop();
                }
            }
            if (preIssue.empty()) {
                break;
            }
            temp.push(preIssue.front());
            preIssue.pop();
        }
        for (int i = 0; i < validInstructions.size(); ++i) {
            if (!issueInst(instructionList, validInstructions[i], isLoadStore(instructionList, validInstructions[i]) ? 1 : 2)) {
                std::cout << "cycle error";
            }

            if (isLoadStore(instructionList, validInstructions[i])) {
                alu1.push(validInstructions[i]);
            } else {
                alu2.push(validInstructions[i]);
            }
        }
        while(!temp.empty()) {
            preIssue.push(temp.front());
            temp.pop();
        }
    }

    bool openALU(std::queue<int> preALU) {
        return preALU.size() < 3;
    }
    bool isLoadStore(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentinst) {
        unsigned long code = (std::bitset<7>(instructionList[currentinst]->opCode.to_ulong()) << 2 | std::bitset<7>(instructionList[currentinst]->catCode.to_ulong())).to_ulong();
        return  code == 15 || code == 22;
    }
    bool isStore(std::vector<std::unique_ptr<Instruction>> &instructionList, int currentinst) {
        unsigned long code = (std::bitset<7>(instructionList[currentinst]->opCode.to_ulong()) << 2 | std::bitset<7>(instructionList[currentinst]->catCode.to_ulong())).to_ulong();
        return  code == 15;
    }

};

struct ALU {
    int FU;
    ALU(int aluNum) {
        FU = aluNum;
    }
    void cycle(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> &preALU, std::queue<int> &postALU) {
        if (preALU.empty() && postALU.size() < 2) {
            return;
        }

        unIssueInst(instructionList, preALU.front(), FU);
        postALU.push(preALU.front());
        issueInst(instructionList, postALU.front(), FU == 1 ? 3 : 0);
        preALU.pop();
    }
};

struct WriteBack {
    int cycle(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> &postMem) {
        if (postMem.empty()) {
            return -1;
        }
        int inst = postMem.front();
        instructionList[inst]->preformOperation();
        postMem.pop();
        return inst;
    }
};
struct postMemQ {
    int cycle(std::vector<std::unique_ptr<Instruction>> &instructionList, std::queue<int> &preALU, std::queue<int> &postALU) {
        if (preALU.empty()) {
            return -1;
        }
        int inst = preALU.front();
        postALU.push(inst);
        preALU.pop();
        return inst;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Error: Input File is Required." << std::endl;
        std::exit(EXIT_FAILURE);
    }



    std::vector<std::bitset<32>> codes;
    std::vector<std::unique_ptr<Instruction>> instructionList;

    getCodes(codes, argv[1]);
    uint32_t breakAddress = createInstructions(instructionList, codes);
    loadData(breakAddress, codes);

    IF instructionDecoder;
    issue issuer;
    ALU alu1(1);
    ALU alu2(2);
    WriteBack wb;
    postMemQ mem;

    std::queue<int> preIssue;
    std::queue<int> preALU1;
    std::queue<int> preALU2;
    std::queue<int> postALU2;
    std::queue<int> preMem;
    std::queue<int> postMem;

    std::stringstream output;

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
        Result[i] = -1;
    }

    bool breakFound = false;
    int cycleNum = 1;
    try {
        while (!breakFound) {
        output << "--------------------\nCycle " << cycleNum << ":\n\n";
        int inst3 = wb.cycle(instructionList, postMem);
        int inst1 = wb.cycle(instructionList, postALU2);
        int inst2 = mem.cycle(instructionList, preMem, postMem);
        alu2.cycle(instructionList, preALU2, postALU2);
        alu1.cycle(instructionList, preALU1, preMem);
        issuer.cycle(instructionList, preIssue, preALU1, preALU2);
        instructionDecoder.cycle(instructionList, preIssue);
        output << instructionDecoder.IFtoString(instructionList);
        output << writePREISSUEQ(preIssue, instructionList);
        output << writePREALU1(preALU1, instructionList);
        output << writePreMemQueue(preMem, instructionList);
        output << writePostMemQueue(postMem, instructionList);
        output << writePREALU2(preALU2, instructionList);
        output << writePostALU2(postALU2, instructionList) << std::endl << std::endl;
        output << getRegistersStr() << std::endl << std::endl;
        output << getDataMapStr(instructionList.back()->address) << std::endl;
        if (inst1 != -1) {
            instructionList[inst1]->unflagRegisters();
            unIssueInst(instructionList, inst1, 0);
        }
        if (inst3 != -1) {
            instructionList[inst3]->unflagRegisters();
            unIssueInst(instructionList, inst3, 3);
        }
        cycleNum++;
        }
    } 
    catch(std::runtime_error) {
        output << instructionDecoder.IFtoString(instructionList);
        output << writePREISSUEQ(preIssue, instructionList);
        output << writePREALU1(preALU1, instructionList);
        output << writePreMemQueue(preMem, instructionList);
        output << writePostMemQueue(postMem, instructionList);
        output << writePREALU2(preALU2, instructionList);
        output << writePostALU2(postALU2, instructionList) << std::endl << std::endl;
        output << getRegistersStr() << std::endl << std::endl;
        output << getDataMapStr(instructionList.back()->address) << std::endl;
    }

    std::string fileName = "simulation.txt";
    std::ofstream outputFile(fileName);
    if (!outputFile) {
        std::cerr << "Error: Failed Opening Output File." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    outputFile << output.str() << std::endl;
    outputFile.close();
    return 0;
}