#include "Sky.hpp"

#include <glm/glm.hpp>
#include <array>
#include <cmath>
#include <algorithm>
#include <utility>
#include <glad/glad.h>
#include "Common/HosekDataset.hpp"

namespace Renderer {
	inline constexpr std::array ALBEDO = {0.1f, 0.1f, 0.1f};

	void Sky::setup() {
		const float quadVertices[] = {
			-1.f, 1.f,
			-1.f, -1.f,
			1.f, 1.f,
			1.f, -1.f
		};

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glBindVertexArray(0);
	}

	Sky::~Sky() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

	inline float EvaluateSpline(const float* dataset, std::size_t start, std::size_t stride, float value) {
		float inv = 1.0f - value;
		return
			(inv * inv * inv * inv * inv) * dataset[start + 0 * stride] +
			(5.0f * inv * inv * inv * inv * value) * dataset[start + 1 * stride] +
			(10.0f * inv * inv * inv * value * value) * dataset[start + 2 * stride] +
			(10.0f * inv * inv * value * value * value) * dataset[start + 3 * stride] +
			(5.0f * inv * value * value * value * value) * dataset[start + 4 * stride] +
			(value * value * value * value * value) * dataset[start + 5 * stride];
	}

	inline float Evaluate(const float* dataset, std::size_t stride, float turbidity, float albedo, float sun_theta) {
		float elevationK = glm::pow(glm::max(0.0f, 1.0f - sun_theta / glm::half_pi<float>()), 1.0f / 3.0f);

		int turbidity0 = glm::clamp(static_cast<int>(std::floor(turbidity)), 1, 10);
		int turbidity1 = std::min(turbidity0 + 1, 10);
		float turbidityK = glm::clamp(turbidity - static_cast<float>(turbidity0), 0.0f, 1.0f);

		std::size_t datasetA0 = 0;
		std::size_t datasetA1 = stride * 6 * 10;

		float a0t0 = EvaluateSpline(dataset, datasetA0 + stride * 6 * (turbidity0 - 1), stride, elevationK);
		float a1t0 = EvaluateSpline(dataset, datasetA1 + stride * 6 * (turbidity0 - 1), stride, elevationK);
		float a0t1 = EvaluateSpline(dataset, datasetA0 + stride * 6 * (turbidity1 - 1), stride, elevationK);
		float a1t1 = EvaluateSpline(dataset, datasetA1 + stride * 6 * (turbidity1 - 1), stride, elevationK);

		return a0t0 * (1.0f - albedo) * (1.0f - turbidityK)
			+ a1t0 * albedo * (1.0f - turbidityK)
			+ a0t1 * (1.0f - albedo) * turbidityK
			+ a1t1 * albedo * turbidityK;
	}

	// hosek-wilkie approximation (component-wise, params is array of 9 vec3)
	glm::vec3 HosekWilkie(float cos_theta, float gamma, float cos_gamma,
	                             const std::array<glm::vec3, 10>& params) {
		const glm::vec3& A = params[0];
		const glm::vec3& B = params[1];
		const glm::vec3& C = params[2];
		const glm::vec3& D = params[3];
		const glm::vec3& E = params[4];
		const glm::vec3& F = params[5];
		const glm::vec3& G = params[6];
		const glm::vec3& H = params[7];
		const glm::vec3& I = params[8];

		glm::vec3 chi = glm::vec3(1.0f + cos_gamma * cos_gamma) /
			glm::pow(H * H + glm::vec3(1.0f) - 2.0f * cos_gamma * H, glm::vec3(1.5f));

		glm::vec3 first = (A * glm::exp(B / glm::vec3(cos_theta + 0.01f))) + glm::vec3(1.0f);
		glm::vec3 second = C + D * glm::exp(E * gamma) + glm::vec3(F * (cos_gamma * cos_gamma)) + G * chi + I *
			glm::sqrt(glm::max(glm::vec3(0.0f), glm::vec3(cos_theta)));

		return first * second;
	}

	// calculates sun direction based on sun position and lut for shader
	void Sky::calculateSun(const glm::vec2& sun_pos) {
		glm::quat qy = glm::angleAxis(sun_pos.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat qx = glm::angleAxis(-sun_pos.x, glm::vec3(1.0f, 0.0f, 0.0f));
		sun_direction = qy * (qx * glm::vec3(0.0f, 0.0f, 1.0f));

		float sun_theta = std::acos(glm::clamp(sun_direction.y, 0.0f, 1.0f));

		for (auto& p : atm_params) p = glm::vec3(0.0f);

		// channel 0..2
		const float* datasets[3] = {
			Hosek::DATASET_RGB1.data(), Hosek::DATASET_RGB2.data(), Hosek::DATASET_RGB3.data()
		};
		const float* datasets_rad[3] = {
			Hosek::DATASET_RGB_RAD1.data(), Hosek::DATASET_RGB_RAD2.data(), Hosek::DATASET_RGB_RAD3.data()
		};

		for (int c = 0; c < 3; ++c) {
			const float* ds = datasets[c];

			atm_params[0][c] = Evaluate(ds + 0, 9, atm_turbidity, ALBEDO[c], sun_theta);
			atm_params[1][c] = Evaluate(ds + 1, 9, atm_turbidity, ALBEDO[c], sun_theta);
			atm_params[2][c] = Evaluate(ds + 2, 9, atm_turbidity, ALBEDO[c], sun_theta);
			atm_params[3][c] = Evaluate(ds + 3, 9, atm_turbidity, ALBEDO[c], sun_theta);
			atm_params[4][c] = Evaluate(ds + 4, 9, atm_turbidity, ALBEDO[c], sun_theta);
			atm_params[5][c] = Evaluate(ds + 5, 9, atm_turbidity, ALBEDO[c], sun_theta);
			atm_params[6][c] = Evaluate(ds + 6, 9, atm_turbidity, ALBEDO[c], sun_theta);
			atm_params[7][c] = Evaluate(ds + 8, 9, atm_turbidity, ALBEDO[c], sun_theta); // swapped
			atm_params[8][c] = Evaluate(ds + 7, 9, atm_turbidity, ALBEDO[c], sun_theta);

			// RAD dataset
			atm_params[9][c] = Evaluate(datasets_rad[c], 1, atm_turbidity, ALBEDO[c], sun_theta);
		}

		glm::vec3 S = HosekWilkie(std::cos(sun_theta), 0.0f, 1.0f, atm_params) * atm_params[9];
		float denom = glm::dot(S, glm::vec3(0.2126f, 0.7152f, 0.0722f));
		if (denom != 0.0f) atm_params[9] /= denom;

		float sun_amount = std::fmod(sun_direction.y / glm::half_pi<float>(), 4.0f);
		if (sun_amount > 2.0f) sun_amount = 0.0f;
		if (sun_amount > 1.0f) sun_amount = 2.0f - sun_amount;
		else if (sun_amount < -1.0f) sun_amount = -2.0f - sun_amount;

		float normalized_sun_y = 0.6f + 0.45f * sun_amount;
		sun_color = glm::normalize(S * sun_amount) * glm::clamp(sun_direction.y, 0.f, 1.f);
		atm_params[9] *= normalized_sun_y;
	}
} // namespace Sky
