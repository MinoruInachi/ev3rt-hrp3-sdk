/*
 * cli_motor.c
 *
 *  Created on: Jun 22, 2014
 *      Author: liyixiao
 */

#include "ev3api.h"
#include "app.h"
#include "syssvc/serial.h"
#include <unistd.h>
#include <ctype.h>

static
const CliMenuEntry* select_motor() {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Motor port A", .exinf = EV3_PORT_A },
		{ .key = '2', .title = "Motor port B", .exinf = EV3_PORT_B },
		{ .key = '3', .title = "Motor port C", .exinf = EV3_PORT_C },
		{ .key = '4', .title = "Motor port D", .exinf = EV3_PORT_D },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Select Port",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
	const CliMenuEntry* cme = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);

#if 1
	return cme;
#else
	show_cli_menu(&climenu);

	return select_menu_entry(&climenu);
#endif
}

void connect_motor(intptr_t unused) {
	const CliMenuEntry* cme_port = NULL;
	while(cme_port == NULL) {
//		fio_clear_screen();
//		fprintf(fio, "--- Connect Motor ---\n");
		cme_port = select_motor();
	}

	if(cme_port->exinf == -1) return;

	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Large motor", .exinf = LARGE_MOTOR },
		{ .key = '2', .title = "Medium motor", .exinf = MEDIUM_MOTOR },
		{ .key = '3', .title = "Unregulated", .exinf = UNREGULATED_MOTOR },
		{ .key = 'N', .title = "Not connected", .exinf = NONE_MOTOR },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Select Type",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	const CliMenuEntry* cme_type = NULL;
	while(cme_type == NULL) {
#if 1
		show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
		cme_type = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
#else
		fio_clear_screen();
		fprintf(fio, "--- Connect Motor ---\n");
		fprintf(fio, ">>> %s\n", cme_port->title);
		show_cli_menu(&climenu);
		cme_type = select_menu_entry(&climenu);
#endif
	}

	if(cme_type->exinf == -1) return;

	ev3_motor_config(cme_port->exinf, cme_type->exinf);
}



