#define SAVE_SCREEN
#define ENABLE_ERROR_RETURN
 
#define USE_TI89
#define MIN_AMS 200
 
#include <tigcclib.h>
#include "extgraph.h"
 
#include <stdlib.h>
#include <stdio.h>
 
#define true 1
#define false 0
 
#define KEY_PLUS 43
#define KEY_MINUS 45
#define KEY_T 116
#define KEY_SHIFT_T 84
 
/*
Mandafractal Ti89 version 1.1
 
by Ben Jones
*/
 
typedef unsigned char bool;
 
struct fixed{
    long int value;
};
 
typedef struct fixed fixed;
 
fixed add_fixed(fixed a, fixed b){
    return (fixed) {.value = a.value + b.value};
}
 
fixed subtract_fixed(fixed a, fixed b){
    return (fixed) {.value = a.value - b.value};
}
 
fixed multiply_fixed(fixed a, fixed b){
    return (fixed) {.value = (a.value>>14)*(b.value>>14)};
}
 
fixed float_to_fixed(double a){
    fixed output;
    output.value = ((long int) (a*(1UL<<15)))<<13;
    return output;
}
 
struct complex{
    fixed real;
    fixed imaginary;
};
 
typedef struct complex complex;
 
complex add_complex(complex a, complex b){
    return (complex) {.real = add_fixed(a.real, b.real), .imaginary = add_fixed(a.imaginary, b.imaginary)};
}
 
complex subtract_complex(complex a, complex b){
    return (complex) {.real = subtract_fixed(a.real, b.real), .imaginary = subtract_fixed(a.imaginary, b.imaginary)};
}
 
complex multiply_complex(complex a, complex b){
    return (complex) {.real = subtract_fixed(multiply_fixed(a.real, b.real), multiply_fixed(a.imaginary, b.imaginary)), .imaginary = add_fixed(multiply_fixed(a.real, b.imaginary), multiply_fixed(a.imaginary, b.real))};
}
 
bool equals_complex(complex a, complex b){
    return a.real.value == b.real.value && a.imaginary.value == b.imaginary.value;
}
 
complex zero;
complex z;
complex c;
 
bool rendered[160][100];
unsigned char resolutions_x[5] = {8, 16, 40, 80, 160};
unsigned char resolutions_y[5] = {5, 10, 25, 50, 100};
unsigned char square_width[5] = {20, 10, 4, 2, 1};
unsigned int iterations[30] = {9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529, 576, 625, 676, 729, 784, 841, 900, 961, 1024};
int iterations_num;
volatile bool quit;
volatile bool do_exit;
volatile bool not_rendered;
complex increment_imaginary;
complex increment_real;
 
char gray_1[LCD_SIZE];
char gray_2[LCD_SIZE];
char gray_1_virtual[LCD_SIZE];
char gray_2_virtual[LCD_SIZE];
 
long int left;
long int top;
long int view_width;
long int view_height;
long int old_left;
long int old_top;
long int old_view_width;
long int old_view_height;
 
char *message;
int julia_zoom_level;
int mandelbrot_zoom_level;
unsigned int color_scheme;
 
unsigned char get_point_bw(unsigned int iterations){
    while(z.real.value < (2L<<28) && z.real.value > (-2L<<28) && z.imaginary.value < (2L<<28) && z.imaginary.value > (-2L<<28)){
        if(iterations == 0){
            return 0;
        }
        z = add_complex(multiply_complex(z, z), c);
        iterations -= 1;
    }
    return 1;
}
 
unsigned char get_point_color(unsigned int iterations){
    while(((long long int) z.real.value)*((long long int) z.real.value) + ((long long int) z.imaginary.value)*((long long int) z.imaginary.value) < (4LL<<56)){
        if(iterations == 0){
            return 0;
        }
        z = add_complex(multiply_complex(z, z), c);
        iterations -= 1;
    }
    return (iterations%3) + 1;
}
 
