#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_ITER = 256;

// Область просмотра (изменяется при зуме)
double minX = -2.0, maxX = 1.0;
double minY = -1.2, maxY = 1.2;

void render_mandelbrot(uint32_t* pixels) 
{
    double dx = (maxX - minX) / WIDTH;
    double dy = (maxY - minY) / HEIGHT;
    for (int y = 0; y < HEIGHT; y++) {
        double ci = minY + (double)y * dy;
        for (int x = 0; x < WIDTH; x++) {
            double cr = minX + (double)x * dx;

            double pr = 0, pi = 0;
            int n = 0;

            while (n < MAX_ITER) {
                double pr2 = pr * pr;
                double pi2 = pi * pi;
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

int main(int argc, char* argv[]) 
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
    {
        printf("SDL error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Mandelbrot Zoom (Mouse Wheel)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    
    int quit = 0;
    SDL_Event e;
    int redraw = 1;

    while (!quit) {
        while (SDL_PollEvent(&e)) 
        {
            if (e.type == SDL_QUIT) quit = 1;
            
            if (e.type == SDL_MOUSEWHEEL) 
            {
                double zoomFactor = (e.wheel.y > 0) ? 0.9 : 1.1;
                
                // Центрируем зум на мышке
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                double mouseR = minX + (double)mx * (maxX - minX) / WIDTH;
                double mouseI = minY + (double)my * (maxY - minY) / HEIGHT;

                minX = mouseR + (minX - mouseR) * zoomFactor;
                maxX = mouseR + (maxX - mouseR) * zoomFactor;
                minY = mouseI + (minY - mouseI) * zoomFactor;
                maxY = mouseI + (maxY - mouseI) * zoomFactor;
                redraw = 1;
            }
        }

        if (redraw) 
        {
            void* rawPixels;
            int pitch;

            // 1. "Запираем" текстуру. SDL дает нам прямой адрес своего внутреннего буфера
            if (SDL_LockTexture(texture, NULL, &rawPixels, &pitch) == 0) 
            {
                uint32_t* pixels = (uint32_t*)rawPixels;
                
                uint32_t start_time = SDL_GetTicks(); 
            
                // 2. Считаем фрактал СРАЗУ в этот буфер
                render_mandelbrot(pixels); 

                uint32_t end_time = SDL_GetTicks();   // Засекаем время ПОСЛЕ
                uint32_t duration = end_time - start_time;

                // Выводим в заголовок окна, сколько миллисекунд занял кадр
                char title[50];
                snprintf(title, sizeof(title), "Mandelbrot - Time: %u ms", duration);
                SDL_SetWindowTitle(window, title);

                // 3. "Отпираем". В этот момент данные ОДНИМ махом улетают в VRAM
                SDL_UnlockTexture(texture);
            }

            // redraw = 1; // will start rendering at the event if redraw = 0
        }

        SDL_RenderClear(renderer);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    return 0;
}
