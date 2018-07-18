#ifndef GAME_TIMER_H
#define GAME_TIMER_H
#include <time.h>

class Callable;

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

	void CalcFrameStat();

	void SetTimer( float interval , Callable *cb );
	void DelTimer();

	void beginDebugTime();
	void endDebugTime();
	void showDebugTime( const char *str );
	void clearDebug();

private:
	double secondsPerCount;
	double deltaTime;
	double totalTime;
	__int64 currCount;
	__int64 prevCount;
	float timerScale;
	bool pauseStatus;

	int fpsCount;
	float fpsBaseTime;

	Callable *callback;
	double timerInterval;
	double timer;

	clock_t debugStart;
	clock_t debugEnd;
};

#endif // !GAME_TIMER_H