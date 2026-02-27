#include <verilated.h>
#include <iostream>
#include <iomanip>
#include "../build/obj_dir/Vtop.h"

// 指令定义
#define ADDI 0b0010011
#define JALR 0b1100111
#define ADD  0b0110011
#define LUI  0b0110111
#define LW   0b0000011
#define LBU  0b0000011
#define SW   0b0100011
#define SB   0b0100011

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vtop* top = new Vtop;
    
    int pass_count = 0;
    int test_count = 0;
    
    std::cout << "=== IDU Test Start ===" << std::endl;
    
    // 测试用例1: ADDI (I类型)
    {
        top->inst = 0x00850513;  // addi x10, x10, 8
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 0 && top->rd == 10 && top->rs1 == 10 && 
            top->imm_I == 8 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test1 (ADDI): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test1 (ADDI): FAIL" << std::endl;
        }
    }
    
    // 测试用例2: JALR (I类型)
    {
        top->inst = 0x00050067;  // jalr x0, x10, 0
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 0 && top->rd == 0 && top->rs1 == 10 && 
            top->imm_I == 0 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test2 (JALR): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test2 (JALR): FAIL" << std::endl;
        }
    }
    
    // 测试用例3: ADD (R类型)
    {
        top->inst = 0x00A50533;  // add x10, x10, x10
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 1 && top->rd == 10 && top->rs1 == 10 && 
            top->rs2 == 10 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test3 (ADD): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test3 (ADD): FAIL" << std::endl;
        }
    }
    
    // 测试用例4: LUI (U类型)
    {
        top->inst = 0x12345037;  // lui x0, 0x12345
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 2 && top->rd == 0 && 
            top->imm_U == 0x12345000 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test4 (LUI): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test4 (LUI): FAIL" << std::endl;
        }
    }
    
    // 测试用例5: LW (I类型)
    {
        top->inst = 0x00452503;  // lw x10, 4(x10)
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 0 && top->rd == 10 && top->rs1 == 10 && 
            top->imm_I == 4 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test5 (LW): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test5 (LW): FAIL" << std::endl;
        }
    }
    
    // 测试用例6: LBU (I类型)
    {
        top->inst = 0x00550503;  // lbu x10, 5(x10)
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 0 && top->rd == 10 && top->rs1 == 10 && 
            top->imm_I == 5 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test6 (LBU): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test6 (LBU): FAIL" << std::endl;
        }
    }
    
    // 测试用例7: SW (S类型)
    {
        top->inst = 0x00A52023;  // sw x10, 0(x10)
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 3 && top->rs1 == 10 && top->rs2 == 10 && 
            top->imm_S == 0 && top->wen_reg == 0 && top->wen_mem == 1) {
            std::cout << "Test7 (SW): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test7 (SW): FAIL" << std::endl;
        }
    }
    
    // 测试用例8: SB (S类型)
    {
        top->inst = 0x00A50023;  // sb x10, 0(x10)
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 3 && top->rs1 == 10 && top->rs2 == 10 && 
            top->imm_S == 0 && top->wen_reg == 0 && top->wen_mem == 1) {
            std::cout << "Test8 (SB): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test8 (SB): FAIL" << std::endl;
        }
    }
    
    // 测试用例9: 负立即数测试 (I类型)
    {
        top->inst = 0xFFF50513;  // addi x10, x10, -1
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 0 && top->rd == 10 && top->rs1 == 10 && 
            top->imm_I == -1 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test9 (Negative Imm): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test9 (Negative Imm): FAIL" << std::endl;
        }
    }
    
    // 测试用例10: 最大正立即数 (I类型)
    {
        top->inst = 0x7FF50513;  // addi x10, x10, 2047
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 0 && top->rd == 10 && top->rs1 == 10 && 
            top->imm_I == 2047 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test10 (Max Positive Imm): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test10 (Max Positive Imm): FAIL" << std::endl;
        }
    }
    
    // 测试用例11: 不同寄存器编号
    {
        top->inst = 0x003081B3;  // add x3, x1, x3
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 1 && top->rd == 3 && top->rs1 == 1 && 
            top->rs2 == 3 && top->wen_reg == 1 && top->wen_mem == 0) {
            std::cout << "Test11 (Different Regs): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test11 (Different Regs): FAIL" << std::endl;
        }
    }
    
    // 测试用例12: 边界值测试 (imm_S最大负值)
    {
        top->inst = 0xFE052E23;  // sw x0, -4(x10)
        top->eval();
        
        test_count++;
        if (top->itype == 1 << 3 && top->rs1 == 10 && top->rs2 == 0 && 
            top->imm_S == -4 && top->wen_reg == 0 && top->wen_mem == 1) {
            std::cout << "Test12 (Negative Store): PASS" << std::endl;
            pass_count++;
        } else {
            std::cout << "Test12 (Negative Store): FAIL" << std::endl;
        }
    }
    
    std::cout << "=== Test Summary: " << pass_count << "/" << test_count << " PASSED ===" << std::endl;
    
    delete top;
    return (pass_count == test_count) ? 0 : 1;
}