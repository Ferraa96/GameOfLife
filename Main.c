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

#define row 30
#define col 60
#define ALIVE 1
#define DEAD 0
//the update delay in ms
#define DELAY 100

typedef struct {
    GtkWidget *window, *grid, *b1, *b2;
    GtkWidget *text1, *text2;
} display_t;

typedef struct {
    GtkWidget *grid, ***pos;
    bool **initialMap;
    bool **oldMap;
    bool **newMap;
    int columns;
    int rows;
    guint loopID;
    boolean running;
} maps_t;

/**
 * instanciate all the structures for the map
*/
void instanciateMap(maps_t *map) {
    map -> initialMap = malloc(map -> columns * sizeof(bool **));
    map -> oldMap = malloc(map -> columns * sizeof(bool **));
    map -> newMap = malloc(map -> columns * sizeof(bool **));
    for (int i = 0; i < map -> columns; i++) {
        map -> initialMap[i] = malloc(map -> rows * sizeof(bool *));
        map -> oldMap[i] = malloc(map -> rows * sizeof(bool *));
        map -> newMap[i] = malloc(map -> rows * sizeof(bool *));
    }
}

/**
 * creates the row x col map
*/
void createRandomMap(maps_t *maps) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (rand() % 2 == 0) {
                maps -> initialMap[i][j] = false;
                maps -> oldMap[i][j] = false;
            } else {
                maps -> initialMap[i][j] = true;
                maps -> oldMap[i][j] = true;
            }
        }
    }
}

