#pragma once

class Game {
public:

	bool isRunning() const { return _running; };
	void update(double dt);

private:

	bool _running;
};