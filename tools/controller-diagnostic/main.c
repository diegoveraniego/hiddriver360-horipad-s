// Shows and records the Xbox's final XInput state. Does not change controller input.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "xsdk/gpu.h"
#include "xsdk/kernel.h"
#include "xsdk/log.h"
#include "xsdk/present.h"
#include "xsdk/text.h"

enum { PLAYERS = 4, WIDTH = 1280, HEIGHT = 720, BUFFERS = 2, TIMEOUT_MS = 5000 };

typedef struct {
	uint32_t result;
	INPUT_STATE state;
} Sample;

static const struct { uint16_t mask; const char *name; } button_names[] = {
	{ INPUT_GAMEPAD_DPAD_UP, "UP" }, { INPUT_GAMEPAD_DPAD_DOWN, "DOWN" },
	{ INPUT_GAMEPAD_DPAD_LEFT, "LEFT" }, { INPUT_GAMEPAD_DPAD_RIGHT, "RIGHT" },
	{ INPUT_GAMEPAD_START, "START" }, { INPUT_GAMEPAD_BACK, "BACK" },
	{ INPUT_GAMEPAD_LEFT_THUMB, "LS" }, { INPUT_GAMEPAD_RIGHT_THUMB, "RS" },
	{ INPUT_GAMEPAD_LEFT_SHOULDER, "LB" }, { INPUT_GAMEPAD_RIGHT_SHOULDER, "RB" },
	{ INPUT_GAMEPAD_GUIDE, "GUIDE" }, { INPUT_GAMEPAD_A, "A" },
	{ INPUT_GAMEPAD_B, "B" }, { INPUT_GAMEPAD_X, "X" }, { INPUT_GAMEPAD_Y, "Y" },
};

static void button_text(uint16_t buttons, char *out, size_t cap)
{
	out[0] = '\0';
	for (size_t i = 0; i < sizeof(button_names) / sizeof(button_names[0]); i++) {
		if (!(buttons & button_names[i].mask)) continue;
		const size_t used = strlen(out);
		if (used >= cap - 1) break;
		snprintf(out + used, cap - used, "%s%s", used ? "+" : "", button_names[i].name);
	}
	if (!out[0]) snprintf(out, cap, "none");
}

static bool changed(const Sample *a, const Sample *b)
{
	if (a->result != b->result) return true;
	if (a->result != ERROR_SUCCESS) return false;
	const INPUT_GAMEPAD *x = &a->state.gamepad, *y = &b->state.gamepad;
	if (x->buttons != y->buttons || x->left_trigger != y->left_trigger ||
		x->right_trigger != y->right_trigger) return true;
	if ((int)x->thumb_lx - y->thumb_lx > 1024 || (int)y->thumb_lx - x->thumb_lx > 1024 ||
		(int)x->thumb_ly - y->thumb_ly > 1024 || (int)y->thumb_ly - x->thumb_ly > 1024 ||
		(int)x->thumb_rx - y->thumb_rx > 1024 || (int)y->thumb_rx - x->thumb_rx > 1024 ||
		(int)x->thumb_ry - y->thumb_ry > 1024 || (int)y->thumb_ry - x->thumb_ry > 1024) return true;
	return false;
}

static void log_sample(unsigned player, uint64_t elapsed_ms, const Sample *s)
{
	if (s->result != ERROR_SUCCESS) {
		xsdk_log_write("t=%llu player=%u disconnected result=%08X", (unsigned long long)elapsed_ms,
			player + 1, (unsigned)s->result);
		return;
	}
	char buttons[128];
	button_text(s->state.gamepad.buttons, buttons, sizeof buttons);
	xsdk_log_write("t=%llu player=%u packet=%u bits=%04X buttons=%s LT=%u RT=%u LX=%d LY=%d RX=%d RY=%d",
		(unsigned long long)elapsed_ms, player + 1, (unsigned)s->state.packet_number,
		(unsigned)s->state.gamepad.buttons, buttons,
		(unsigned)s->state.gamepad.left_trigger, (unsigned)s->state.gamepad.right_trigger,
		(int)s->state.gamepad.thumb_lx, (int)s->state.gamepad.thumb_ly,
		(int)s->state.gamepad.thumb_rx, (int)s->state.gamepad.thumb_ry);
}

