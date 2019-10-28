#include "displayWaves.h"

void printWave(enum WaveType waveType) {
    GrContextForegroundSet(&sContext, ClrWhite);
    switch (waveType) {
        case SINE:
            displaySineWave();
            break;
        
        case SQUARE:
            displaySquareWave();
            break;
        
        case TRIANGLE:
            displayTriangleWave();
            break;

        case TRAPEZOID:
            displayTrapezoidWave();
            break;

        case SAWTOOTH:
            displaySawtoothWave();
            break;
    }
}

void displaySineWave() {
	int i;
	float j, sin_j;
	
	for(i = 0, j = 0.0; i < 100; i++, j += 2*PI / 100.0) {
		sin_j = sin(j);
		GrPixelDraw(&sContext, 14 + i, 59 - (sin_j * 25.0));
	}
}


void displaySquareWave() {
    int i = 14, j = 84, k;

    for(k = 0; k < 4; k++) {
        // Desenha primeira linha vertical
        for(j = 52; j < 84; j++) {
            GrPixelDraw(&sContext, i, j);
        }

        // Desenha primeira linha horizontal
        j = 52;
        for(i = 14 + k*25; i < 26 + k*25; i++) {
            GrPixelDraw(&sContext, i, j);
        }

        // Desenha segunda linha vertical
        for(j = 52; j < 84; j++) {
            GrPixelDraw(&sContext, i, j);
        }

        // Desenha segunda linha horizontal
        j = 84;
        for(i = 26 + k*25 + 1; i < 38 + k*25; i++) {
            GrPixelDraw(&sContext, i, j);
        }
    }
}

void displayTriangleWave() {
	int i, j, k;
	
	for(k = 0; k < 4; k++) {
		// Desenha subida
		for(i = 0; i < 12; i++)
			GrPixelDraw(&sContext, i + 24*k + 14, 84 - i);
		
		// Desenha descida
		for(i = 0; i < 12; i++)
			GrPixelDraw(&sContext, i + 24*k + 26, 72 + i);
	}
}

void displaySawtoothWave() {
	int i, j, k;
	
	for(k = 0; k < 4; k++) {
		// Desenha subida
		for(i = 0; i < 24; i++)
			GrPixelDraw(&sContext, i + 24*k + 14, 84 - i);
		
		// Desenha reta vertical
		for(j = 0; j < 25; j++)
			GrPixelDraw(&sContext, i + 24*k + 14, 84 - j);
	}
}

void displayTrapezoidWave() {
	int i, j, k;
	
	for(k = 0; k < 2; k++) {
		// Desenha subida
		for(i = 0; i < 15; i++)
			GrPixelDraw(&sContext, i + 50*k + 14, 84 - i);
		
		// Desenha primeira linha reta
		for(i = 0; i < 10; i++)
			GrPixelDraw(&sContext, i + 50*k + 29, 70);
		
		// Desenha descida
		for(i = 0; i < 15; i++)
			GrPixelDraw(&sContext, i + 50*k + 39, 70 + i);
		
		// Desenha segunda linha reta
		for(i = 0; i < 10; i++)
			GrPixelDraw(&sContext, i + 50*k + 54, 84);
	}
}

void clearDisplayWave() {
    int i, j;

    GrContextForegroundSet(&sContext, ClrBlack);
    for(i = 0; i < 128; i++) {
        for(j = 20; j < 85; j++)
            GrPixelDraw(&sContext, i, j);
    }
}