#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <ctype.h>

// 指令编码函数声明
uint32_t encode_addi(uint8_t rd, uint8_t rs1, int16_t imm);
uint32_t encode_jalr(uint8_t rd, uint8_t rs1, int16_t imm);
uint32_t encode_lui(uint8_t rd, int32_t imm);
uint32_t encode_add(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t encode_sb(uint8_t rs1, uint8_t rs2, int16_t imm);
uint32_t encode_sw(uint8_t rs1, uint8_t rs2, int16_t imm);
uint32_t encode_lbu(uint8_t rd, uint8_t rs1, int16_t imm);
uint32_t encode_lw(uint8_t rd, uint8_t rs1, int16_t imm);

int main() {
    DIR *dir;
    struct dirent *ent;
    char *test_dir = "test/";
    
    if ((dir = opendir(test_dir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char *filename = ent->d_name;
            if (strstr(filename, ".txt")) {
                // 构建完整文件路径
                char input_path[256];
                char output_path[256];
                snprintf(input_path, sizeof(input_path), "%s%s", test_dir, filename);
                snprintf(output_path, sizeof(output_path), "%s%.*s.bin", 
                         test_dir, (int)(strlen(filename) - 4), filename);
                
                // 打开输入输出文件
                FILE *in_file = fopen(input_path, "r");
                FILE *out_file = fopen(output_path, "wb");
                
                if (!in_file || !out_file) {
                    perror("文件打开错误");
                    continue;
                }
                
                char line[256];
                while (fgets(line, sizeof(line), in_file)) {
                    // 移除行尾换行符
                    size_t len = strlen(line);
                    if (len > 0 && line[len-1] == '\n') {
                        line[len-1] = '\0';
                    }
                    
                    // 跳过空行和注释
                    if (line[0] == '\0' || line[0] == '#') {
                        continue;
                    }
                    
                    // 转换为小写
                    for (int i = 0; line[i]; i++) {
                        line[i] = tolower(line[i]);
                    }
                    // 解析指令
                    char *token = strtok(line, " ");
                    if (!token) continue;  // 空行
                    
                    // 获取操作码
                    char opcode[16];
                    strcpy(opcode, token);
                    
                    uint8_t rd = 0, rs1 = 0, rs2 = 0;
                    int32_t imm = 0;
                    
                    // 解析操作数
                    token = strtok(NULL, ", ");
                    if (token && token[0] == 'x') {
                        rd = strtol(token+1, NULL, 10);
                    }
                    
                    // 对于lui指令，只有一个目标寄存器和立即数
                    if (strcmp(opcode, "lui") == 0) {
                        token = strtok(NULL, ", ");
                        if (token) {
                            imm = strtol(token, NULL, 10);
                        }
                    }

                    token = strtok(NULL, ", ");
                    if (token && token[0] == 'x') {
                        rs1 = strtol(token+1, NULL, 10);
                    }
                    
                    token = strtok(NULL, ", ");
                    if (token) {
                        // 检查是否是寄存器
                        if (token[0] == 'x') {
                            rs2 = strtol(token+1, NULL, 10);
                        } else {
                            // 否则是立即数
                            imm = strtol(token, NULL, 10);
                        }
                    }
                    
                    
                    
                    // 编码指令
                    uint32_t instruction = 0;
                    if (strcmp(opcode, "addi") == 0) {
                        instruction = encode_addi(rd, rs1, imm);
                    } else if (strcmp(opcode, "jalr") == 0) {
                        instruction = encode_jalr(rd, rs1, imm);
                    } else if (strcmp(opcode, "lui") == 0) {
                        instruction = encode_lui(rd, imm);
                    } else if (strcmp(opcode, "add") == 0) {
                        instruction = encode_add(rd, rs1, rs2);
                    } else if (strcmp(opcode, "sb") == 0) {
                        instruction = encode_sb(rs1, rd, imm);
                    } else if (strcmp(opcode, "sw") == 0) {
                        instruction = encode_sw(rs1, rd, imm);
                    } else if (strcmp(opcode, "lbu") == 0) {
                        instruction = encode_lbu(rd, rs1, imm);
                    } else if (strcmp(opcode, "lw") == 0) {
                        instruction = encode_lw(rd, rs1, imm);
                    } else {
                        fprintf(stderr, "未知指令: %s\n", opcode);
                        continue;
                    }
                    
                    // 以小端序写入
                    uint8_t bytes[4];
                    bytes[0] = instruction & 0xFF;
                    bytes[1] = (instruction >> 8) & 0xFF;
                    bytes[2] = (instruction >> 16) & 0xFF;
                    bytes[3] = (instruction >> 24) & 0xFF;
                    fwrite(bytes, 1, 4, out_file);
                }
                
                fclose(in_file);
                fclose(out_file);
            }
        }
        closedir(dir);
    } else {
        perror("无法打开test目录");
        return EXIT_FAILURE;
    }
    
    return 0;
}

// 指令编码函数实现
// 指令编码函数实现（修正版）
uint32_t encode_addi(uint8_t rd, uint8_t rs1, int16_t imm) {
    return (0b0010011) | (rd << 7) | (0b000 << 12) | (rs1 << 15) | ((imm & 0xFFF) << 20);
}

uint32_t encode_jalr(uint8_t rd, uint8_t rs1, int16_t imm) {
    return (0b1100111) | (rd << 7) | (0b000 << 12) | (rs1 << 15) | ((imm & 0xFFF) << 20);
}

uint32_t encode_lui(uint8_t rd, int32_t imm) {
    // 取高20位立即数
    return (0b0110111) | (rd << 7) | ((imm & 0xFFFFF) << 12);
}

uint32_t encode_add(uint8_t rd, uint8_t rs1, uint8_t rs2) {
    return (0b0110011) | (rd << 7) | (0b000 << 12) | (rs1 << 15) | (rs2 << 20) | (0b0000000 << 25);
}

uint32_t encode_sb(uint8_t rs1, uint8_t rs2, int16_t imm) {
    uint32_t imm_low = imm & 0x1F;
    uint32_t imm_high = (imm >> 5) & 0x7F;
    return (0b0100011) | (imm_low << 7) | (0b000 << 12) | (rs1 << 15) | (rs2 << 20) | (imm_high << 25);
}

uint32_t encode_sw(uint8_t rs1, uint8_t rs2, int16_t imm) {
    uint32_t imm_low = imm & 0x1F;
    uint32_t imm_high = (imm >> 5) & 0x7F;
    return (0b0100011) | (imm_low << 7) | (0b010 << 12) | (rs1 << 15) | (rs2 << 20) | (imm_high << 25);
}

uint32_t encode_lbu(uint8_t rd, uint8_t rs1, int16_t imm) {
    return (0b0000011) | (rd << 7) | (0b100 << 12) | (rs1 << 15) | ((imm & 0xFFF) << 20);
}

uint32_t encode_lw(uint8_t rd, uint8_t rs1, int16_t imm) {
    return (0b0000011) | (rd << 7) | (0b010 << 12) | (rs1 << 15) | ((imm & 0xFFF) << 20);
}
