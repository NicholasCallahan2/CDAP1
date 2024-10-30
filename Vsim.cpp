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

uint32_t registers[32];
int32_t pc = 256;
std::unordered_map<int32_t, int32_t> dataMap;

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
};

struct BEQ : public Category1 {
    BEQ(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        if (registers[this->s1.to_ullong()] == registers[this->s2.to_ullong()]) {
            pc = pc + (twoComplement(concatinateBitsets(this->imm1, this->imm2)) << 1) - 4;
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
            pc = pc + (twoComplement(concatinateBitsets(this->imm1, this->imm2)) << 1) - 4;
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
            pc = pc + (twoComplement(concatinateBitsets(this->imm1, this->imm2)) << 1) - 4;
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
        registers[this->rd.to_ullong()] = twoComplement(twoComplement(registers[this->s1.to_ullong()]) + twoComplement(this->imm));
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << "\taddi x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm) << std::endl;
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
        pc = pc + (twoComplement(this->imm) << 1) - 4;
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

    writeDisassembly(OUTPUTDISASSEMBLY, getDisassembly(codes, instructionList));
    writeSim(OUTPUTSIMULATION, simulateInstructions(instructionList));

    return 0;
}