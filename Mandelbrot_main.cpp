#include "Mandelbrot.h"

// Viewing area (changes when zoomed in)
float minX = -2.0, maxX = 1.0;
float minY = -1.2, maxY = 1.2;

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
                float zoomFactor = (e.wheel.y > 0) ? 0.9 : 1.1;
                
                // Центрируем зум на мышке
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                float mouseR = minX + (float)mx * (maxX - minX) / WIDTH;
                float mouseI = minY + (float)my * (maxY - minY) / HEIGHT;

                minX = mouseR + (minX - mouseR) * zoomFactor;
                maxX = mouseR + (maxX - mouseR) * zoomFactor;
                minY = mouseI + (minY - mouseI) * zoomFactor;
                maxY = mouseI + (maxY - mouseI) * zoomFactor;
                redraw = 1;
            }
        }

        void* rawPixels;
        int pitch;

        // 1. "Запираем" текстуру. SDL дает нам прямой адрес своего внутреннего буфера
        if (SDL_LockTexture(texture, NULL, &rawPixels, &pitch) == 0) 
        {
            uint32_t* pixels = (uint32_t*)rawPixels;
            
            unsigned int aux; 

            unsigned long long t1 = __rdtsc(); 
            render_mandelbrot(pixels);
            unsigned long long t2 = __rdtscp(&aux); 

            unsigned long long cycles = t2 - t1;

            // Display in the window title how many milliseconds the frame took
            char title[50];
            snprintf(title, sizeof(title), "Mandelbrot - cycles: %llu", cycles);
            
            SDL_SetWindowTitle(window, title);

            // 3. "Отпираем". В этот момент данные ОДНИМ махом улетают в VRAM
            SDL_UnlockTexture(texture);
        }

        SDL_RenderClear(renderer);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    return 0;
}
