#include "Event.h"

#include "Cube/Core/Log.h"

namespace Cube {

    void EventDispatcher::dispatch(const Event& e) {
		for(auto& handler : listener[e.getType()]) {
			handler(e);
			CB_CORE_TRACE("{} was handled", e.toString());
		}
	}
}