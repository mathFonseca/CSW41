#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include <math.h>
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"

#define PI 3.141592

extern tContext sContext;

enum WaveType {
    SQUARE,
    SINE,
    TRIANGLE,
    SAWTOOTH,
    TRAPEZOID
};

// Function to identify wave type
void printWave(enum WaveType waveType);

// Functions that write a specified wave on the screen
void displaySineWave();
void displaySquareWave();
void displayTriangleWave();
void displaySawtoothWave();
void displayTrapezoidWave();

// Utility functions
void clearDisplayWave();