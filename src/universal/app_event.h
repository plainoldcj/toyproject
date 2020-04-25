#pragma once

// TODO(cj) Maybe rename this, because there are already app states on game side.

struct AppEventMouse
{
	int x;
	int y;
};

enum AppEventType
{
	APP_EVENT_MOUSE_BUTTON_UP
};

struct AppEvent
{
	int type; // In AppEventType
	struct AppEventMouse mouse;
};
