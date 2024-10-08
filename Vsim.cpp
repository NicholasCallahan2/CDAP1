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

int32_t twoComplement(std::bitset<32> bits) {
    return bits[31] ? ((~bits).to_ullong() + 1) * -1 : bits.to_ullong();
}
int32_t twoComplement(std::bitset<20> bits) {
    return bits[19] ? ((~bits).to_ullong() + 1) * -1 : bits.to_ullong();
}
int32_t twoComplement(int32_t v) {
    return v < 0 ? ~(v * -1) + 1: v;
}

std::string getRegistersStr() {
    std::stringstream registersStr;
    registersStr << "registers";
    int row = 0;
    for (int i = 0; i < 32; ++i) {
        if (i % 8 == 0) {
            registersStr << std::endl;
            registersStr << (i < 10 ? "x0" : "x") << row << "\t" << twoComplement(registers[i]);
            row += 8;
        } else {
            registersStr << "\t ";
            registersStr << twoComplement(registers[i]);
        }
    }
    return registersStr.str();
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
            pc = this->imm1.to_ullong();
        }
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << " beq x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << std::endl;
        return str.str();
    }
};
struct BNE : public Category1 {
    BNE(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        if (registers[this->s1.to_ullong()] != registers[this->s2.to_ullong()]) {
            pc = this->imm1.to_ullong();
        }
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << " bne x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << std::endl;
        return str.str();
    }
};
struct BLT : public Category1 {
    BLT(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        int32_t v2 = twoComplement(registers[this->s2.to_ullong()]);
        int32_t v1 = twoComplement(registers[this->s1.to_ullong()]);

        if (v1 < v2) {
            pc = this->address + this->imm1.to_ullong() * 4;
        }
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << " blt x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << twoComplement(this->imm1.to_ullong()) << std::endl;
        return str.str();
    }
};
struct SW : public Category1 {
    int32_t immV;