void display_message(){
    PortSet(gray_1, 239, 127);
    DrawStr(0, 0, "                           ", A_REVERSE);
    DrawStr(0, 0, message, A_REVERSE);
    PortSet(gray_2, 239, 127);
    DrawStr(0, 0, "                           ", A_REVERSE);
    DrawStr(0, 0, message, A_REVERSE);
}
 
void render_mandelbrot_color(int iterations){
    unsigned char x;
    unsigned char y;
    unsigned char screen_x;
    unsigned char screen_y;
    unsigned char i;
    unsigned char point_value;
    long int width;
    long int height;
    width = view_width/160;
    height = view_height/100;
    memset(rendered, false, sizeof(rendered)*sizeof(bool));
    for(i = 0; i < 5; i++){
        for(x = 0; x < resolutions_x[i]; x++){
            for(y = 0; y < resolutions_y[i]; y++){
                screen_x = x*square_width[i];
                screen_y = y*square_width[i];
                if(!rendered[screen_x][screen_y]){
                    c.real.value = left + width*screen_x;
                    c.imaginary.value = top - height*screen_y;
                    z = zero;
                    point_value = get_point_color(iterations);
                    if(!point_value){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else if(point_value == 3){
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    } else if(point_value == 2){
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else if(point_value == 1){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                     
                    if(i != 1){
                        rendered[screen_x][screen_y] = true;
                    }
                } else {
                    if(EXT_GETPIX(gray_1_virtual, screen_x, screen_y)){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                    if(EXT_GETPIX(gray_2_virtual, screen_x, screen_y)){
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                }
                if(quit){
                    return;
                }
            }
        }
         
        memcpy(gray_1_virtual, gray_1, LCD_SIZE);
        memcpy(gray_2_virtual, gray_2, LCD_SIZE);
        display_message();
        memcpy(GrayGetPlane(LIGHT_PLANE), gray_1, LCD_SIZE);
        memcpy(GrayGetPlane(DARK_PLANE), gray_2, LCD_SIZE);
    }
    not_rendered = false;
}
 
void render_julia_color(int iterations){
    unsigned char x;
    unsigned char y;
    unsigned char screen_x;
    unsigned char screen_y;
    unsigned char i;
    unsigned char point_value;
    long int width;
    long int height;
    width = view_width/160;
    height = view_height/100;
    memset(rendered, false, sizeof(rendered)*sizeof(bool));
    for(i = 0; i < 5; i++){
        for(x = 0; x < resolutions_x[i]; x++){
            for(y = 0; y < resolutions_y[i]; y++){
                screen_x = x*square_width[i];
                screen_y = y*square_width[i];
                if(!rendered[screen_x][screen_y]){
                    z.real.value = left + width*screen_x;
                    z.imaginary.value = top - height*screen_y;
                    point_value = get_point_color(iterations);
                    if(!point_value){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else if(point_value == 3){
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    } else if(point_value == 2){
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else if(point_value == 1){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                    if(i != 1){
                        rendered[screen_x][screen_y] = true;
                    }
                } else {
                    if(EXT_GETPIX(gray_1_virtual, screen_x, screen_y)){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                    if(EXT_GETPIX(gray_2_virtual, screen_x, screen_y)){
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_2, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                }
                if(quit){
                    return;
                }
            }
        }
         
        memcpy(gray_1_virtual, gray_1, LCD_SIZE);
        memcpy(gray_2_virtual, gray_2, LCD_SIZE);
        display_message();
        memcpy(GrayGetPlane(LIGHT_PLANE), gray_1, LCD_SIZE);
        memcpy(GrayGetPlane(DARK_PLANE), gray_2, LCD_SIZE);
    }
    not_rendered = false;
}
 
void render_mandelbrot_bw(int iterations){
    unsigned char x;
    unsigned char y;
    unsigned char screen_x;
    unsigned char screen_y;
    unsigned char i;
    long int width;
    long int height;
    width = view_width/160;
    height = view_height/100;
    memset(rendered, false, sizeof(rendered)*sizeof(bool));
    for(i = 0; i < 5; i++){
        for(x = 0; x < resolutions_x[i]; x++){
            for(y = 0; y < resolutions_y[i]; y++){
                screen_x = x*square_width[i];
                screen_y = y*square_width[i];
                if(!rendered[screen_x][screen_y]){
                    c.real.value = left + width*screen_x;
                    c.imaginary.value = top - height*screen_y;
                    z = zero;
                    if(!get_point_bw(iterations)){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                    if(i != 1){
                        rendered[screen_x][screen_y] = true;
                    }
                } else {
                    if(EXT_GETPIX(gray_1_virtual, screen_x, screen_y)){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                }
                if(quit){
                    return;
                }
            }
        }
        memcpy(gray_1_virtual, gray_1, LCD_SIZE);
        display_message();
        memcpy(LCD_MEM, gray_1, LCD_SIZE);
    }
    not_rendered = false;
}
 
void render_julia_bw(int iterations){
    unsigned char x;
    unsigned char y;
    unsigned char screen_x;
    unsigned char screen_y;
    unsigned char i;
    long int width;
    long int height;
    width = view_width/160;
    height = view_height/100;
    memset(rendered, false, sizeof(rendered)*sizeof(bool));
    for(i = 0; i < 5; i++){
        for(x = 0; x < resolutions_x[i]; x++){
            for(y = 0; y < resolutions_y[i]; y++){
                screen_x = x*square_width[i];
                screen_y = y*square_width[i];
                if(!rendered[screen_x][screen_y]){
                    z.real.value = left + width*screen_x;
                    z.imaginary.value = top - height*screen_y;
                    if(!get_point_bw(iterations)){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                    if(i != 1){
                        rendered[screen_x][screen_y] = true;
                    }
                } else {
                    if(EXT_GETPIX(gray_1_virtual, screen_x, screen_y)){
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_NORMAL);
                    } else {
                        FastFillRect(gray_1, screen_x, screen_y, screen_x + square_width[i], screen_y + square_width[i], A_REVERSE);
                    }
                }
                if(quit){
                    return;
                }
            }
        }
        memcpy(gray_1_virtual, gray_1, LCD_SIZE);
        display_message();
        memcpy(LCD_MEM, gray_1, LCD_SIZE);
    }
    not_rendered = false;
}
 
void *kbq;
INT_HANDLER old_int_5 = NULL;
bool mandelbrot;
bool panning;
 
DEFINE_INT_HANDLER (input_update){
    unsigned int key;
    while(!OSdequeue(&key, kbq)){
        if(key == KEY_ESC){
            quit = true;
            do_exit = true;
        }
        if(key == KEY_UP){
            top += view_width>>2;
            quit = true;
            not_rendered = true;
            sprintf(message, "%f+%fi", ((double) (left + (view_width>>1)))/(1L<<28), ((double) (top - (view_height>>1)))/(1L<<28));
        }
        if(key == KEY_DOWN){
            top -= view_width>>2;
            quit = true;
            not_rendered = true;
            sprintf(message, "%f+%fi", ((double) (left + (view_width>>1)))/(1L<<28), ((double) (top - (view_height>>1)))/(1L<<28));
        }
        if(key == KEY_LEFT){
            left -= view_width>>2;
            quit = true;
            not_rendered = true;
            sprintf(message, "%f+%fi", ((double) (left + (view_width>>1)))/(1L<<28), ((double) (top - (view_height>>1)))/(1L<<28));
        }
        if(key == KEY_RIGHT){
            left += view_width>>2;
            quit = true;
            not_rendered = true;
            sprintf(message, "%f+%fi", ((double) (left + (view_width>>1)))/(1L<<28), ((double) (top - (view_height>>1)))/(1L<<28));
        }
        if(key == KEY_T){
            iterations_num++;
            if(iterations_num >= 30){
                iterations_num = 29;
            }
            quit = true;
            not_rendered = true;
            sprintf(message, "Iterations: %d", iterations[iterations_num]);
        }
        if(key == KEY_SHIFT_T){
            iterations_num--;
            if(iterations_num < 0){
                iterations_num = 0;
            }
            quit = true;
            not_rendered = true;
            sprintf(message, "Iterations: %d", iterations[iterations_num]);
        }
        if(key == KEY_PLUS){
            left += view_width>>2;
            top -= view_height>>2;
            view_width >>= 1;
            view_height >>= 1;
            quit = true;
            not_rendered = true;
            if(mandelbrot){
                mandelbrot_zoom_level++;
                sprintf(message, "Zoom: %d", mandelbrot_zoom_level);
            } else {
                julia_zoom_level++;
                sprintf(message, "Zoom: %d", julia_zoom_level);
            }
        }
        if(key == KEY_MINUS){
            view_width <<= 1;
            view_height <<= 1;
            left -= view_width>>2;
            top += view_height>>2;
            quit = true;
            not_rendered = true;
            if(mandelbrot){
                mandelbrot_zoom_level--;
                sprintf(message, "Zoom: %d", mandelbrot_zoom_level);
            } else {
                julia_zoom_level--;
                sprintf(message, "Zoom: %d", julia_zoom_level);
            }
        }
        if(key == KEY_ENTER){
            color_scheme = !color_scheme;
            if(color_scheme){
                GrayOn();
                sprintf(message, "Grayscale mode");
            } else {
                GrayOff();
                sprintf(message, "Black and white mode");
            }
            quit = true;
            not_rendered = true;
        }
        if(key == KEY_MODE){
            if(mandelbrot){
                mandelbrot = false;
                c.real.value = left + (view_width>>1);
                c.imaginary.value = top - (view_height>>1);
                old_left = left;
                old_top = top;
                old_view_width = view_width;
                old_view_height = view_height;
                left = -2L<<28;
                top = 5L<<26;
                view_width = 8L<<27;
                view_height = 5L<<27;
                quit = true;
                not_rendered = true;
            } else {
                mandelbrot = true;
                left = old_left;
                top = old_top;
                view_width = old_view_width;
                view_height = old_view_height;
                quit = true;
                not_rendered = true;
            }
        }
    }
}
 
int _main(){
    quit = false;
    do_exit = false;
    not_rendered = true;
    iterations_num = 0;
    julia_zoom_level = 1;
    mandelbrot_zoom_level = 1;
    color_scheme = 0;
    message = malloc(sizeof(char)*256);
    sprintf(message, "Mandafractal Ti89 v1.1");
    increment_real = (complex) {.real = float_to_fixed(0.05), .imaginary = float_to_fixed(0)};
    increment_imaginary = (complex) {.real = float_to_fixed(0), .imaginary = float_to_fixed(0.05)};
    zero = (complex) {.real = float_to_fixed(0), .imaginary = float_to_fixed(0)};
    z = zero;
    left = -2L<<28;
    top = 5L<<26;
    view_width = 8L<<27;
    view_height = 5L<<27;
    old_left = left;
    old_top = top;
    old_view_width = view_width;
    old_view_height = view_height;
    mandelbrot = true;
    panning = true;
    clrscr();
     
    kbq = kbd_queue();
    old_int_5 = GetIntVec(AUTO_INT_5);
    SetIntVec(AUTO_INT_5, input_update);
     
    c = (complex) {.real = float_to_fixed(0.25), .imaginary = float_to_fixed(0)};
    while(!do_exit){
        quit = false;
        if(not_rendered && mandelbrot){
            if(color_scheme){
                render_mandelbrot_color(iterations[iterations_num]);
            } else {
                render_mandelbrot_bw(iterations[iterations_num]);
            }
        } else if(not_rendered){
            if(color_scheme){
                render_julia_color(iterations[iterations_num]);
            } else {
                render_julia_bw(iterations[iterations_num]);
            }
        }
    }
    SetIntVec(AUTO_INT_5, old_int_5);
    GrayOff();
    PortRestore();
    free(message);
    return 0;
}
