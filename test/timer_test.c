#include "timer.h"
#include <stdio.h>

int main() {
    double now = get_time();
    printf("%f\n", now/3600);
}