    SW(const std::bitset<32>& instCode, const uint32_t& ad) : Category1(instCode.to_ullong(), ad) {
        this->immV = this->imm1.to_ullong() + this->imm2.to_ullong();
    }
    void preformOperation() const override {
        dataMap[immV + registers[this->s2.to_ullong()]] = registers[this->s1.to_ullong()];
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << " sw x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << ", #" << this->immV << std::endl;
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
        str << this->address << " add x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
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
        str << this->address << " sub x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
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
        str << this->address << " and x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
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
        str << this->address << " or x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", x" << twoComplement(this->s2.to_ullong()) << std::endl;
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
        str << this->address << " addi x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
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
        str << this->address << " andi x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
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
        str << this->address << " ori x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
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
        str << this->address << " sll x" << twoComplement(this->rd.to_ullong()) << ", x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
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
        str << this->address << " sra x" << twoComplement(this->s1.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << std::endl;
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
        str << this->address << " lw x" << twoComplement(this->rd.to_ullong()) << ", #" << twoComplement(this->imm.to_ullong()) << "(x" << this->s1.to_ullong() << ")" << std::endl;
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
        str << this->address << " jal x" << twoComplement(this->rd.to_ullong()) << ", #" << twoComplement(this->imm) << std::endl;
        return str.str();
    }
};
struct BREAK : public Category4 {
    BREAK(const std::bitset<32>& instCode, const uint32_t& ad) : Category4(instCode.to_ullong(), ad) {}
    void preformOperation() const override {
        pc = 0;
    }
    std::string instructionToString() const override {
        std::stringstream str;
        str << this->address << " break" << std::endl;
        return str.str();
    }
};



int main(int argc, char *argv[]) {
    //if (argc != 2) {
    //    std::cerr << "Error: Input File is Required." << std::endl;
    //    std::exit(EXIT_FAILURE);
    //}
    //std::ifstream inputFile(argv[1]);
    //if (!inputFile) {
    //    std::cerr << "Error: Failed Opening Input File." << std::endl;
    //    std::exit(EXIT_FAILURE);
    //}

    std::cout << twoComplement(std::bitset<20>(0b11111111111111111101)) << std::endl;
    //std::cout << twoComplement(0b11111111111111111110) << std::endl;

    std::vector<std::bitset<32>> codes;
    codes.push_back(std::bitset<32>(0b00000000000000000000000010000001));
    codes.push_back(std::bitset<32>(0b00000000001100000000000100000010));
    codes.push_back(std::bitset<32>(0b00000000001000001000101100000011));
    codes.push_back(std::bitset<32>(0b00000000001000001000001100001110));
    codes.push_back(std::bitset<32>(0b00010011100000110000000110010110));
    codes.push_back(std::bitset<32>(0b00010100010000110000001000010110));
    codes.push_back(std::bitset<32>(0b00000000010000011000001010000001));
    codes.push_back(std::bitset<32>(0b00000000000000101000001100001011));
    codes.push_back(std::bitset<32>(0b00000000010000011000001010000101));
    codes.push_back(std::bitset<32>(0b00000000000000000100001110000000));
    codes.push_back(std::bitset<32>(0b00010100011000101000001000001111));
    codes.push_back(std::bitset<32>(0b00000000000100001000000010000010));
    codes.push_back(std::bitset<32>(0b11111111111111101100010000000000));
    codes.push_back(std::bitset<32>(0b00000000000000000000000001111100));

    std::vector<std::unique_ptr<Instruction>> instructionList;
    for (auto c : codes) {
        int opcode = c.to_ullong() & 0b1111111;
        if (opcode == 0) {
            instructionList.push_back(std::make_unique<JAL>(c, pc));
        } else if (opcode == 1) {
            instructionList.push_back(std::make_unique<ADD>(c, pc));
        } else if (opcode == 2) {
            instructionList.push_back(std::make_unique<ADDI>(c, pc));
        } else if (opcode == 3) {
            instructionList.push_back(std::make_unique<BEQ>(c, pc));
        } else if (opcode == 5) {
            instructionList.push_back(std::make_unique<SUB>(c, pc));
        } else if (opcode == 6) {
            instructionList.push_back(std::make_unique<ANDI>(c, pc));
        } else if (opcode == 7) {
            instructionList.push_back(std::make_unique<BNE>(c, pc));
        } else if (opcode == 9) {
            instructionList.push_back(std::make_unique<AND>(c, pc));
        } else if (opcode == 10) {
            instructionList.push_back(std::make_unique<ORI>(c, pc));
        } else if (opcode == 11) {
            instructionList.push_back(std::make_unique<BLT>(c, pc));
        } else if (opcode == 13) {
            instructionList.push_back(std::make_unique<OR>(c, pc));
        } else if (opcode == 14) {
            instructionList.push_back(std::make_unique<SLL>(c, pc));
        } else if (opcode == 15) {
            instructionList.push_back(std::make_unique<SW>(c, pc));
        } else if (opcode == 18) {
            instructionList.push_back(std::make_unique<SRA>(c, pc));
        } else if (opcode == 22) {
            instructionList.push_back(std::make_unique<LW>(c, pc));
        } else if (opcode == 124) {
            instructionList.push_back(std::make_unique<BREAK>(c, pc));
        } else {
            std::cerr << "INVALID INSTRUCTION: " << c.to_ullong() << std::endl;
            std::exit(EXIT_FAILURE);
        }
        pc = pc + 4;
    }
    pc = 256;

    dataMap[312] = -1;
    dataMap[316] = -2;
    dataMap[320] = -4;
    dataMap[324] = 1;
    dataMap[328] = 2;
    dataMap[332] = 3;
    dataMap[336] = -4;
    dataMap[340] = 10;
    dataMap[344] = 7;
    dataMap[348] = 9;
    dataMap[352] = 1;
    dataMap[356] = 0;
    dataMap[360] = -1;

    int cycleCounter = 0;
    while(pc) {
        std::cout << "Cycle " << ++cycleCounter << ":\t" << instructionList[(pc - 256)/4]->instructionToString();
        instructionList[(pc - 256)/4]->preformOperation();
        std::cout << getRegistersStr() << std::endl;
        std::cout << pc << std::endl << std::endl;
        pc = pc + 4;
        if (cycleCounter > 40) {
            break;
        }
    }

    //ADD(i7, 280).preformOperation();
    //BLT(i8, 284).preformOperation();
    //SUB(i9, 288).preformOperation();
    //JAL(i10, 292).preformOperation();
    //ADDI(i12, 300).preformOperation();
    //JAL(i13, 304).preformOperation();
    //SLL(i4, 268).preformOperation();
    //LW(i5, 272).preformOperation();
    //LW(i6, 276).preformOperation();
    //ADD(i7, 280).preformOperation();
    //BLT(i8, 284).preformOperation();
    //SUB(i9, 288).preformOperation();
    //BEQ(i3, 264).preformOperation();
    //JAL(i10, 292).preformOperation();
    //ADDI(i12, 300).preformOperation();
    //JAL(i13, 304).preformOperation();
    //BEQ(i3, 264).preformOperation();
    //SLL(i4, 268).preformOperation();
    //LW(i5, 272).preformOperation();
    //LW(i6, 276).preformOperation();
    //ADD(i7, 280).preformOperation();
    //BLT(i8, 284).preformOperation();
    //SW(i11, 296).preformOperation();
    //ADDI(i12, 300).preformOperation();
    //JAL(i13, 304).preformOperation();
    //BEQ(i3, 264).preformOperation();
    //BREAK(i14, 308).preformOperation();



    //std::bitset<32> beq(0b00000000001000001000101100000011);
    //std::bitset<32> blt(0b00000000000000101000001100001011);
    //std::bitset<32> add(0b00000000010000011000001010000001);
    //std::bitset<32> addi(0b00000000000100001000000010000010);
    //std::bitset<32> jal(0b00000000000000000100001110000000);
    //std::bitset<32> sw(0b00010100011000101000001000001111);
    //std::bitset<32> sub(0b00000000010000011000001010000101);
    //registers[3] = 0b11111111111111111111111111111101; // -3
    //registers[4] = 0b00000000000000000000000000000001; // 1
    //SUB testingSUB(sub, 288);
    //testingSUB.preformOperation();
    //BLT testingBLT = BLT(blt, 284);
    //std::cout << pc << std::endl;
    // registers[5] < registers[0]
    //testingBLT.preformOperation();
    //std::cout << pc << std::endl;
    //SW testingSW = SW(sw, 296);
    //Category1 testingBEQ = Category1(beq, 264);
    //ADD testingADD = ADD(add, 280);
    //testingADD.preformOperation();
    //std::cout << std::bitset<32>(registers[5]);
    //Category3 testingADDI = Category3(addi, 300);
    //Category4 testingJAL = Category4(jal, 292);
    //std::cout << testingBLT.instructionToString();
}