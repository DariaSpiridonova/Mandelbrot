#include "Mandelbrot.h"

extern float minX, maxX, minY, maxY;

// main function for calculations
void render_mandelbrot(uint32_t* pixels) 
{
    float dx = (maxX - minX) / WIDTH;
    float dy = (maxY - minY) / HEIGHT;

    __m256 dx_vec   = _mm256_set1_ps(dx);

    #pragma omp parallel for
    for (ssize_t y = 0; y < HEIGHT; y++) {
        float y0 = (float)minY + (float)y * dy;
        __m256 y_ar = _mm256_set1_ps(y0);
        
        for (ssize_t x = 0; x < WIDTH; x += 8) // Step = 8
        { 
            float x0_f = (float)minX + (float)x * dx;
            __m256 x0_f_vec = _mm256_set1_ps(x0_f);

            __m256 x_ar = _mm256_fmadd_ps(x_steps, dx_vec, x0_f_vec);

            __m256i cmp = _mm256_setzero_si256();  
            __m256 pr  = zero_ps;
            __m256 pi  = zero_ps;
            __m256i N = _mm256_setzero_si256();           

            ssize_t n = 0;
            while (n < MAX_ITER) 
            {
                __m256 pr2 = _mm256_mul_ps(pr, pr);
                __m256 pi2 = _mm256_mul_ps(pi, pi);
                
                __m256 sum_pr2_pi2 = _mm256_add_ps(pr2, pi2); 
                
                __m256 mask = _mm256_cmp_ps(sum_pr2_pi2, r2_ps, _CMP_LE_OQ);
                __m256i mask_i = _mm256_castps_si256(mask);

                cmp = _mm256_and_si256(mask_i, v_one_i);

                int m = _mm256_movemask_ps(mask);
                if (m == 0) break; 

                pi = _mm256_fmadd_ps(_mm256_add_ps(pr, pr), pi, y_ar);
                pr = _mm256_add_ps(_mm256_fmsub_ps(pr, pr, pi2), x_ar);
                
                N = _mm256_add_epi32(N, cmp);
                n++;
            }

            int32_t n_buf[8]; 
            _mm256_storeu_si256((__m256i_u *)n_buf, N);
            
            for (ssize_t i = 0; i < 8; i++)
            {
                if (n_buf[i] == MAX_ITER) 
                {
                    pixels[y * WIDTH + x + i] = 0xFF000000; 
                } 
                else 
                {
                    uint8_t r = (uint8_t)(sinf(0.1f * (float)n_buf[i] + 0) * 127 + 128);
                    uint8_t g = (uint8_t)(sinf(0.1f * (float)n_buf[i] + 2) * 127 + 128);
                    uint8_t b = (uint8_t)(sinf(0.1f * (float)n_buf[i] + 4) * 127 + 128);
                    pixels[y * WIDTH + x + i] = (0xFFU << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
                }
            }
        }
    }
}