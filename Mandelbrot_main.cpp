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

    SDL_Window* window = SDL_CreateWindow("Mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    
    int quit = 0;
    SDL_Event e;
    int redraw = 1;

    uint32_t last_update_time = SDL_GetTicks();
    unsigned long long total_cycles = 0;
    int frame_count = 0;

    while (!quit) 
    {
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

            uint32_t start_time = SDL_GetTicks(); 
            unsigned long long t1 = __rdtsc(); 

            render_mandelbrot(pixels);

            unsigned long long t2 = __rdtscp(&aux); 
            uint32_t end_time = SDL_GetTicks();  
            
            unsigned long long cycles = t2 - t1;
            
            // Display in the window title fps and number of processor cycles
            unsigned long long current_cycles = t2 - t1;
            total_cycles += current_cycles; // Накапливаем такты
            frame_count++;

            uint32_t current_time = SDL_GetTicks();
            if (current_time - last_update_time >= 3000) // Прошло 3 секунды
            {
                unsigned long long avg_cycles = total_cycles / frame_count; // Среднее за 3 сек
                
                char title[100];
                snprintf(title, sizeof(title), "Mandelbrot | Avg Cycles: %llu | FPS: %.1f", 
                        avg_cycles, (double)frame_count / 3.0);
                
                SDL_SetWindowTitle(window, title);
                
                // Сбрасываем счетчики для следующего интервала
                total_cycles = 0;
                frame_count = 0;
                last_update_time = current_time;
            }

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
