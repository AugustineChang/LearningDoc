#ifndef GAME_TIMER_H
#define GAME_TIMER_H

class GameTimer
{
public:
	GameTimer();
	~GameTimer();

	void Pause();
	void UnPause();
	bool isPaused() const;
	void SetTimeScale( float scale );
	void Tick();
	void Reset();
	float DeltaTime() const;
	float TotalTime() const;

private:
	double secondsPerCount;
	double deltaTime;
	double totalTime;
	__int64 currCount;
	__int64 prevCount;
	float timerScale;
	bool pauseStatus;
};

#endif // !GAME_TIMER_H