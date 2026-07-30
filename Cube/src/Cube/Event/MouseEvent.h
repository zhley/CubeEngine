#pragma once

#include "Event.h"
#include "Cube/Core/MouseCodes.h"

namespace Cube {
	class MouseEvent : public Event {
	public:
		MouseEvent(double x, double y) : x(x), y(y){}
		virtual ~MouseEvent() = default;

		double x;
		double y;
	};

	class MousePressedEvent : public MouseEvent {
	public:
		MousePressedEvent(double x, double y, MouseCode mouseCode) : MouseEvent(x, y), mouseCode(mouseCode){}
		virtual ~MousePressedEvent() = default;
		MouseCode getMouseCode() {
			return mouseCode;
		}
		EVENT_TYPE(MousePressedEvent)

		MouseCode mouseCode;
	};

	class MouseReleasedEvent : public MouseEvent {
	public:
		MouseReleasedEvent(double x, double y, MouseCode mouseCode) : MouseEvent(x, y), mouseCode(mouseCode){}
		virtual ~MouseReleasedEvent() = default;
		MouseCode getMouseCode() {
			return mouseCode;
		}
		EVENT_TYPE(MouseReleasedEvent)

		MouseCode mouseCode;
	};

	class MouseScrolledEvent : public MouseEvent {
	public:
		MouseScrolledEvent(double x, double y, double xOffset, double yOffset) : MouseEvent(x, y), xOffset(xOffset), yOffset(yOffset){}
		virtual ~MouseScrolledEvent() = default;
		EVENT_TYPE(MouseScrolledEvent)

		double xOffset;
		double yOffset;
	};

	class MouseMovedEvent : public MouseEvent {
	public:
		MouseMovedEvent(double x, double y) : MouseEvent(x, y){}
		virtual ~MouseMovedEvent() = default;
		EVENT_TYPE(MouseMovedEvent)
	};
}