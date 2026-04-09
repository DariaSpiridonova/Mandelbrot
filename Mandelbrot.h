#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <SDL2/SDL.h>
#include <immintrin.h>
#include <stdio.h>

const int32_t WIDTH = 800;
const int32_t HEIGHT = 600;
const int32_t MAX_ITER = 256;

const __m256 r2_ps    = _mm256_set1_ps(4.0f);
const __m256 zero_ps = _mm256_set1_ps(0.0f);  
const __m256i v_one_i = _mm256_set1_epi32(1);

const __m256 x_steps = _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f);

void render_mandelbrot(uint32_t* pixels);

#endif

