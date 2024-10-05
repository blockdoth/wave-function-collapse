#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include "hashmap.h"


#define WIDTH 100
#define HEIGHT 100
#define CELL_SIZE 10
#define FRAME_TARGET 20

#define GRID_CHECKS { \
    {-1, -1},{0, -1},{1, -1}, \
    {-1, 0},        {1, 0}, \
    {-1, 1}, {0, 1}, {1, 1} \
}; \

#define PLUS_CHECKS { \
            {0, -1}, \
    {-1, 0},        {1, 0}, \
            {0, 1} \
}; \



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

//Color colors[] = { RED, GREEN, BLUE, YELLOW, ORANGE, PINK, PURPLE, BROWN, BEIGE, LIME, GOLD, SKYBLUE, DARKGREEN, VIOLET, DARKBROWN, DARKGRAY, BLACK};
Color base_colors[] = { BLACK, DARKGRAY};



ColorNode* create_color_set(Color *colorArray) {
    ColorNode* prevNode = NULL;
    int colors_amount = sizeof(base_colors) / sizeof(base_colors[0]);
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


typedef struct ValidState {
    int x;
    int y;
    ColorNode* valid;
    struct ValidState* next;
} ValidState;



void add_valid_state(ValidState **list, int x, int y, Color colors[]) {
    ValidState* valid_state = (ValidState*) malloc(sizeof(ValidState));
    valid_state->x = x;
    valid_state->y = y;
    valid_state->next = NULL;
    valid_state->valid = create_color_set(colors);

    if(*list == NULL){
        *list = valid_state;
    }else{
        ValidState* current = *list;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = valid_state;
    }
}

char* convert_color_key(Color color_key) {
    char* key = (char*) malloc(sizeof(char*) * 64); // Assuming a reasonable buffer size

    // Format the string
    snprintf(key, 16, "%u-%u-%u-%u", color_key.r, color_key.g, color_key.b, color_key.a);


    return key;
}


HashMap *initStatesMap() {
    HashMap* hm = create_hashmap(1000);
    ValidState *black_state = NULL;
    add_valid_state(&black_state, 1, 1, (Color[]) {BLACK, WHITE});


    add_valid_state(&black_state, 0, 1, (Color[]) {BLACK, WHITE});



    insert_data(hm,convert_color_key(BLACK), black_state);
}


void init_grid(Grid grid[HEIGHT][WIDTH]){
    for (int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++){
            grid[y][x].stateCount = sizeof(base_colors) / sizeof(base_colors[0]);
            grid[y][x].stateSet = create_color_set(base_colors);
            grid[y][x].collapsed = false;
        }
    }
}


void debug_states(ColorNode *set) {
    char buffer[1024];
    strcpy(buffer,"Set contains:\t[ ");

    ColorNode* cur = set;
    while(cur->next != NULL){
        strcat(buffer, convert_color_key(*cur->color));
        strcat(buffer, " ");
        cur = cur->next;
    }
    strcat(buffer, " ]");
    TraceLog(LOG_DEBUG,buffer);
}

void remove_invalid_states(ColorNode *state_set, ColorNode *valid_state_set) {
    ColorNode* main_set = state_set;
    ColorNode* valid_set = valid_state_set;


    debug_states(main_set);
    debug_states(valid_set);


    ColorNode* prev_node = NULL;
    while(main_set != NULL && main_set->next != NULL){
        bool not_found = true;
        while(valid_set->next != NULL){
            if(main_set->color == valid_set->color){
                not_found = false;
            }
            valid_set = valid_set->next;
        }
        //Reset inner loop
        valid_set = valid_state_set;
        if(not_found){
            if(prev_node == NULL){
                ColorNode* temp = main_set;
                main_set = main_set->next;
                free(temp);
            }else{
                prev_node->next = main_set->next;
                ColorNode* next = main_set->next;
                free(main_set);
                main_set = next;
                continue;
            }
        }

        prev_node = main_set;
        main_set = main_set->next;

    }


}


int main(void){
    SetTraceLogLevel(LOG_DEBUG);
    InitWindow(WIDTH * CELL_SIZE, HEIGHT*CELL_SIZE, "Wave collapse");
    srand(time(NULL));
    SetTargetFPS(FRAME_TARGET);

    HashMap* validStatesMap = initStatesMap();


    Grid grid[HEIGHT][WIDTH];
    init_grid(grid);

    //Initial collapse
    collapse(grid,rand() % HEIGHT,rand() % WIDTH);

    while (!WindowShouldClose()){

        BeginDrawing();
        ClearBackground(RAYWHITE);

        HashMap* hm = initStatesMap();

        //Candidates* candidates = NULL;
        //Collapse
        for (int y = 0; y < HEIGHT; y++) {
            for(int x = 0; x < WIDTH; x++){
                Gridpoint gridpoint = grid[y][x];
                if(!gridpoint.collapsed){
                    continue;
                }
                TraceLog(LOG_DEBUG,"Collapsed (%d,%d)",x,y);

                ValidState* valid_states = get_data(validStatesMap, convert_color_key(*gridpoint.stateSet->color));
                if (valid_states != NULL){
                    ValidState* current = valid_states;
                    while(current != NULL){
                        int offsetX = x + current->x;
                        int offsetY = y + current->y;
                        if (offsetY >= 0 && offsetY < HEIGHT && offsetX >= 0 && offsetX < WIDTH && -(current->y != 0 || current->x != 0)) {
                            Gridpoint* candidatePoint = &grid[offsetY][offsetX];
                            TraceLog(LOG_DEBUG,"Checking (%d,%d)",offsetX,offsetY);
                            DrawRectangle(offsetX * CELL_SIZE,offsetY * CELL_SIZE,CELL_SIZE,CELL_SIZE, BLUE);
                            remove_invalid_states(candidatePoint->stateSet, current->valid);
                        }
                        current = current->next;
                    }

                }

            }
        }

        //Draw grid
        for (int y = 0; y < HEIGHT; y++) {
            for(int x = 0; x < WIDTH; x++){
                //collapse(grid, x, y);
                Gridpoint point = grid[y][x];
                if(point.collapsed){
                    TraceLog(LOG_DEBUG,"Collapsed at (%d,%d)",x,y);
                    Color color;
                    if(point.stateCount > 1){
                        color = GRAY;
                    }else{
                        color = *point.stateSet->color;
                    }
                    DrawRectangle(x*CELL_SIZE,y*CELL_SIZE, CELL_SIZE,CELL_SIZE,color);
                }
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}


