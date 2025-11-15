#pragma once
#include "camera.hpp"

namespace Camera {
	class ICameraController {
	public:
		virtual void update(Camera&, double /*dt*/) = 0;
		virtual void handleControls(Camera&) = 0;

		virtual ~ICameraController() = default;
	};

	class OrbitCameraController final : public ICameraController {
	public:
		explicit OrbitCameraController(glm::vec3 target = {0, 0, 0}, float radius = 1)
			: _radius(radius), _target(target), _lastx(0), _lasty(0) {}

		void update(Camera&, double dt) override;
		void handleControls(Camera&) override;

		glm::vec3 getTarget() { return _target; }
		void setTarget(glm::vec3 target) { _target = target; }

	private:
		float _radius;
		glm::vec3 _target;
		float _lastx;
		float _lasty;
	};

	class FPSCameraController final : public ICameraController {
	public:
		FPSCameraController(float speed)
			: _speed(speed), _lasty(0), _lastx(0) {}

		void update(Camera&, double dt) override;
		void handleControls(Camera&) override;

		float getSpeed() {return _speed;}
		void setSpeed(float speed) {_speed = speed;}

	private:
		float _speed;
		float _lasty;
		float _lastx;
	};
}
