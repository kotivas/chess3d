#include "cvar.hpp"
#include "logger.hpp"

namespace CVarSys {
	std::vector<cvar_t> g_cvars;

	void CVar::set(const float& value) {
		if (value >= min && value <= max) {
			val = value;
			cb(value);
		} else {
			Log::Warning("CVarSys::CVar::set: value out of range");
		}
	}

	void CVar::exec() const {
		cb(val);
	}

	void Register(const cvar_t& cvar) {
		g_cvars.push_back(cvar);
	}

	void Register(const std::string& name, const std::string& desc, float val, CVarCb cb, float min,
	              float max, float defVal) {
		Register({name, desc, val, cb, min, max, defVal});
	}

	void Unregister(const std::string& name) {
		for (int i = 0; i < g_cvars.size(); i++) {
			if (g_cvars[i].name == name) {
				g_cvars.erase(g_cvars.begin() + i);
				return;
			}
		}
	}

	CVar* Find(const std::string& name) {
		for (auto& cvar : g_cvars) {
			if (cvar.name == name) {
				return &cvar;
			}
		}
		return nullptr;
	}
}
