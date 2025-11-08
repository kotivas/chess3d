#include "cmdsystem.hpp"
#include "logger.hpp"
#include "com/util.hpp"

namespace CMDSystem {
	std::unordered_map<std::string, CVar::cvar_t> g_cvars;

	void Execute(const std::string& name, const std::string& args) {
		auto* cvar = Find(name);
		if (!cvar) return;


		std::visit([&]<typename T>(T& current) {
			if (auto parsed = Util::TryParse<T>(args)) {
				cvar->set(*parsed);
			} else {
				Log::Warning("Failed to parse args <" + args + ">");
			}
		}, cvar->val);
	}

	void Register(const CVar::cvar_t& cvar) {
		g_cvars.emplace(cvar.name, cvar);
	}

	// --- float ---
	void Register(const std::string& name, const std::string& desc, float& target, float min,
	              float max) {
		Register(CVar::CVar{
			name,
			target,
			[&target](const CVar::CVar& cvar) { target = std::get<float>(cvar.val); },
			desc,
			min,
			max
		});
	}

	// --- int ---
	void Register(const std::string& name, const std::string& desc, int& target, int min,
	              int max) {
		Register(CVar::CVar{
			name,
			target,
			[&target](const CVar::CVar& cvar) { target = std::get<int>(cvar.val); },
			desc,
			(min),
			(max)
		});
	}

	// --- bool ---
	void Register(const std::string& name, const std::string& desc, bool& target) {
		Register(CVar::CVar{
			name,
			target,
			[&target](const CVar::CVar& cvar) { target = std::get<bool>(cvar.val); },
			desc
		});
	}

	// --- vec2 ---
	void Register(const std::string& name, const std::string& desc, std::array<float, 2>& target) {
		Register(CVar::CVar{
			name,
			target,
			[&target](const CVar::CVar& cvar) { target = std::get<std::array<float, 2>>(cvar.val); },
			desc
		});
	}

	// --- vec3 ---
	void Register(const std::string& name, const std::string& desc, std::array<float, 3>& target) {
		Register(CVar::CVar{
			name,
			target,
			[&target](const CVar::CVar& cvar) { target = std::get<std::array<float, 3>>(cvar.val); },
			desc
		});
	}

	// --- vec4 ---
	void Register(const std::string& name, const std::string& desc, std::array<float, 4>& target) {
		Register(CVar::CVar{
			name,
			target,
			[&target](const CVar::CVar& cvar) { target = std::get<std::array<float, 4>>(cvar.val); },
			desc
		});
	}

	void Register(const std::string& name, const std::string& desc, glm::vec3& target) {
		Register(CVar::CVar{
			name,
			std::array{target.x, target.y, target.z},
			[&target](const CVar::CVar& cvar) {
				const auto arr = std::get<std::array<float, 3>>(cvar.val);
				target.x = arr[0];
				target.y = arr[1];
				target.z = arr[2];
			},
			desc
		});
	}

	void Unregister(const std::string& name) {
		g_cvars.erase(name);
	}

	CVar::cvar_t* Find(const std::string& name) {
		const auto it = g_cvars.find(name);
		if (it != g_cvars.end()) {
			return &it->second;
		}
		Log::Warning("CVarSys::Find: unable to find " + name);
		return nullptr;
	}
}
