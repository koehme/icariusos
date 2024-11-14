/**
 * @file ps2.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ps2.h"
#include "io.h"

// Sends data to a PS/2 port
void ps2_send(const uint8_t port, const uint8_t byte)
{
	outb(port, byte);
	return;
};

// Receives data from a PS/2 port
uint8_t ps2_receive(void)
{
	const uint8_t data = inb(PS2_DATA_PORT);
	return data;
};

void ps2_process_dev(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device)
{
	uint8_t ps2_package;

	while (fifo_dequeue(fifo, &ps2_package)) {
		handler(device, ps2_package);
	};
	return;
};