static
void test_normal_motor(ID port) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Control speed", .exinf = (intptr_t)ev3_motor_set_power },
		{ .key = '2', .title = "Rotate", .exinf = (intptr_t)ev3_motor_rotate },
		{ .key = '3', .title = "Stop", .exinf = (intptr_t)ev3_motor_stop },
		{ .key = '4', .title = "Show counts", .exinf = (intptr_t)ev3_motor_get_counts },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static CliMenu climenu = {
//		.title     = ">>> Select an operation:",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	motor_type_t mt = ev3_motor_get_type(port);

	while(1) {
#if 1
		// Clear menu area & draw title
		static char title[100];
		sprintf(title, "Test %s @ %c", (mt == LARGE_MOTOR ? "L-Motor" : "M-Motor"), 'A' + port);
		climenu.title = title;
		show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
		const CliMenuEntry* cme_op = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
#else
		fio_clear_screen();
		fprintf(fio, "--- Test %s @ Port%c---\n", (mt == LARGE_MOTOR ? "Large Servo Motor" : "Medium Servo Motor"), 'A' + port);
		show_cli_menu(&climenu);
		const CliMenuEntry* cme_op = select_menu_entry(&climenu);
#endif
		if(cme_op == NULL) continue;
		if(cme_op->exinf == (intptr_t)ev3_motor_set_power) {
			// Refresh current speed
			ev3_lcd_fill_rect(0, MENU_FONT_HEIGHT, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
			while (1) {
				static char title[100];
				sprintf(title, "Power: %-4d", ev3_motor_get_power(port));
				ev3_lcd_draw_string(title, 0, MENU_FONT_HEIGHT);
				if (ev3_button_is_pressed(UP_BUTTON)) {
					while(ev3_button_is_pressed(UP_BUTTON));
					ev3_motor_set_power(port, ev3_motor_get_power(port) + 10);
				}
				if (ev3_button_is_pressed(DOWN_BUTTON)) {
					while(ev3_button_is_pressed(DOWN_BUTTON));
					ev3_motor_set_power(port, ev3_motor_get_power(port) - 10);
				}
				if (ev3_button_is_pressed(BACK_BUTTON)) {
					while(ev3_button_is_pressed(BACK_BUTTON));
					break;
				}
			}
		} else if(cme_op->exinf == (intptr_t)ev3_motor_rotate) {
	    	ev3_motor_rotate(port, 90, 30, false);
	    	show_message_box("Rotate Motor", "Motor is rotated for 90 degrees at 30% power.");
		} else if(cme_op->exinf == (intptr_t)ev3_motor_stop) {
			// Refresh current speed
			ev3_lcd_fill_rect(0, MENU_FONT_HEIGHT, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
			ev3_lcd_draw_string("ENTER: Brake.", 0, MENU_FONT_HEIGHT);
			ev3_lcd_draw_string("DOWN:  Coast.", 0, MENU_FONT_HEIGHT * 2);
			ev3_lcd_draw_string("BACK:  Cancel.", 0, MENU_FONT_HEIGHT * 3);
			while (1) {
				if (ev3_button_is_pressed(ENTER_BUTTON)) {
					while(ev3_button_is_pressed(ENTER_BUTTON));
					ev3_motor_stop(port, true);
					break;
				}
				if (ev3_button_is_pressed(DOWN_BUTTON)) {
					while(ev3_button_is_pressed(DOWN_BUTTON));
					ev3_motor_stop(port, false);
					break;
				}
				if (ev3_button_is_pressed(BACK_BUTTON)) {
					while(ev3_button_is_pressed(BACK_BUTTON));
					break;
				}
			}
		} else if(cme_op->exinf == (intptr_t)ev3_motor_get_counts) {
			// Refresh current speed
			ev3_lcd_fill_rect(0, MENU_FONT_HEIGHT, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
			while (1) {
				static char title[100];
				sprintf(title, "Counts: %-5ld", ev3_motor_get_counts(port));
				ev3_lcd_draw_string(title, 0, MENU_FONT_HEIGHT);
				if (ev3_button_is_pressed(BACK_BUTTON)) {
					while(ev3_button_is_pressed(BACK_BUTTON));
					break;
				}
			}
		} else if(cme_op->exinf == -1) {
			ev3_motor_stop(port, false);
			return;
		} else assert(false);
	}
}

static
void test_unreg_motor(ID port) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Control Power", .exinf = (intptr_t)ev3_motor_set_power },
		{ .key = '2', .title = "Stop", .exinf = (intptr_t)ev3_motor_stop },
		{ .key = '3', .title = "Tachometer", .exinf = (intptr_t)ev3_motor_get_counts },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Test Unreg Motor",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	while(1) {
#if 1
		show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
		const CliMenuEntry* cme_op = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
#else
		fio_clear_screen();
		fprintf(fio, "--- Test Unregulated Motor @ Port%c---\n", 'A' + port);
		show_cli_menu(&climenu);
		const CliMenuEntry* cme_op = select_menu_entry(&climenu);
#endif
		if(cme_op == NULL) continue;

		if(cme_op->exinf == (intptr_t)ev3_motor_set_power) {
			// Refresh current speed
			ev3_lcd_fill_rect(0, MENU_FONT_HEIGHT, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
			while (1) {
				static char title[100];
				sprintf(title, "Power: %-4d", ev3_motor_get_power(port));
				ev3_lcd_draw_string(title, 0, MENU_FONT_HEIGHT);
				if (ev3_button_is_pressed(UP_BUTTON)) {
					while(ev3_button_is_pressed(UP_BUTTON));
					ev3_motor_set_power(port, ev3_motor_get_power(port) + 10);
				}
				if (ev3_button_is_pressed(DOWN_BUTTON)) {
					while(ev3_button_is_pressed(DOWN_BUTTON));
					ev3_motor_set_power(port, ev3_motor_get_power(port) - 10);
				}
				if (ev3_button_is_pressed(BACK_BUTTON)) {
					while(ev3_button_is_pressed(BACK_BUTTON));
					break;
				}
			}
		} else if(cme_op->exinf == (intptr_t)ev3_motor_stop) {
			// Refresh current speed
			ev3_lcd_fill_rect(0, MENU_FONT_HEIGHT, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
			ev3_lcd_draw_string("ENTER: Brake.", 0, MENU_FONT_HEIGHT);
			ev3_lcd_draw_string("DOWN:  Coast.", 0, MENU_FONT_HEIGHT * 2);
			ev3_lcd_draw_string("BACK:  Cancel.", 0, MENU_FONT_HEIGHT * 3);
			while (1) {
				if (ev3_button_is_pressed(ENTER_BUTTON)) {
					while(ev3_button_is_pressed(ENTER_BUTTON));
					ev3_motor_stop(port, true);
					break;
				}
				if (ev3_button_is_pressed(DOWN_BUTTON)) {
					while(ev3_button_is_pressed(DOWN_BUTTON));
					ev3_motor_stop(port, false);
					break;
				}
				if (ev3_button_is_pressed(BACK_BUTTON)) {
					while(ev3_button_is_pressed(BACK_BUTTON));
					break;
				}
			}
		} else if(cme_op->exinf == (intptr_t)ev3_motor_get_counts) {
			// Refresh current speed
			ev3_lcd_fill_rect(0, MENU_FONT_HEIGHT, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
			while (1) {
				static char title[100];
				sprintf(title, "Counts: %-5ld", ev3_motor_get_counts(port));
				ev3_lcd_draw_string(title, 0, MENU_FONT_HEIGHT);
				if (ev3_button_is_pressed(BACK_BUTTON)) {
					while(ev3_button_is_pressed(BACK_BUTTON));
					break;
				}
			}
		} else if(cme_op->exinf == -1) {
			ev3_motor_stop(port, false);
			return;
		} else assert(false);
	}
}

void test_motor(intptr_t unused) {
	const CliMenuEntry* cme_port = NULL;
	while(cme_port == NULL) {
//		fio_clear_screen();
//		fprintf(fio, "--- Test Motor ---\n");
		cme_port = select_motor();
	}

	if(cme_port->exinf == -1) return;

	motor_type_t mt = ev3_motor_get_type(cme_port->exinf);

	switch(mt) {
	case LARGE_MOTOR:
	case MEDIUM_MOTOR:
		test_normal_motor(cme_port->exinf);
		break;

	case UNREGULATED_MOTOR:
		test_unreg_motor(cme_port->exinf);
		break;

	case NONE_MOTOR: {
		char msgbuf[100];
		sprintf(msgbuf, "%s is not connected.", cme_port->title);
		show_message_box("No Motor", msgbuf);
		}
		break;

	default:
		assert(false);
	}

}
