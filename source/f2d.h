#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

typedef struct float_d {
    bool s;
    uint32_t m;
    int32_t e;
} float_d;

typedef uint32_t float32;

void toString_hex(uint32_t num, char* numArray);
float_d f2d(float32 f);
float32 stringToFloat(char* s, int n);
int floatToString(float32 f, char* s, int n);
