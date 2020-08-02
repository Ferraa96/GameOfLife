#include <stdio.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef linux
#include <unistd.h>
#endif

#define row 16
#define col 30
#define ALIVE 1
#define DEAD 0
//the update delay in ms
#define DELAY 200

typedef struct {
    GtkWidget *window, *grid, *b1, *b2;
    GtkWidget *text1, *text2;
} firstScreen_t;

typedef struct {
    GtkWidget *grid;
    bool **oldMap;
    bool **newMap;
    int columns;
    int rows;
} maps_t;

void instanciateMap(maps_t *map) {
    map->oldMap = malloc(map->columns * sizeof(bool **));
    map->newMap = malloc(map->columns * sizeof(bool **));
    for (int i = 0; i < map->columns; i++) {
        map->oldMap[i] = malloc(map->rows * sizeof(bool *));
        map->newMap[i] = malloc(map->rows * sizeof(bool *));
    }
}

/**
 * creates the [SIZE_X] x [SIZE_Y] map
*/
void createRandomMap(maps_t *maps) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (rand() % 2 == 0) {
                maps->oldMap[i][j] = false;
            }
            else {
                maps->oldMap[i][j] = true;
                gtk_widget_set_name(GTK_WIDGET(gtk_grid_get_child_at(GTK_GRID(maps->grid), j, i)), "black");
            }
        }
    }
}

void deserializeMap(maps_t *maps, char *fileName) {
    FILE *file = fopen(fileName, "r");
    char alive;
    if (file == NULL) {
        printf("An error occurred while opening the file\n");
        exit(0);
    }
    fscanf(file, "%d", &maps -> rows);
    fseek(file, 1, SEEK_CUR);
    fscanf(file, "%d", &maps -> columns);
    maps->oldMap = malloc(maps -> columns * sizeof(bool**));
    maps->newMap = malloc(maps -> columns * sizeof(bool**));
    fseek(file, 2, SEEK_CUR);
    for (int i = 0; i < maps -> columns; i++) {
        maps->oldMap[i] = malloc(maps -> rows * sizeof(bool*));
        maps->newMap[i] = malloc(maps -> rows * sizeof(bool*));
    }
    for (int i = 0; i < maps -> rows; i++) {
        for(int j = 0; j < maps -> columns; j++) {
            fscanf(file, "%c", &alive);
            if (alive - 48 == ALIVE) {
                maps->oldMap[i][j] = true;
            }
            else {
                maps -> oldMap[i][j] = false;
            }
        }
        fseek(file, 2, SEEK_CUR);
    }
    fclose(file);
}

/**
 * frees all the structures
*/
void freeMap(GtkWidget *widget, gpointer data) {
    maps_t *gameMaps = (maps_t*) data;
    for(int i = 0; i < gameMaps -> columns; i++) {
        free(gameMaps -> newMap[i]);
        free(gameMaps -> oldMap[i]);
    }
    free(gameMaps -> newMap);
    free(gameMaps -> oldMap);
    free(gameMaps);
    gtk_main_quit();
    exit(0);
}

int countNear(maps_t *gameMap, int r, int c) {
    int num = 0;
    int minRow = r - 1, minCol = c - 1, maxRow = r + 2, maxCol = c + 2;
    if (minRow < 0) {
        minRow = 0;
    }
    else if (maxRow > gameMap->rows) {
        maxRow = gameMap->rows;
    }
    if (minCol < 0) {
        minCol = 0;
    }
    else if (maxCol > gameMap->columns) {
        maxCol = gameMap->columns;
    }
    for (int i = minRow; i < maxRow; i++) {
        for (int j = minCol; j < maxCol; j++) {
            num += gameMap->oldMap[i][j];
        }
    }
    return num - gameMap->oldMap[r][c];
}

void myCSS(void) {
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    const gchar *myCssFile = "style.css";
    GError *error = 0;

    gtk_css_provider_load_from_file(provider, g_file_new_for_path(myCssFile), &error);
    g_object_unref(provider);
}

gint gameLogic(void *mapp) {
    maps_t *map = (maps_t *)mapp;
    int num;
    boolean alive = false;
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->columns; j++) {
            num = countNear(map, i, j);
            if (map->oldMap[i][j]) {
                if (num < 2) {
                    map->newMap[i][j] = false;
                } else if (num > 1) {
                    map->newMap[i][j] = true;
                }
                if (num > 3) {
                    map->newMap[i][j] = false;
                }
            } else {
                if (num == 3) {
                    map->newMap[i][j] = true;
                }
                else {
                    map->newMap[i][j] = false;
                }
            }
            if (map->newMap[i][j]) {
                alive = true;
                gtk_widget_set_name(GTK_WIDGET(gtk_grid_get_child_at(GTK_GRID(map->grid), j, i)), "black");
            } else {
                gtk_widget_set_name(GTK_WIDGET(gtk_grid_get_child_at(GTK_GRID(map->grid), j, i)), "white");
            }
        }
    }
    bool **temp = map->oldMap;
    map->oldMap = map->newMap;
    map->newMap = temp;
    if (alive) {
        return 1;
    }
    else {
        return 0;
    }
}

