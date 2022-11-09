#include "chip8.h"
#include "tinyfiledialogs.h"
#include <time.h>

void render();
const char* selectFile();

void setup(){
    srand(time(NULL)); 
    size(CH8_W*10, CH8_H*10);
    frameRate(60);
    initBuzzer();
    initChip8();
    const char* filename = selectFile();
    if(filename == NULL)
        exit(1);
    loadROM(filename);
    onKeyPressed = setChip8Key;
    onKeyReleased = unsetChip8Key;
    onExit = closeBuzzer;
}

void loop(){
    emulate();
    render();
    memcpy(prev_keyState, keyState, 16);

    if(isKeyReleased && keyReleased == ' '){
        const char* filename = selectFile();
        if(filename != NULL){
            initChip8();
            loadROM(filename);
        }
    }
}

void render(){
    for(int y = 0; y < CH8_H; y++){
        for(int x = 0; x < CH8_W; x++){
            int col = display[x + y*CH8_W] ? color(255, 255, 255) : color(0, 0, 0);
            // x10 upscale
            for(int xx = 0; xx < 10; xx++)
                for(int yy = 0; yy < 10; yy++)
                    pixels[(xx+x*10) + (yy+y*10)*width] = col;
        }
    }
}

const char* selectFile(){
    const char* file_ext[1] = { "*.ch8" };
    return tinyfd_openFileDialog("select ROM", "./", 1, file_ext, NULL, 0);
}