#include "chip8.h"

uint8_t memory[MEMORY_SIZE];
bool display[CH8_W*CH8_H];

uint16_t PC;
uint16_t I;

uint8_t V[16];
uint8_t ST;
uint8_t DT;

uint16_t stack[STACKSIZE];
size_t stackIndex;

bool keyState[16];
bool prev_keyState[16];

void initChip8(){
    memset(memory, 0, MEMORY_SIZE);
    memset(V, 0, sizeof(uint8_t)*16);
    memset(display, 0, CH8_W*CH8_H);
    memset(stack, 0, sizeof(uint16_t)*STACKSIZE);

    PC = 0x200;
    I = 0;
    ST = 0;
    DT = 0;
    stackIndex = -1;

    uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    memcpy(memory+0x50, font, sizeof(font)/sizeof(uint8_t));
}

void loadROM(const char* filename){
    FILE* fptr = fopen(filename, "rb");
    fseek(fptr, 0, SEEK_END);
    unsigned int filesize = ftell(fptr);
    rewind(fptr);
    fread(memory+0x200, 1, filesize, fptr);
    fclose(fptr);
}

void cpuTimers(){
    if(ST != 0) ST--;
    if(DT != 0) DT--;
    
    if(ST != 0)
        playBeep();
}

void RunOpcode(){
    uint16_t opcode = (memory[PC] << 8) | memory[PC+1];
    uint8_t first = (opcode & 0xF000) >> 12;
    uint8_t second = (opcode & 0xF00) >> 8;
    uint8_t third = (opcode & 0xF0) >> 4;
    uint8_t fourth = opcode & 0xF;
    
    if(first == 0x0){
        if(second == 0x0){
            if(third == 0xE){
                if(fourth == 0x0){
                    //00E0
                    //CLS
                    for(int i = 0; i < CH8_W*CH8_H; i++)
                        display[i] = false;
                }
                if(fourth == 0xE){
                    //00EE
                    //RET
                    PC = popStack();
                }
            }
        }
    }
    if(first == 0x1){
        //1NNN
        //JUMP
        PC = opcode & 0xFFF;
        return;
    }
    if(first == 0x2){
        //2NNN
        //CALL addr
        pushStack(PC);
        PC = opcode & 0xFFF;
        return;
    }
    if(first == 0x3){
        //3XNN
        //SE Vx, byte
        if(V[second] == (opcode & 0xFF)) PC += 2;
    }
    if(first == 0x4){
        //4XNN
        //SNE Vx, byte
        if(V[second] != (opcode & 0xFF)) PC += 2;
    }
    if(first == 0x5){
        if(fourth == 0x0){
            //5XY0
            //SE Vx, Vy
            if(V[second] == V[third]) PC += 2;
        }
    }
    if(first == 0x6){
        //6XNN
        V[second] = opcode & 0xFF;
    }
    if(first == 0x7){
        //7XNN
        V[second] += (opcode & 0xFF);
    }
    if(first == 0x8){
        if(fourth == 0x0){
            //8XY0
            V[second] = V[third];
        }
        if(fourth == 0x1){
            //8XY1
            V[second] |= V[third];
        }
        if(fourth == 0x2){
            //8XY2
            V[second] &= V[third];
        }
        if(fourth == 0x3){
            //8XY3
            V[second] ^= V[third];
        }
        if(fourth == 0x4){
            //8XY4
            bool flag = false;
            if((V[second])+(V[third]) > 255)
                flag = true;
            V[second] += V[third];
            V[0xF] = flag;
        }
        if(fourth == 0x5){
            //8XY5
            bool flag = false;
            if(V[second] > V[third])
                flag = true;
            V[second] -= V[third];
            V[0xF] = flag;
        }
        if(fourth == 0x6){
            //8XY6
            bool flag = false;
            if(V[second] & 0x01)
                flag = true;
            V[second] = V[second] >> 1;
            V[0xF] = flag;
        }
        if(fourth == 0x7){
            //8XY7
            bool flag = false;
            if(V[third] > V[second])
                flag = true;
            V[second] = V[third] - V[second];
            V[0xF] = flag;
        }
        if(fourth == 0xE){
            //8XYE
            bool flag = false;
            if(V[second] & 0x80)
                flag = true;
            V[second] = V[second] << 1;
            V[0xF] = flag;
        }
    }
    if(first == 0x9){
        if(fourth == 0x0){
            //9XY0
            //SNE Vx, Vy
            if(V[second] != V[third]) PC += 2;
        }
    }
    if(first == 0xA){
        //ANN
        I = opcode & 0xFFF;
    }
    if(first == 0xB){
        //BNN
        PC = (opcode & 0xFFF) + V[0x0];
    }
    if(first == 0xC){
        V[second] = (rand()%256) & (opcode & 0xFF);
    }
    if(first == 0xD){
        //DXYN
        int vx = V[second]%CH8_W;
        int vy = V[third]%CH8_H;
        V[0xF] = 0;
        for(int y = 0; y < fourth; y++){
            int row = memory[I+y];
            for(int x = 0; x < 8; x++){
                int offset = vx+x+(vy+y)*CH8_W;
                if(vy+y >= CH8_H || vx+x >= CH8_W) break;
                bool value = (row & (1 << (7-x))) >> (7-x);  
                if(display[offset] && value)
                    V[0xF] = 1;
                display[offset] ^= value;
            }
        }
    }
    if(first == 0xE){
        if(third == 0x9){
            if(fourth == 0xE){
                //EX95
                if(keyState[V[second]]) PC += 2;
            }
        }
        if(third == 0xA){
            if(fourth == 0x1){
                //EXA1
                if(!keyState[V[second]]) PC += 2;
            }
        }
    }
    if(first == 0xF){
        if(third == 0x0){
            if(fourth == 0x7){
                //FX07
                V[second] = DT;
            }
            if(fourth == 0xA){
                //FX0A
                bool detectKeyPress = false;
                for(int i = 0; i < 16; i++)
                    if(!prev_keyState[i] && keyState[i]){
                        V[second] = i;
                        detectKeyPress = true;
                        break;
                    }
                if(!detectKeyPress)
                    return;
            }
        }
        if(third == 0x1){
            if(fourth == 0x5){
                //FX15
                DT = V[second];
            }
            if(fourth == 0x8){
                ST = V[second];
            }
            if(fourth == 0xE){
                //FX1E
                I += V[second];
            }
        }
        if(third == 0x2){
            if(fourth == 0x9){
                //FX29
                I = V[second]*5 + 0x50;
            }
        }
        if(third == 0x3){
            if(fourth == 0x3){
                //FX33
                memory[I] = ( V[second] / 100);
                memory[I+1] = ( (V[second] / 10) % 10);
                memory[I+2] = ( V[second] % 10);
            }
        }
        if(third == 0x5){
            if(fourth == 0x5){
                //FX55
                for(int i = 0; i <= second; i++)
                    memory[I+i] = V[i];
            }
        }
        if(third == 0x6){
            if(fourth == 0x5){
                //FX65
                for(int i = 0; i <= second; i++)
                    V[i] = memory[I+i];
            }
        }
    }
    
    PC += 2;
}

