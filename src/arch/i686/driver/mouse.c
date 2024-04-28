/**
 * @file mouse.c
 * @brief PS/2 Mouse implementation
 * @author Kevin Oehme
 * @date 28.04.2024
 * @version 1.0
 * @copyright MIT
 */

#include "mouse.h"
#include "ps2.h"
#include "idt.h"

extern void asm_interrupt_32h(void);

typedef enum MouseMask
{
    LEFT_BUTTON_MASK = 0b00000001,
    RIGHT_BUTTON_MASK = 0b00000010,
    MIDDLE_BUTTON_MASK = 0b00000100,
    ALIGNED_PACKET_MASK = 0b00001000,
    Y_AXIS_OVERFLOW_MASK = 0b01000000,
    X_AXIS_OVERFLOW_MASK = 0b10000000,
    SIGN_BIT_MASK = 0b100000000,
} MouseMask;

Mouse mouse = {
    .x = 0,
    .y = 0,
    .cycle = 0,
    .flags = 0,
    .x_movement = 0,
    .y_movement = 0,
    .bytes = {0},
};

// Determines whether the specified mouse flag is set in the mouse flags, facilitating conditional checks based on specific mouse events
static inline bool mouse_has_flag(const Mouse *self, const MouseMask mask)
{
    return (self->flags & mask) != 0;
};

// Updates the x and y-coordinate movement of the mouse, taking into account the sign bit of the mouse flags for accurate positioning especially for negative values
static void mouse_update_coordinates(Mouse *self)
{
    // Adjusts the x and y-coordinate movement by checking the sign bit of the mouse flags, ensuring correct positioning even for negative values
    const int16_t delta_x = self->x_movement - ((self->flags << 3) & SIGN_BIT_MASK);
    const int16_t delta_y = self->y_movement - ((self->flags << 4) & SIGN_BIT_MASK);
    self->x += delta_x;
    self->y += delta_y;
    return;
};

// Handles the PS/2 mouse input, processing mouse packets and updating mouse coordinates
void mouse_handler(Mouse *self)
{
    switch (self->cycle)
    {
    case 0:
    {
        self->flags = ps2_receive();

        if (!mouse_has_flag(self, ALIGNED_PACKET_MASK))
        {
            self->cycle = 0;
            break;
        };

        if (mouse_has_flag(self, LEFT_BUTTON_MASK))
        {
            printf("Left Btn\n");
        }
        else if (mouse_has_flag(self, RIGHT_BUTTON_MASK))
        {
            printf("Right Btn\n");
        }
        else if (mouse_has_flag(self, MIDDLE_BUTTON_MASK))
        {
            printf("Mid Btn\n");
        };
        self->cycle++;
        break;
    };
    case 1:
    {
        self->x_movement = ps2_receive();
        self->cycle++;
        break;
    };
    case 2:
    {
        self->y_movement = ps2_receive();

        if (!mouse_has_flag(self, Y_AXIS_OVERFLOW_MASK) || !mouse_has_flag(self, X_AXIS_OVERFLOW_MASK))
        {
            mouse_update_coordinates(self);
            printf("(%d,%d)\n", self->x, self->y);
        };
        self->cycle = 0;
        break;
    };
    };
    return;
};

// Initializes the PS/2 mouse device, enabling auxiliary mouse device, setting interrupts and configuring default settings for data reporting
void mouse_init(Mouse *self)
{
    // Enable the auxiliary mouse device
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_ENABLE_AUX);
    // Enable the interrupts
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_GET_COMPAQ_STATUS);
    const uint8_t status = ps2_receive() | 0b00000010;
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SET_COMPAQ_STATUS);
    ps2_send(PS2_DATA_PORT, status);
    // Tell the mouse to use default settings
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SEND_COMMAND);
    ps2_send(PS2_DATA_PORT, MOUSE_SET_DEFAULT);
    ps2_receive();
    // Enable data reporting
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SEND_COMMAND);
    ps2_send(PS2_DATA_PORT, MOUSE_ENABLE_DATA_REPORT);
    ps2_receive();
    idt_set(0x2C, asm_interrupt_32h);
    return;
};