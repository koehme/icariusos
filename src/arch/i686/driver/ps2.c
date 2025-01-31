/**
 * @file ps2.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ps2.h"
#include "io.h"

/* PUBLIC API */
int ps2_wait(const uint8_t type);
void ps2_send(const uint8_t port, const uint8_t byte);
uint8_t ps2_receive(void);
void ps2_dispatch(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device);

int ps2_wait(const uint8_t type)
{
	uint32_t timeout = 100000;

	while (timeout--) {
		switch (type) {
		case PS2_BUFFER_INPUT:
			if (!(inb(PS2_STATUS_COMMAND_PORT) & PS2_BUFFER_INPUT)) {
				return 0; // Input buffer is ready
			};
			break;
		case PS2_BUFFER_OUTPUT:
			if (inb(PS2_STATUS_COMMAND_PORT) & PS2_BUFFER_OUTPUT) {
				return 0; // Output buffer is ready
			};
			break;
		default:
			return -1; // Invalid type
		};
	};
	return -1; // Timeout occurred
};

void ps2_send(const uint8_t port, const uint8_t byte)
{
	// Wait until the Input Buffer is empty
	if (ps2_wait(PS2_BUFFER_INPUT) == 0) {
		// Send the byte to the specified port
		outb(port, byte);
	};
	return;
}

uint8_t ps2_receive(void)
{
	// Wait until the Output Buffer is full
	if (ps2_wait(PS2_BUFFER_OUTPUT) == 0) {
		// Read the byte from the data port
		return inb(PS2_DATA_PORT);
	};
	return 0;
};

void ps2_dispatch(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device)
{
	uint8_t ps2_data;

	for (size_t packages = 0; packages < PS2_PACKAGE_DISPATCH && fifo_dequeue(fifo, &ps2_data); packages++) {
		handler(device, ps2_data);
	};
	return;
};