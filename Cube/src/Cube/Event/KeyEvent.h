#pragma once

#include "Event.h"
#include "Cube/Core/KeyCodes.h"

namespace Cube {

	class KeyPressedEvent : public Event {
	public:
		EVENT_TYPE(KeyPressedEvent)

		KeyPressedEvent(KeyCode keyCode) : keyCode(keyCode), isRepeat(false){}
		KeyPressedEvent(KeyCode keyCode, bool isRepeat) : keyCode(keyCode), isRepeat(isRepeat) {}
        virtual  ~KeyPressedEvent() override = default;

		KeyCode keyCode;
		bool isRepeat;
	};

	class KeyReleasedEvent : public Event {
	public:
		EVENT_TYPE(KeyReleasedEvent)

		KeyReleasedEvent(KeyCode keyCode) : keyCode(keyCode){}
		virtual ~KeyReleasedEvent() override = default;

	    KeyCode keyCode;
	};
}
