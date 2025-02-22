#include "EventManager.h"

std::unordered_map<std::string, std::vector<sol::function>> EventManager::m_Handlers = {
	/*** SERVER **/
	{"onServerInit", {}},
	{"onServerShutdown", {}},
	{"onServerFrame", {}},
	{"onPluginCommand", {}},

	/*** PLAYER ***/
	{"onClientData", {}},
	{"onPlayerModuleList", {}},
	{"onPlayerConnection", {}},
	{"onPlayerConnect", {}},
	{"onPlayerDisconnect", {}},
	{"onPlayerRequestClass", {}},
	{"onPlayerRequestSpawn", {}},
	{"onPlayerSpawn", {}},
	{"onPlayerWasted", {}},
	{"onPlayerKill", {}},
	{"onPlayerUpdate", {}},
	{"onPlayerRequestEnterVehicle", {}},
	{"onPlayerEnterVehicle", {}},
	{"onPlayerExitVehicle", {}},
	{"onPlayerNameChange", {}},
	{"onPlayerStateChange", {}},
	{"onPlayerActionChange", {}},
	{"onPlayerFireChange", {}},
	{"onPlayerCrouchChange", {}},
	{"onPlayerGameKeysChange", {}},
	{"onPlayerBeginTyping", {}},
	{"onPlayerFinishTyping", {}},
	{"onPlayerAwayChange", {}},
	{"onPlayerMessage", {}},
	{"onPlayerCommand", {}},
	{"onPlayerPM", {}},
	{"onPlayerKeyDown", {}},
	{"onPlayerKeyUp", {}},
	{"onPlayerSpectate", {}},
	{"onPlayerCrashReport", {}},

	/*** VEHICLES ***/
	{"onVehicleUpdate", {}},
	{"onVehicleExplode", {}},
	{"onVehicleRespawn", {}},

	/*** OBJECTS ***/
	{"onObjectShot", {}},
	{"onObjectTouch", {}},

	/*** PICKUPS ***/
	{"onPickupPickAttempt", {}},
	{"onPickupPicked", {}},
	{"onPickupRespawn", {}},

	/*** CHECKPOINTS ***/
	{"onCheckpointEnter", {}},
	{"onCheckpointExit", {}},

	/*** MISC ***/
	{"onEntityPoolChange", {}},
	{"onServerPerformanceReport", {}},
};

bool EventManager::m_bWasEventCancelled = false;

void EventManager::Init(sol::state* Lua) {
	sol::usertype<EventManager> userdata = Lua->new_usertype<EventManager>("Event");

	userdata["create"] = &EventManager::create;
	userdata["trigger"] = &EventManager::trigger;

	userdata["bind"] = &EventManager::bind;
	userdata["unbind"] = &EventManager::unbind;
	userdata["cancel"] = &EventManager::cancel;
}

void EventManager::Reset() {
	for (auto& [eventKey, eventHandlers] : m_Handlers)
		eventHandlers.clear();
}

void EventManager::Trigger(const std::string& eventName) {
	// This is used by the plugin itself not by Lua
	if (eventExists(eventName)) {
		auto handlers = GetHandlers(eventName);
		if(handlers.size() > 0)
			for (auto fn : handlers) {
				fn();
				if (EventManager::m_bWasEventCancelled) {
					EventManager::cancelEvent();
					break;
				}
			}
	}
}

const std::vector<sol::function>& EventManager::GetHandlers(std::string eventName) {
	return m_Handlers[eventName];
}

bool EventManager::eventExists(const std::string& eventName) {
	auto eventExists = m_Handlers.find(eventName);
	if (eventExists != m_Handlers.end())
		return true;
	return false;
}

bool EventManager::create(std::string eventName) {
	if (eventExists(eventName)) {
		spdlog::error("Event Manager :: A custom event named {} already exists", eventName);
		return false;
	}
	m_Handlers[eventName] = {};
	return true;
}

bool EventManager::trigger(std::string eventName, sol::variadic_args args) {
	if (!eventExists(eventName)) {
		spdlog::error("Event Manager :: A custom event with the name {} does not exist", eventName);
		return false;
	}

	auto handlers = GetHandlers(eventName);
	if (handlers.size() == 0) return true;

	std::vector<sol::object> largs(args.begin(), args.end());
	for (auto fn : handlers) {
		fn(sol::as_args(largs));
		if (EventManager::m_bWasEventCancelled) {
			EventManager::cancelEvent();
			break;
		}
	}

	return true;
}

bool EventManager::bind(std::string eventName, sol::function handler) {
	if (!eventExists(eventName)) {
		spdlog::error("Event Manager :: No such event named {} exists", eventName);
		return false;
	}

	for (const auto& handle : m_Handlers[eventName]) {
		if (handle.pointer() == handler.pointer()) {
			spdlog::error("Event Manager :: Function handler for event {} is already bound to the given function!", eventName);
			return false;
		}
	}
	m_Handlers[eventName].push_back(handler);
	return true;
}

bool EventManager::unbind(std::string eventName, sol::function handler) {
	if (!eventExists(eventName)) {
		spdlog::error("Event Manager :: No such named {} exists", eventName);
		return false;
	}

	for (auto it = m_Handlers[eventName].begin(); it != m_Handlers[eventName].end(); it++) {
		if (it->pointer() == handler.pointer()) {
			it = m_Handlers[eventName].erase(it);
			return true;
		}
	}
	spdlog::error("Event Manager :: No function handlers are bound for event {}!", eventName);
	return false;
}

bool EventManager::cancel() {
	if (!m_bWasEventCancelled)
		m_bWasEventCancelled = true;

	return true;
}