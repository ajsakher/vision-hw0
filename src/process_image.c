#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

int validate_coordinate(int x, int valid_x) {
    if (x < 0) {
        return 0;
    }
    if (x >= valid_x) {
        return valid_x - 1;
    }
    return x;
}

float get_pixel(image im, int x, int y, int c)
{
    // map coordinates to their appropriate clamp over/under flow values
    x = validate_coordinate(x, im.w);
    y = validate_coordinate(y, im.h);
    c = validate_coordinate(c, im.c);

    return im.data[x + y*im.w + c*im.w*im.h];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    int test_x = validate_coordinate(x, im.w);
    int test_y = validate_coordinate(y, im.h);
    int test_c = validate_coordinate(c, im.c);

    if (
        test_x != x
        || test_y != y
        || test_c != c
    ) {
        return;
    }

    im.data[x + y*im.w + c*im.w*im.h] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    
    for (int x = 0; x < im.w*im.h*im.c; x++) {
        copy.data[x] = im.data[x];
    }
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    for (int x = 0; x < im.w*im.h; x++) {
        gray.data[x] = 0.299*im.data[x] + 0.587*im.data[x + im.w*im.h] + 0.114*im.data[x + 2*im.w*im.h];
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    for (int i = c*im.w*im.h; i < (c + 1)*im.w*im.h; i++) {
        im.data[i] += v;
    }
}

void clamp_image(image im)
{
    for (int x = 0; x < im.w*im.h*im.c; x++) {
        if (im.data[x] > 1) {
            im.data[x] = 1;
        } else if (im.data[x] < 0) {
            im.data[x] = 0;
        }
    }
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

float calculate_hue(float r, float g, float b, float c, float v) {
    if (c == 0) {
        return 0;
    }

    float h_prime;
    if (v == r) {
        h_prime = (g - b) / c;
    } else if (v == g) {
        h_prime = ((b - r) / c) + 2;
    } else {
        h_prime = ((r - g) / c) + 4;
    }

    float h = h_prime / 6;
    if (h_prime < 0) {
        h += 1;
    }

    return h;
}

void rgb_to_hsv(image im)
{
    for (int x = 0; x < im.w*im.h; x++) {
        float r = im.data[x];
        float g = im.data[x + im.w*im.h];
        float b = im.data[x + 2 * im.w*im.h];

        float V = three_way_max(r, g, b);
        
        float m = three_way_min(r, g, b);
        float C = V - m;
        float S = V == 0 ? 0 : C / V;

        float H = calculate_hue(r, g, b, C, V);

        im.data[x] = H;
        im.data[x + im.w*im.h] = S;
        im.data[x + 2 * im.w*im.h] = V;
    }
}

void hsv_to_rgb(image im)
{
    for (int x = 0; x < im.w*im.h; x++) {
        float H = im.data[x];
        float S = im.data[x + im.w*im.h];
        float V = im.data[x + 2*im.w*im.h];
        float C = S*V;

        if (C == 0) {
            im.data[x] = V;
            im.data[x + im.w*im.h] = V;
            im.data[x + 2*im.w*im.h] = V;
            continue;
        }

        float r, g, b;

        float m = (1 - S)*V;
        float h_prime = H*6;

        if (h_prime >= 5.5) {
            // V == r, H' < 1
            h_prime = (H - 1)*6;

        }

        if (h_prime >= 3) {
            // V == b
            b = V;
            float r_min_g = (h_prime - 4)*C;
            if (r_min_g <= 0) {
                r = m;
                g = r - r_min_g;
            } else {
                g = m;
                r = g + r_min_g;
            }
        } else if (h_prime >= 1) {
            // V == g
            g = V;
            float b_min_r = (h_prime - 2)*C;
            if (b_min_r <= 0) {
                b = m;
                r = b - b_min_r;
            } else {
                r = m;
                b = r + b_min_r;
            }
        } else {
            // V == r
            r = V;
            float g_min_b = h_prime*C;
            if (g_min_b <= 0) {
                g = m;
                b = g - g_min_b;
            } else {
                b = m;
                g = b + g_min_b;
            }
        }

        im.data[x] = r;
        im.data[x + im.w*im.h] = g;
        im.data[x + 2*im.w*im.h] = b;
    }
}