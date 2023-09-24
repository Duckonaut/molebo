#pragma once

typedef struct input {
    int held;
    int just_pressed;
    int just_released;
} input_t;

void input_update(input_t *input);