/**
 * deserialize the map file in *filename path
*/
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

    instanciateMap(maps);
    fseek(file, 2, SEEK_CUR);

    for (int i = 0; i < maps -> rows; i++) {
        for(int j = 0; j < maps -> columns; j++) {
            fscanf(file, "%c", &alive);
            if (alive - 48 == ALIVE) {
                maps -> initialMap[i][j] = true;
                maps -> oldMap[i][j] = true;
            } else {
                maps -> initialMap[i][j] = false;
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

    gameMaps -> running = false;
    if(gameMaps -> loopID > 0) {
        g_source_remove(gameMaps -> loopID);
    }
    for(int i = 0; i < gameMaps -> columns; i++) {
        free(gameMaps -> newMap[i]);
        free(gameMaps -> oldMap[i]);
        free(gameMaps -> initialMap[i]);
    }
    free(gameMaps -> newMap);
    free(gameMaps -> oldMap);
    free(gameMaps -> initialMap);
    free(gameMaps);
    gtk_main_quit();
    exit(0);
}

/**
 * counts the neighbors of the cell in position [r, c]
*/
int countNear(maps_t *gameMap, int r, int c) {
    int num = 0;
    int minRow = r - 1, minCol = c - 1, maxRow = r + 2, maxCol = c + 2;
    if (minRow < 0) {
        minRow = 0;
    } else if (maxRow > gameMap -> rows) {
        maxRow = gameMap -> rows;
    }
    if (minCol < 0) {
        minCol = 0;
    } else if (maxCol > gameMap -> columns) {
        maxCol = gameMap -> columns;
    }
    for (int i = minRow; i < maxRow; i++) {
        for (int j = minCol; j < maxCol; j++) {
            num += gameMap -> oldMap[i][j];
        }
    }
    return num - gameMap -> oldMap[r][c];
}

/**
 * use the CSS file to load the graphic configurations
*/
void myCSS(void) {
    GtkCssProvider *provider;
    GdkDisplay *display_t;
    GdkScreen *screen;

    provider = gtk_css_provider_new();
    display_t = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display_t);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    const gchar *myCssFile = "style.css";
    GError *error = 0;

    gtk_css_provider_load_from_file(provider, g_file_new_for_path(myCssFile), &error);
    g_object_unref(provider);
}

/**
 * implements the game rules
*/
gint gameLogic(void *mapp) {
    maps_t *map = (maps_t *)mapp;
    int num;
    boolean alive = false;
    for (int i = 0; i < map -> rows; i++) {
        for (int j = 0; j < map -> columns; j++) {
            num = countNear(map, i, j);
            if (map -> oldMap[i][j]) {
                if (num < 2) {
                    map -> newMap[i][j] = false;
                } else if (num > 1) {
                    map -> newMap[i][j] = true;
                }
                if (num > 3) {
                    map -> newMap[i][j] = false;
                }
            } else {
                if (num == 3) {
                    map -> newMap[i][j] = true;
                } else {
                    map -> newMap[i][j] = false;
                }
            }
            if (map -> newMap[i][j]) {
                alive = true;
                gtk_widget_set_name(GTK_WIDGET(map -> pos[j][i]), "black");
            } else {
                gtk_widget_set_name(GTK_WIDGET(map -> pos[j][i]), "white");
            }
        }
    }
    bool **temp = map -> oldMap;
    map -> oldMap = map -> newMap;
    map -> newMap = temp;
    if (alive && map -> running) {
        return 1;
    } else {
        map -> loopID = -2;
        return 0;
    }
}

/**
 * destroys unused widget after a button is clicked
*/
void destroyUselessWidget(display_t *fs) {
    gtk_widget_destroy(fs -> b1);
    gtk_widget_destroy(fs -> b2);
    gtk_widget_destroy(fs -> text1);
    gtk_widget_destroy(fs -> text2);
}

void run(GtkWidget *widget, gpointer data) {
    maps_t *map = (maps_t*) data;
    if(!map -> running) {
        map -> running = true;
        map -> loopID = g_timeout_add(DELAY, gameLogic, map);
    }
}

void stop(GtkWidget *widget, gpointer data) {
    maps_t *map = (maps_t*) data;
    if(map -> running) {
        map -> running = false;
        map -> loopID = -2;
    }
}

void modifySize(GtkWidget *widget, gpointer data) {
    maps_t *map = (maps_t*) data;
    int size;
    gtk_widget_get_size_request(map -> pos[0][0], &size, &size);
    if(strcmp((gtk_widget_get_name(widget)), "sizeUp") == 0) {
        size++;
    } else {
        if(size == 16) {
            return;
        }
        size--;
    }
    for(int i = 0; i < map -> rows; i++) {
        for(int j = 0; j < map -> columns; j++) {
            gtk_widget_set_size_request(map -> pos[j][i], size, size);
        }
    }
}

void reinitiateMap(GtkWidget *widget, gpointer data) {
    maps_t *map = (maps_t*) data;

    if(map -> loopID == -1) {                               //mai avviato
        return;
    }
    if(map -> loopID != -2) {                               //avviato e fermato
        g_source_remove(map -> loopID);
    }
    map -> running = false;
    map -> loopID = -1;
    for(int i = 0; i < map -> rows; i++) {
        for(int j = 0; j < map -> columns; j++) {
            if(map -> initialMap[i][j]) {
                map -> oldMap[i][j] = true;
                gtk_widget_set_name(GTK_WIDGET(map -> pos[j][i]), "black");
            } else {
                map -> oldMap[i][j] = false;
                gtk_widget_set_name(GTK_WIDGET(map -> pos[j][i]), "white");
            }
        }
    }
}

void setGameWindow(maps_t *map, display_t *display) {
    GtkWidget *play = gtk_button_new(), *pause = gtk_button_new(), *sizeUp = gtk_button_new(), *sizeDown = gtk_button_new();
    GtkWidget *redo = gtk_button_new();
    GtkWidget *playImage, *pauseImage, *sizeUpImage, *sizeDownImage, *redoImage;
    
    destroyUselessWidget(display);

    g_signal_connect(display -> window, "destroy", G_CALLBACK(freeMap), map);
    map -> grid = display -> grid;
    map -> loopID = -1;

    map -> pos = malloc(map -> columns * sizeof(GtkWidget**));
    for (int i = 0; i < map -> columns; i++) {
        map -> pos[i] = malloc(map -> rows * sizeof(GtkWidget*));
    }
    for (int i = 0; i < map -> rows; i++) {
        for (int j = 0; j < map -> columns; j++) {
            map -> pos[j][i] = gtk_image_new();
            gtk_widget_set_size_request(map -> pos[j][i], 16, 16);
            gtk_grid_attach(GTK_GRID(map -> grid), map -> pos[j][i], j, i, 1, 1);
            if(map -> oldMap[i][j]) {
                gtk_widget_set_name(GTK_WIDGET(map -> pos[j][i]), "black");
            }
        }
    }
    gtk_grid_attach(GTK_GRID(map -> grid), play, 0, row, 4, 1);
    gtk_grid_attach(GTK_GRID(map -> grid), pause, 4, row, 4, 1);
    gtk_grid_attach(GTK_GRID(map -> grid), sizeUp, col - 2, row, 2, 1);
    gtk_grid_attach(GTK_GRID(map -> grid), sizeDown, col - 4, row, 2, 1);
    gtk_grid_attach(GTK_GRID(map -> grid), redo, 8, row, 4, 1);

    g_signal_connect (play, "clicked", G_CALLBACK (run), map);
    g_signal_connect (pause, "clicked", G_CALLBACK (stop), map);
    g_signal_connect (sizeUp, "clicked", G_CALLBACK (modifySize), map);
    g_signal_connect (sizeDown, "clicked", G_CALLBACK (modifySize), map);
    g_signal_connect (redo, "clicked", G_CALLBACK (reinitiateMap), map);

    playImage = gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    pauseImage = gtk_image_new_from_icon_name("media-playback-pause", GTK_ICON_SIZE_BUTTON);
    redoImage = gtk_image_new_from_icon_name("edit-undo", GTK_ICON_SIZE_BUTTON);
    sizeUpImage = gtk_image_new_from_icon_name("list-add", GTK_ICON_SIZE_BUTTON);
    sizeDownImage = gtk_image_new_from_icon_name("list-remove", GTK_ICON_SIZE_BUTTON);

    gtk_button_set_image(GTK_BUTTON(play), playImage);
    gtk_button_set_image(GTK_BUTTON(pause), pauseImage);
    gtk_button_set_image(GTK_BUTTON(redo), redoImage);
    gtk_button_set_image(GTK_BUTTON(sizeUp), sizeUpImage);
    gtk_button_set_image(GTK_BUTTON(sizeDown), sizeDownImage);
    gtk_widget_set_name(GTK_WIDGET(play), "play");
    gtk_widget_set_name(GTK_WIDGET(pause), "pause");
    gtk_widget_set_name(GTK_WIDGET(redo), "redo");
    gtk_widget_set_name(GTK_WIDGET(sizeUp), "sizeUp");
    gtk_widget_set_name(GTK_WIDGET(sizeDown), "sizeDown");
    
    gtk_widget_show_all(display -> window);
    gtk_main();
}

/**
 * creates a map using an existent file
*/
void chooseFile(GtkWidget *widget, gpointer data) {
    maps_t *map = malloc(sizeof(maps_t));
    
    char *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
    deserializeMap(map, fileName);
    
    setGameWindow(map, (display_t*) data);
}

/**
 * creates a random map of size row x col
*/
void playWithRandomMap(GtkWidget *widget, gpointer data) {
    maps_t *map = malloc(sizeof(maps_t));

    map -> columns = col;
    map -> rows = row;
    instanciateMap(map);
    createRandomMap(map);

    setGameWindow(map, (display_t*) data);
}

int main(int argc, char *argv[]) {
    display_t *display = malloc(sizeof(display_t));

    gtk_init(&argc, &argv);
    myCSS();
    display -> window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(GTK_WIDGET(display -> window), "background");
    display -> grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(display -> window), display -> grid);
    gtk_window_set_title(GTK_WINDOW(display -> window), "The game of life");
    g_signal_connect(display -> window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_resizable(GTK_WINDOW(display -> window), false);

    display -> b1 = gtk_button_new_with_label("RANDOM");
    display -> b2 = gtk_file_chooser_button_new("Choose map", GTK_FILE_CHOOSER_ACTION_OPEN);
    display -> text1 = gtk_label_new("Generate random map");
    display -> text2 = gtk_label_new("Or input a map file");

    gtk_widget_set_name(display -> b1, "button");
    gtk_widget_set_name(display -> b2, "button");
    gtk_widget_set_name(display -> text1, "text");
    gtk_widget_set_name(display -> text2, "text");

    gtk_grid_attach(GTK_GRID(display -> grid), display -> text1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(display -> grid), display -> b1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(display -> grid), display -> text2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(display -> grid), display -> b2, 0, 3, 1, 1);

    g_signal_connect (display -> b1, "clicked", G_CALLBACK (playWithRandomMap), display);
    g_signal_connect (display -> b2, "file-set", G_CALLBACK (chooseFile), display);

    gtk_widget_show_all(display -> window);
    gtk_main();
    return 0;
}