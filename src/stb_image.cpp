/**
 * @file stb_image.cpp
 * @brief STB image library implementation file.
 *
 * This file serves as the implementation point for the STB image library.
 * By defining STB_IMAGE_IMPLEMENTATION before including the header, it
 * tells the STB library to include all the function implementations
 * in this translation unit.
 *
 * STB_image is a public domain image loading library that supports:
 * - JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock
 * IJG lib)
 * - PNG 1/2/4/8/16-bit-per-channel
 * - TGA (not sure what subset, if a subset)
 * - BMP non-1bpp, non-RLE
 * - PSD (composited view only, no extra channels, 8/16 bit-per-channel)
 * - GIF (*comp always reports as 4-channel)
 * - HDR (radiance rgbE format)
 * - PIC (Softimage PIC)
 * - PNM (PPM and PGM binary only)
 *
 * The library provides simple functions for loading images into memory
 * as arrays of pixels, which is perfect for OpenGL texture creation.
 */

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