int main(void)
{
	xsdk_log_write("HORIPAD diagnostic: XamInputGetState only; no raw USB capture in this build");
	NTSTATUS status = xsdk_gpu_open();
	if (!NT_SUCCESS(status)) return 1;
	status = xsdk_text_open();
	if (!NT_SUCCESS(status)) { xsdk_gpu_close(); return 1; }
	uint32_t width = 0, height = 0;
	status = xsdk_present_open(XSDK_DISPLAY_720P, BUFFERS, 1, &width, &height);
	if (!NT_SUCCESS(status)) { xsdk_text_close(); xsdk_gpu_close(); return 1; }
	Sample previous[PLAYERS] = { 0 }, current[PLAYERS] = { 0 };
	uint16_t last_probe_extra[PLAYERS] = { 0 };
	uint64_t start = 0;
	KeQuerySystemTime(&start);
	bool first = true;
	for (uint32_t frame = 0;; frame++) {
		uint64_t now = 0;
		KeQuerySystemTime(&now);
		const uint64_t elapsed_ms = (now - start) / 10000;
		for (unsigned p = 0; p < PLAYERS; p++) {
			current[p].state = (INPUT_STATE){ 0 };
			current[p].result = XamInputGetState(p, 0, &current[p].state);
			if (first || changed(&current[p], &previous[p]))
				log_sample(p, elapsed_ms, &current[p]);
			previous[p] = current[p];
			if (frame % 60 == 0 && current[p].result == ERROR_SUCCESS) {
				INPUT_STATE seeded = { 0 };
				seeded.gamepad.buttons = INPUT_GAMEPAD_A | INPUT_GAMEPAD_B | INPUT_GAMEPAD_X | INPUT_GAMEPAD_Y;
				const uint32_t seeded_result = XamInputGetState(p, 0, &seeded);
				if (seeded_result == ERROR_SUCCESS) {
					const uint16_t extra = seeded.gamepad.buttons & ~current[p].state.gamepad.buttons;
					if (extra != last_probe_extra[p] || first)
						xsdk_log_write("probe t=%llu player=%u clean=%04X seeded=%04X extra=%04X",
							(unsigned long long)elapsed_ms, p + 1,
							(unsigned)current[p].state.gamepad.buttons,
							(unsigned)seeded.gamepad.buttons, (unsigned)extra);
					last_probe_extra[p] = extra;
				}
			}
		}
		first = false;
		uint32_t address = 0;
		status = xsdk_present_begin(&address, TIMEOUT_MS);
		if (!NT_SUCCESS(status)) break;
		xsdk_gpu_set_target(width, height);
		xsdk_gpu_resolve(address, width, height, 0xFF101820);
		xsdk_gpu_set_target(width, height);
		xsdk_text_draw(35, 25, 2, 0xFFFFFFFF, "HORIPAD / XBOX 360 INPUT DIAG");
		xsdk_text_draw(35, 85, 1, 0xFFCCCCCC, "Press one button at a time. Xbox Home exits. Log: game:\\horipad_diag.log");
		for (unsigned p = 0; p < PLAYERS; p++) {
			char line[220];
			const Sample *s = &current[p];
			if (s->result != ERROR_SUCCESS) {
				snprintf(line, sizeof line, "Player %u: disconnected (%08X)", p + 1, (unsigned)s->result);
			} else {
				char buttons[128];
				button_text(s->state.gamepad.buttons, buttons, sizeof buttons);
				snprintf(line, sizeof line, "Player %u: %s  LT %u  RT %u", p + 1, buttons,
					(unsigned)s->state.gamepad.left_trigger, (unsigned)s->state.gamepad.right_trigger);
			}
			xsdk_text_draw(35, 145 + (int32_t)p * 95, 1, p == 0 ? 0xFF7DFFB3 : 0xFFFFFFFF, line);
			if (s->result == ERROR_SUCCESS) {
				snprintf(line, sizeof line, "  sticks: LX %d LY %d RX %d RY %d; packet %u",
					(int)s->state.gamepad.thumb_lx, (int)s->state.gamepad.thumb_ly,
					(int)s->state.gamepad.thumb_rx, (int)s->state.gamepad.thumb_ry,
					(unsigned)s->state.packet_number);
				xsdk_text_draw(35, 175 + (int32_t)p * 95, 1, 0xFFB0B0B0, line);
			}
		}
		xsdk_gpu_resolve(address, width, height, 0xFF101820);
		xsdk_present_end();
	}
	xsdk_present_close();
	xsdk_text_close();
	xsdk_gpu_close();
	return 0;
}