void destroyUselessWidget(firstScreen_t *fs) {
    gtk_widget_destroy(fs -> b1);
    gtk_widget_destroy(fs -> b2);
    gtk_widget_destroy(fs -> text1);
    gtk_widget_destroy(fs -> text2);
}

void chooseFile(GtkWidget *widget, gpointer data) {
    GtkWidget *window, ***pos;
    firstScreen_t *fs = (firstScreen_t*) data;
    maps_t *map = malloc(sizeof(maps_t));

    destroyUselessWidget(fs);
    
    char *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
    window = fs -> window;
    g_signal_connect(window, "destroy", G_CALLBACK(freeMap), map);
    map -> grid = fs -> grid;
    
    deserializeMap(map, fileName);
    pos = malloc(map -> columns * sizeof(GtkWidget**));
    for (int i = 0; i < map -> columns; i++) {
        pos[i] = malloc(map -> rows * sizeof(GtkWidget*));
    }
    for (int i = 0; i < map -> rows; i++) {
        for (int j = 0; j < map -> columns; j++) {
            pos[j][i] = gtk_button_new();
            gtk_grid_attach(GTK_GRID(map -> grid), pos[j][i], j, i, 1, 1);
            if(map -> oldMap[i][j]) {
                gtk_widget_set_name(GTK_WIDGET(gtk_grid_get_child_at(GTK_GRID(map -> grid), j, i)), "black");
            }
        }
    }
    gtk_widget_show_all(window);
    g_timeout_add(DELAY, gameLogic, map);
    gtk_main();
}

void playWithRandomMap(GtkWidget *widget, gpointer data) {
    GtkWidget *window, ***pos;
    firstScreen_t *fs = (firstScreen_t*) data;
    maps_t *map = malloc(sizeof(maps_t));

    destroyUselessWidget(fs);
    
    window = fs -> window;
    g_signal_connect(window, "destroy", G_CALLBACK(freeMap), map);
    map -> grid = fs -> grid;

    map -> columns = col;
    map -> rows = row;
    instanciateMap(map);
    pos = malloc(col * sizeof(GtkWidget**));
    for (int i = 0; i < col; i++) {
        pos[i] = malloc(row * sizeof(GtkWidget*));
    }
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            pos[j][i] = gtk_button_new();
            gtk_grid_attach(GTK_GRID(map -> grid), pos[j][i], j, i, 1, 1);
        }
    }
    createRandomMap(map);
    gtk_widget_show_all(window);
    g_timeout_add(DELAY, gameLogic, map);
    gtk_main();
}

int main(int argc, char *argv[]) {
    firstScreen_t *firstScreen = malloc(sizeof(firstScreen_t));
    gtk_init(&argc, &argv);
    myCSS();
    firstScreen -> window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    firstScreen -> grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(firstScreen -> window), firstScreen -> grid);
    gtk_window_set_title(GTK_WINDOW(firstScreen -> window), "The game of life");
    g_signal_connect(firstScreen -> window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    firstScreen -> b1 = gtk_button_new_with_label("RANDOM");
    firstScreen -> b2 = gtk_file_chooser_button_new("Choose map", GTK_FILE_CHOOSER_ACTION_OPEN);
    firstScreen -> text1 = gtk_label_new("Generate random map");
    firstScreen -> text2 = gtk_label_new("Or input a map file");

    gtk_widget_set_name(firstScreen -> b1, "button");
    gtk_widget_set_name(firstScreen -> b2, "button");
    gtk_widget_set_name(firstScreen -> text1, "text");
    gtk_widget_set_name(firstScreen -> text2, "text");

    gtk_grid_attach(GTK_GRID(firstScreen -> grid), firstScreen -> text1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(firstScreen -> grid), firstScreen -> b1, 0, 1, 1, 1);
    g_signal_connect (firstScreen -> b1, "clicked", G_CALLBACK (playWithRandomMap), firstScreen);
    gtk_grid_attach(GTK_GRID(firstScreen -> grid), firstScreen -> text2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(firstScreen -> grid), firstScreen -> b2, 0, 3, 1, 1);
    g_signal_connect (firstScreen -> b2, "file-set", G_CALLBACK (chooseFile), firstScreen);
    gtk_widget_show_all(firstScreen -> window);
    gtk_main();
    return 0;
}