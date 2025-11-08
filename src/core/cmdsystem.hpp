#pragma once
#include <glm/vec3.hpp>
#include "cvar.hpp"

namespace CMDSystem {
	void Register(const CVar::cvar_t& cvar);
	void Register(const std::string& name, const std::string& desc, float& target, float min = 0.f, float max = 1.f);
	void Register(const std::string& name, const std::string& desc, int& target, int min = 0, int max = 255);
	void Register(const std::string& name, const std::string& desc, bool& target);
	void Register(const std::string& name, const std::string& desc, std::array<float, 2>& target);
	void Register(const std::string& name, const std::string& desc, std::array<float, 3>& target);
	void Register(const std::string& name, const std::string& desc, glm::vec3& target);
	void Register(const std::string& name, const std::string& desc, std::array<float, 4>& target);

	void Unregister(const std::string& name);
	CVar::cvar_t* Find(const std::string& name);

	void Execute(const std::string& name, const std::string& args);

	extern std::unordered_map<std::string, CVar::cvar_t> g_cvars;
}
