#include "input.h"

#include "nds.h"

void input_update(input_t *input) {
    scanKeys();

    input->held = keysHeld();
    input->just_pressed = keysDown();
    input->just_released = keysUp();
}
