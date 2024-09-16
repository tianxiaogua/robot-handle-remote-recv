#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "driver_tool.h"

typedef struct key_detection {
	uint8 KEY_LEFT;
	uint8 KEY_RIGHT;
	uint8 KEY_UP_;
	uint8 KEY_DOWN_;

	uint8 KEY_A;
	uint8 KEY_B;
	uint8 KEY_X;
	uint8 KEY_Y;

	uint8 KEY_LB;
	uint8 KEY_RB;

	uint8 KEY_SELECT;
	uint8 KEY_START;

	uint8 KEY_R_KEY;
	uint8 KEY_L_KEY;

	uint16 LX;
	uint16 LY;
	uint16 RX;
	uint16 RY;

	uint16 ca_calue_LX;
	uint16 ca_calue_LY;
	uint16 ca_calue_RX;
	uint16 ca_calue_RY;
} KEY_DETECTION;

int32 protocol_decode(const char *json_str, KEY_DETECTION *key_value);

#endif


