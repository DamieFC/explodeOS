#ifndef PROMPT_H
#define PROMPT_H

#include<stdio.h>

char prompt[] = "explodeOS>";
char input[];

void psPrompt(void);

void psPrompt(void){
    printf("%s", prompt);
    gets("%s", input);
}