void pushStack(int val){
  stack[++stackIndex] = val;
}

int popStack(){
  return stack[stackIndex--];
}

void emulate(){
    for(int i = 0; i < HERTZ/frameRate; i++)
      RunOpcode();
    cpuTimers();
}


void setChip8Key(keyboard k){
    if(k == '1')
        keyState[0x1] = true;
    if(k == '2')
        keyState[0x2] = true;
    if(k == '3')
        keyState[0x3] = true;
    if(k == '4')
        keyState[0xC] = true;
    if(k == 'q')
        keyState[0x4] = true;
    if(k == 'w')
        keyState[0x5] = true;
    if(k == 'e')
        keyState[0x6] = true;
    if(k == 'r')
        keyState[0xD] = true;
    if(k == 'a')
        keyState[0x7] = true;
    if(k == 's')
        keyState[0x8] = true;
    if(k == 'd')
        keyState[0x9] = true;
    if(k == 'f')
        keyState[0xE] = true;
    if(k == 'z')
        keyState[0xA] = true;
    if(k == 'x')
        keyState[0x0] = true;
    if(k == 'c')
        keyState[0xB] = true;
    if(k == 'v')
        keyState[0xF] = true;
}

void unsetChip8Key(keyboard k){
    if(k == '1')
        keyState[0x1] = false;
    if(k == '2')
        keyState[0x2] = false;
    if(k == '3')
        keyState[0x3] = false;
    if(k == '4')
        keyState[0xC] = false;
    if(k == 'q')
        keyState[0x4] = false;
    if(k == 'w')
        keyState[0x5] = false;
    if(k == 'e')
        keyState[0x6] = false;
    if(k == 'r')
        keyState[0xD] = false;
    if(k == 'a')
        keyState[0x7] = false;
    if(k == 's')
        keyState[0x8] = false;
    if(k == 'd')
        keyState[0x9] = false;
    if(k == 'f')
        keyState[0xE] = false;
    if(k == 'z')
        keyState[0xA] = false;
    if(k == 'x')
        keyState[0x0] = false;
    if(k == 'c')
        keyState[0xB] = false;
    if(k == 'v')
        keyState[0xF] = false;
}