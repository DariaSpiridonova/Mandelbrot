#include "Mandelbrot.h"

extern float minX, maxX, minY, maxY;

void render_mandelbrot(uint32_t* pixels) 
{
    float dx = (maxX - minX) / WIDTH;
    float dy = (maxY - minY) / HEIGHT;
    for (int y = 0; y < HEIGHT; y++) {
        float ci = minY + (float)y * dy;
        for (int x = 0; x < WIDTH; x++) {
            float cr = minX + (float)x * dx;

            float pr = 0, pi = 0;
            int n = 0;

            while (n < MAX_ITER) {
                float pr2 = pr * pr;
                float pi2 = pi * pi;
                if (pr2 + pi2 > 4.0) break;
                pi = 2 * pr * pi + ci;
                pr = pr2 - pi2 + cr;
                n++;
            }

            if (n == MAX_ITER) {
                pixels[y * WIDTH + x] = 0xFF000000; 
            }
            else 
            {
                uint8_t r = (uint8_t)(sin(0.1 * n + 0) * 127 + 128);
                uint8_t g = (uint8_t)(sin(0.1 * n + 2) * 127 + 128);
                uint8_t b = (uint8_t)(sin(0.1 * n + 4) * 127 + 128);
                pixels[y * WIDTH + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
}