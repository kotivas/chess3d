#pragma once
#include <array>
#include <functional>
#include <string>
#include <utility>
#include <variant>

namespace CVar {
	struct CVar {
		using CVarValue = std::variant<float, bool, int, std::array<float, 2>, std::array<float, 3>, std::array<
			                               float, 4>>;
		typedef std::function<void(const CVar&)> CVarCb;

		std::string name;
		std::string desc;
		CVarValue val;
		CVarValue defVal;
		float minFloat{};
		float maxFloat{};
		CVarCb cb;

		CVar(std::string name, float value, CVarCb onChange, std::string desc = "No description provided",
		     float min = 0.f, float max = 255.f)
			: name(std::move(name)), desc(std::move(desc)), val(value), defVal(value), minFloat(min), maxFloat(max),
			  cb(std::move(onChange)) {}

		CVar(std::string name, int value, CVarCb onChange, std::string desc = "No description provided", int min = 0,
		     int max = 255)
			: name(std::move(name)), desc(std::move(desc)), val(value), defVal(value), minFloat(float(min)),
			  maxFloat(float(max)), cb(std::move(onChange)) {}

		CVar(std::string name, bool value, CVarCb onChange, std::string desc = "No description provided")
			: name(std::move(name)), desc(std::move(desc)), val(value), defVal(value), minFloat(0.f), maxFloat(1.f),
			  cb(std::move(onChange)) {}

		CVar(std::string name, std::array<float, 2> value, CVarCb onChange,
		     std::string desc = "No description provided")
			: name(std::move(name)), desc(std::move(desc)), val(value), defVal(value), cb(std::move(onChange)) {}

		CVar(std::string name, std::array<float, 3> value, CVarCb onChange,
		     std::string desc = "No description provided")
			: name(std::move(name)), desc(std::move(desc)), val(value), defVal(value), cb(std::move(onChange)) {}

		CVar(std::string name, std::array<float, 4> value, CVarCb onChange,
		     std::string desc = "No description provided")
			: name(std::move(name)), desc(std::move(desc)), val(value), defVal(value), cb(std::move(onChange)) {}

		template <typename T>
		void set(T& newVal) {
			T clamped = newVal;

			auto clampVal = [&](float val) {
				if (minFloat) val = std::max(val, minFloat);
				if (maxFloat) val = std::min(val, maxFloat);
				return val;
			};

			if constexpr (std::is_same_v<T, int>) {
				clamped = static_cast<int>(clampVal(static_cast<float>(clamped)));
			} else if constexpr (std::is_same_v<T, float>) {
				clamped = clampVal(clamped);
			} else if constexpr (std::is_same_v<T, std::array<float, 2>> ||
				std::is_same_v<T, std::array<float, 3>> ||
				std::is_same_v<T, std::array<float, 4>>) {
				for (auto& elem : clamped)
					elem = clampVal(elem);
			}

			val = clamped;

			exec();
		}

		template <typename T>
		T get() const {
			return std::get<T>(val);
		}

		void exec() const {
			if (cb) cb(*this);
		}
	} typedef cvar_t;

	inline std::unordered_map<std::string, cvar_t> g_cvars;

}
