#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"



#define WIDTH 100
#define HEIGHT 100
#define CELL_SIZE 10
#define FRAME_TARGET 20


typedef struct ColorNode {
    struct ColorNode* next;
    Color* color;
} ColorNode;

typedef struct Gridpoint{
    bool collapsed;
    ColorNode* stateSet;
    int stateCount;
} Gridpoint;

typedef Gridpoint Grid;

Color colors[] = {WHITE, RED, GREEN, BLUE};



ColorNode* createSet(Color *colorArray, size_t colors_amount) {
    ColorNode* prevNode = NULL;
    while(colors_amount--){
        ColorNode* newNode = (ColorNode*)malloc(sizeof(ColorNode));
        newNode->color = &colorArray[colors_amount];
        newNode->next = prevNode;
        prevNode = newNode;
    }
    return prevNode;
}


typedef struct Candidates {
    Gridpoint* candidate;
    struct Candidates* next;
} Candidates;

void addCandidatePoint(Candidates **candidates, Gridpoint *gridpoint) {
    Candidates* newNode = (Candidates*)malloc(sizeof(Candidates));
    newNode->candidate = gridpoint;
    newNode->next = NULL;

    if (*candidates == NULL) {
        *candidates = newNode;
    } else {
        Candidates* current = *candidates;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}


void collapse(Grid grid[HEIGHT][WIDTH],int x, int y) {
    Gridpoint* gridpoint = &grid[y][x];
    if(!gridpoint->collapsed){
        int stateNumber = rand() % gridpoint->stateCount;
        ColorNode* currentNode = gridpoint->stateSet;
        while(stateNumber--){
            currentNode = currentNode->next;
        }
        gridpoint->stateSet = currentNode;
        gridpoint->stateCount = 1;
        gridpoint->collapsed = true;
    }
}

int main(void){
    InitWindow(WIDTH * CELL_SIZE, HEIGHT*CELL_SIZE, "Wave collapse");
    srand(time(NULL));
    SetTraceLogLevel(LOG_INFO);
    SetTargetFPS(FRAME_TARGET);

    Grid grid[HEIGHT][WIDTH];

    for (int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++){
            int color_amount = sizeof(colors) / sizeof(colors[0]);
            grid[y][x].stateCount = color_amount;
            grid[y][x].stateSet = createSet(colors, color_amount);
            grid[y][x].collapsed = false;
        }
    }

    //Initial collapse
    collapse(grid,rand() % HEIGHT,rand() % WIDTH);

    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Candidates* candidates = NULL;

        int c = 0;
        //Collapse
        for (int y = 0; y < HEIGHT; y++) {
            for(int x = 0; x < WIDTH; x++){
                if(!grid[y][x].collapsed){
                    continue;
                }
                TraceLog(LOG_DEBUG,"Checking (%d,%d)",x,y);
                for(int dy = -1; dy < 2; dy++) {
                    for (int dx = -1; dx < 2; dx++) {
                        int offsetY = y + dy;
                        int offsetX = x + dx;
                        // Check if the new position is within the bounds of the array
                        if (offsetY >= 0 && offsetY < HEIGHT && offsetX >= 0 && offsetX < WIDTH && -(dy != 0 || dx != 0)) {
                            Gridpoint* candidatePoint = &grid[offsetY][offsetX];
                            if(!candidatePoint->collapsed){
                                TraceLog(LOG_DEBUG,"Adding candidate at (%d,%d)",offsetX,offsetY);
                                addCandidatePoint(&candidates, candidatePoint);
                                c++;
                            }
                        }
                    }
                }
            }
        }

        TraceLog(LOG_DEBUG,"%d",c);

        Candidates* current = candidates;
        while(current->next != NULL){

            current->candidate->collapsed = true;
            current = current->next;
        }

        //Draw grid
        for (int y = 0; y < HEIGHT; y++) {
            for(int x = 0; x < WIDTH; x++){
                //collapse(grid, x, y);
                Gridpoint point = grid[y][x];
                if(point.collapsed){
                    TraceLog(LOG_DEBUG,"Collapsed at (%d,%d)",x,y);
                    DrawRectangle(x*CELL_SIZE,y*CELL_SIZE, CELL_SIZE,CELL_SIZE, *point.stateSet->color);
                }
            }
        }


        EndDrawing();
    }
    CloseWindow();
    return 0;
}




