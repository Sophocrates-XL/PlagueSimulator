#pragma once

#include <cstdlib>

#include "Definitions.h"
#include "Classes.h"

#define Concat(str1, str2) str1 ## str2

const char *WIN_TITLE = "PLAGUE SIMULATOR";

// Resource paths.
#define RESOURCE_PATH ".\\Img\\"
const char *PERSON_SUSCEPTIBLE_BMP = Concat(RESOURCE_PATH, "person-susceptible.bmp");
const char *PERSON_INCUBANT_BMP = Concat(RESOURCE_PATH, "person-incubant.bmp");
const char *PERSON_SYMPTOMATIC_BMP = Concat(RESOURCE_PATH, "person-symptomatic.bmp");
const char *PERSON_QUARANTINED_BMP = Concat(RESOURCE_PATH, "person-quarantined.bmp");
const char *PERSON_HOSPITALIZED_BMP = Concat(RESOURCE_PATH, "person-hospitalized.bmp");
const char *PERSON_DEAD_BMP = Concat(RESOURCE_PATH, "person-dead.bmp");
const char *PERSON_RECOVERED_BMP = Concat(RESOURCE_PATH, "person-recovered.bmp");
const char *QUARANTINE_BORDER_BMP = Concat(RESOURCE_PATH, "quarantine-border.bmp");

constexpr uint COLOR_BLACK = 0x000000;
constexpr uint COLOR_ORANGE = 0xFFFF00;
constexpr uint COLOR_GREEN = 0x00FF00;
constexpr uint COLOR_GRAY = 0x808080;

// Tile panel constants.
constexpr uint COLUMN_COUNT = 150;
constexpr uint ROW_COUNT = 150;
constexpr uint TILE_COUNT = ROW_COUNT * COLUMN_COUNT;

// Display constants.
constexpr uint WIN_X = 32;
constexpr uint WIN_Y = 32;
constexpr uint POINT_SPACE = 4;
constexpr uint WIN_W = POINT_SPACE * (COLUMN_COUNT + 1);
constexpr uint WIN_H = POINT_SPACE * (ROW_COUNT + 1);

constexpr uint PERSON_W = 2;
constexpr uint PERSON_H = 2;




// Frame constants.
constexpr uint FPS = 8;
constexpr uint MSEC_PER_FRAME = 1000 / FPS;
constexpr uint MAX_TICK_COUNT = FPS * 600;

// Simulation-related constants.
constexpr uint MIN_POPULATION = 10;
constexpr uint MAX_POPULATION = 3000;
constexpr uint MIN_INFECTION_RATE = 0;
constexpr uint MAX_INFECTION_RATE = 100;
//constexpr int MIN_DUR_INCUBATION = FPS * 1;
//constexpr int MAX_DUR_INCUBATION = FPS * 30;

// Defines various probabilities per time frame / tick in %.
constexpr uint DEFAULT_MANIFESTATION_RATE = 14;
constexpr uint DEFAULT_RECOVERY_RATE = 1;
constexpr uint DEFAULT_DEATH_RATE = 20;
constexpr bool DEFAULT_BOOL_ACQUIRED_IMMUNITY = true;
constexpr uint DEFAULT_DISCOVERY_RATE = 0;
constexpr uint DEFAULT_PATIENT_CAPACITY = 99999;

constexpr uint DEFAULT_MOTION_RATE = 100;
constexpr uint DEFAULT_TELEPORT_RATE = 2;

constexpr bool ALLOWS_DIAGONAL_MOVE = false;

constexpr uint RECOVERY_MULTIPLIER = 3;
constexpr uint DEATH_DIVISOR = 3;


// Draw results.
const char *DRAW_WIN_TITLE = "Result plot";
constexpr uint DRAW_WIN_X = 32;
constexpr uint DRAW_WIN_Y = 32;
constexpr uint DRAW_WIN_W = 640;
constexpr uint DRAW_WIN_H = 480;
constexpr uint PLOT_X_OFFSET = 32;
constexpr uint PLOT_Y_OFFSET = 32;
constexpr uint PLOT_W = DRAW_WIN_W - 2 * PLOT_X_OFFSET;
constexpr uint PLOT_H = DRAW_WIN_H - 2 * PLOT_Y_OFFSET;
