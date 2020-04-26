#pragma once

// TODO(cj) Maybe rename this, because there are already app states on game side.

struct AppEventMouse
{
	int x;
	int y;
};

enum AppKey
{
	APP_KEY_LEFT,
	APP_KEY_RIGHT,
	APP_KEY_UP,
	APP_KEY_DOWN,
	APP_KEY_SPACE
};

struct AppEventKey
{
	uint16_t key; // In AppKey
};

enum AppEventType
{
	APP_EVENT_KEY_DOWN,
	APP_EVENT_KEY_UP,
	APP_EVENT_MOUSE_BUTTON_UP
};

struct AppEvent
{
	int type; // In AppEventType
	struct AppEventMouse mouse;
	struct AppEventKey key;
};
