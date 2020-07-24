#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<malloc.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef linux
#include <unistd.h>
#endif
#include<math.h>

#define ALIVE 1
#define DEAD 0
#define SIZE_X 30
#define SIZE_Y 90
//the update delay in ms
#define DELAY 1000

typedef struct {
    char **oldMap;
    char **newMap;
    int size_x;
    int size_y;
} maps_t;

/**
 * creates the [SIZE_X] x [SIZE_Y] map
*/
void createRandomMap(maps_t *maps) {
    maps -> oldMap = malloc(sizeof(char**));
    maps -> newMap = malloc(sizeof(char**));
    maps -> oldMap = malloc(SIZE_X * sizeof(char**));
    maps -> newMap = malloc(SIZE_X * sizeof(char**));
    maps -> size_x = SIZE_X;
    maps -> size_y = SIZE_Y;
    for(int i = 0; i < SIZE_X; i++) {
        maps -> oldMap[i] = malloc(SIZE_Y * sizeof(char*));
        maps -> newMap[i] = malloc(SIZE_Y * sizeof(char*));
        for(int j = 0; j < SIZE_Y; j++) {
            int random = rand() % 2;
            if(random == 0) {
                maps -> oldMap[i][j] = DEAD;
            } else {
                maps -> oldMap[i][j] = ALIVE;
            }
        }
    }
}

/**
 * deserialize an existing map, formatted as
 * row column
 * 011100101110100
 * 101010110101101
 * ...
*/
void deserializeMap(maps_t *maps, char *fileName) {
    FILE *file = fopen(fileName, "r");
    int sizeX, sizeY;
    char alive;
    if(file == NULL) {
        printf("An error occurred while opening the file\n");
        exit(0);
    }
    fscanf(file, "%d", &sizeX);
    fseek(file, 1, SEEK_CUR);
    fscanf(file, "%d", &sizeY);
    maps -> oldMap = malloc(sizeof(char**));
    maps -> newMap = malloc(sizeof(char**));
    maps -> oldMap = malloc(sizeX * sizeof(char**));
    maps -> newMap = malloc(sizeX * sizeof(char**));
    maps -> size_x = sizeX;
    maps -> size_y = sizeY;
    fseek(file, 2, SEEK_CUR);
    for(int i = 0; i < sizeX; i++) {
        maps -> oldMap[i] = malloc(sizeY * sizeof(char*));
        maps -> newMap[i] = malloc(sizeY * sizeof(char*));
        for(int j = 0; j < sizeY; j++) {
            fscanf(file, "%c", &alive);
            if(alive - 48 == ALIVE) {
                maps -> oldMap[i][j] = ALIVE;
            } else {
                maps -> oldMap[i][j] = DEAD;
            }
        }
        fseek(file, 2, SEEK_CUR);
    }
    fclose(file);
}

/**
 * frees all the structures
*/
void freeMap(maps_t *gameMaps) {
    for(int i = 0; i < gameMaps -> size_x; i++) {
        free(gameMaps -> oldMap[i]);
        free(gameMaps -> newMap[i]);
    }
    free(gameMaps -> oldMap);
    free(gameMaps -> newMap);
    free(gameMaps);
}

/**
 * render the map
*/
void printMap(maps_t *gameMap) {
    for(int i = 0; i < gameMap -> size_y + 2; i++) {
        printf("-");
    }
    printf("\n");
    for(int i = 0; i < gameMap -> size_x; i++) {
        printf("|");
        for(int j = 0; j < gameMap -> size_y; j++) {
            if(gameMap -> oldMap[i][j] == 1) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("|\n");
    }
    for(int i = 0; i < gameMap -> size_y + 2; i++) {
        printf("-");
    }
    printf("\n");
}

/**
 * counts the neighbors entities
*/
int countNear(maps_t *gameMap, int row, int column) {
    int num = 0;
    int minRow = row - 1, minCol = column - 1, maxRow = row + 2, maxCol = column + 2;
    if(minRow < 0 ) {
        minRow = 0;
    } else if(maxRow > gameMap -> size_x) {
        maxRow = gameMap -> size_x;
    }
    if(minCol < 0 ) {
        minCol = 0;
    } else if(maxCol > gameMap -> size_y) {
        maxCol = gameMap -> size_y;
    }
    for(int i = minRow; i < maxRow; i++) {
        for(int j = minCol; j < maxCol; j++){
            num += gameMap -> oldMap[i][j] == ALIVE;
        }
    }
    return num - gameMap -> oldMap[row][column];
}

/**
 * implements the game logic
*/
void game(maps_t *maps) {
    int num;
    for(int i = 0; i < maps -> size_x; i++) {
        for(int j = 0; j < maps -> size_y; j++) {
            num = countNear(maps, i, j);
            if(maps -> oldMap[i][j] == ALIVE) {
                if(num < 2) {
                    maps -> newMap[i][j] = DEAD;
                } else if(num > 1) {
                    maps -> newMap[i][j] = ALIVE;
                }
                if(num > 3) {
                    maps -> newMap[i][j] = DEAD;
                }
            } else {
                if(num == 3) {
                    maps -> newMap[i][j] = ALIVE;
                } else {
                    maps -> newMap[i][j] = DEAD;
                }
            }
        }
    }
    char **temp = maps -> oldMap;
    maps -> oldMap = maps -> newMap;
    maps -> newMap = temp;
}

int main() {
    maps_t *maps = malloc(sizeof(maps_t));
    char ris[100];
    printf("Insert file name or \"random\": ");
    scanf("%s", ris);
    if(strcmp(ris, "random") == 0) {
        createRandomMap(maps);
    } else {
        deserializeMap(maps, ris);
    }
    while(1) {
        printMap(maps);
        game(maps);
        #ifdef _WIN32
        Sleep(DELAY);
        #endif
        #ifdef linux
        sleep(DELAY / 1000);
        #endif
    }
    freeMap(maps);
    return 0;
}