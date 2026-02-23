#pragma once

/// @brief this file contain all the event struct for the event system
//The event system is a simple struct that contain the data of the event and a boolean to know if the event is handled or not
struct Event
{
	virtual ~Event() = default;
	bool handled = false;
};

//The event struct for the key event
struct KeyEvent : public Event
{
	KeyEvent(int k, int a) : key(k), action(a) {}
	int key;
	int action;
};

//The event struct for the mouse event
struct MouseEvent : public Event
{
	MouseEvent(double x, double y) : xPos(x), yPos(y) {}
	double xPos;
	double yPos;
};

//The event struct for the mouse button event
struct MouseButtonEvent : public Event
{
	MouseButtonEvent(int b, int a) : button(b), action(a) {}
	int button;
	int action;
};

//The event struct for the scroll event
struct ScrollEvent : public Event
{
	ScrollEvent(double x, double y) : xOffset(x), yOffset(y) {}
	double xOffset;
	double yOffset;
};

//The event struct for the window resize event
struct WindowResizeEvent : public Event
{
	WindowResizeEvent(int w, int h) : width(w), height(h) {}
	int width;
	int height;
};

//The event struct for the window close event
struct WindowCloseEvent : public Event
{
	WindowCloseEvent() = default;
};

//The event struct for the framebuffer resize event
struct FramebufferResizeEvent : public Event
{
	FramebufferResizeEvent(int w, int h) : width(w), height(h) {}
	int width;
	int height;
};