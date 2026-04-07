#include <SDL2/SDL.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

const ssize_t WIDTH = 800;
const ssize_t HEIGHT = 600;
const ssize_t MAX_ITER = 256;

// Область просмотра (изменяется при зуме)
double minX = -2.0, maxX = 1.0;
double minY = -1.2, maxY = 1.2;
   
const __m256d r2 = _mm256_set1_pd(4.0);
const __m256d v_one = _mm256_set1_pd(1.0);
const __m256d v_two = _mm256_set1_pd(2.0);  

void render_mandelbrot(uint32_t* pixels) 
{
    double dx = (maxX - minX) / WIDTH;
    double dy = (maxY - minY) / HEIGHT;
    for (ssize_t y = 0; y < HEIGHT; y++) {
        double y0 = minY + (double)y * dy;
        __m256d y_ar = _mm256_set1_pd(y0);
        for (ssize_t x = 0; x < WIDTH; x += 4) {
            double x0 = minX + (double)x*dx;
            __m256d x_ar = _mm256_set_pd(x0 + dx*3, x0 + dx*2, x0 + dx, x0);

            __m256d cmp   = _mm256_setzero_pd(); // flags buffer: 1 - if the dot did not eliminat, otherwise 0
            
            __m256d pr = _mm256_setzero_pd();
            __m256d pi = _mm256_setzero_pd();
            // double pr = 0, pi = 0;

            ssize_t n = 0;
            __m256d N = _mm256_setzero_pd(); // array - iteration counter
            // int n = 0;

            while (n < MAX_ITER) {
                __m256d pr2 = _mm256_mul_pd(pr, pr);
                __m256d pi2 = _mm256_mul_pd(pi, pi);
                // double pi2 = pi * pi;
                // double pr2 = pr * pr;
                
                
                // Have all the dots been eliminated?

                __m256d sum_pr2_pi2 = _mm256_add_pd(pr2, pi2); 
                // pr2 + pi2 
                
                __m256d mask = _mm256_cmp_pd(sum_pr2_pi2, r2, _CMP_LE_OQ);
                cmp  = _mm256_and_pd(mask, v_one);

                int m = _mm256_movemask_pd(mask);

                if (m == 0) // if (pr2 + pi2 > 4.0)
                {
                    break; 
                }

                pi = _mm256_add_pd(_mm256_mul_pd(v_two, _mm256_mul_pd(pr, pi)), y_ar);
                // pi = 2 * pr * pi + y0;
                
                pr = _mm256_add_pd(_mm256_sub_pd(pr2, pi2), x_ar);
                // pr = pr2 - pi2 + x0;

                N = _mm256_add_pd(N, cmp);
                n++;
            }

            double n_buf[4];
            _mm256_storeu_pd(n_buf, N);
            
            for (ssize_t i = 0; i < 4; i++)
            {
                if (n_buf[i] == (double)MAX_ITER) {
                    pixels[y * WIDTH + x + i] = 0xFF000000; 
                } 
    
                else 
                {
                    uint8_t r = (uint8_t)(sin(0.1 * n_buf[i] + 0) * 127 + 128);
                    uint8_t g = (uint8_t)(sin(0.1 * n_buf[i] + 2) * 127 + 128);
                    uint8_t b = (uint8_t)(sin(0.1 * n_buf[i] + 4) * 127 + 128);
                    pixels[y * WIDTH + x + i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
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
