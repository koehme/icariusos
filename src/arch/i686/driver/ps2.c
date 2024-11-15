/**
 * @file ps2.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ps2.h"
#include "io.h"

void ps2_send(const uint8_t port, const uint8_t byte)
{
	while (inb(PS2_STATUS_COMMAND_PORT) & PS2_BUFFER_INPUT) {
		;
	};
	outb(port, byte);
	return;
};

uint8_t ps2_receive(void)
{
	while (!(inb(PS2_STATUS_COMMAND_PORT) & PS2_BUFFER_OUTPUT)) {
		;
	};
	const uint8_t data = inb(PS2_DATA_PORT);
	return data;
};

void ps2_dispatch(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device)
{
	uint8_t ps2_data;

	for (size_t packages = 0; packages < PS2_PACKAGE_DISPATCH && fifo_dequeue(fifo, &ps2_data); packages++) {
		handler(device, ps2_data);
	};
	return;
};