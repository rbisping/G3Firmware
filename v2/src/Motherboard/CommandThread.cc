#include "CommandThread.hh"
#include "CommandQueue.hh"
#include "Steppers.hh"
#include "Commands.hh"
#include "DebugPin.hh"
#include "Configuration.hh"
#include "Timeout.hh"

bool command_thread_paused = false;

void pauseCommandThread(bool pause) {
	command_thread_paused = pause;
}

bool isCommandThreadPaused() {
	return command_thread_paused;
}

int32_t pop32() {
	union {
		// AVR is little-endian
		int32_t a;
		struct {
			uint8_t data[4];
		} b;
	} shared;
	shared.b.data[0] = command_buffer.pop();
	shared.b.data[1] = command_buffer.pop();
	shared.b.data[2] = command_buffer.pop();
	shared.b.data[3] = command_buffer.pop();
	return shared.a;
}

enum {
	READY,
	MOVING,
	DELAY,
	WAIT_ON_TOOL
} mode = READY;

Timeout delay_timeout;

void resetCommands() {
	command_buffer.reset();
	mode = READY;
}

// A fast slice for processing commands and refilling the stepper queue, etc.
void runCommandSlice() {
	if (command_thread_paused) { return; }
	if (mode == MOVING) {
		if (!steppers.isRunning()) { mode = READY; }
	}
	if (mode == DELAY) {
		// check timers
		if (delay_timeout.hasElapsed()) {
			mode = READY;
		}
	}
	if (mode == READY) {
		// process next command on the queue.
		if (command_buffer.getLength() > 0) {
			uint8_t command = command_buffer[0];
			if (command == HOST_CMD_QUEUE_POINT_ABS) {
				// check for completion
				if (command_buffer.getLength() >= 17) {
					command_buffer.pop(); // remove the command code
					mode = MOVING;
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					int32_t dda = pop32();
					steppers.setTarget(Point(x,y,z),dda);
				}
			} else if (command == HOST_CMD_SET_POSITION) {
				// check for completion
				if (command_buffer.getLength() >= 13) {
					command_buffer.pop(); // remove the command code
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					steppers.definePosition(Point(x,y,z));
				}
			} else if (command == HOST_CMD_DELAY) {
				if (command_buffer.getLength() >= 5) {
					mode = DELAY;
					command_buffer.pop(); // remove the command code
					uint32_t microseconds = pop32();
					delay_timeout.start(microseconds);
				}
			}
		}
	}
}
