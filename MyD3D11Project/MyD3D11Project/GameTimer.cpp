#include "GameTimer.h"
#include "SimpleMath.h"
#include <windows.h>

GameTimer::GameTimer() :
	secondsPerCount( 0.0 ) , deltaTime( 0.0 ) , timerScale( 1.0f ) ,
	currCount( 0 ) , prevCount( 0 ) , totalTime( 0.0 ) , pauseStatus( false )
{
	__int64 countsPerSec;
	QueryPerformanceFrequency( (LARGE_INTEGER*) &countsPerSec );
	secondsPerCount = 1.0 / countsPerSec;
}


GameTimer::~GameTimer()
{
}

void GameTimer::Pause()
{
	pauseStatus = true;
}

void GameTimer::UnPause()
{
	__int64 count;
	QueryPerformanceCounter( (LARGE_INTEGER*) &count );

	prevCount = count;
	pauseStatus = false;
}

bool GameTimer::isPaused() const
{
	return pauseStatus;
}

void GameTimer::SetTimeScale( float scale )
{
	if ( scale > 0.0f && timerScale <= 0.0f )//restart
	{
		__int64 count;
		QueryPerformanceCounter( (LARGE_INTEGER*) &count );

		prevCount = count;
	}

	timerScale = SimpleMath::Clamp<float>( scale , 0.0f , 1.0f );
}

void GameTimer::Tick()
{
	if ( pauseStatus || timerScale <= 0.0f )
	{
		deltaTime = 0.0f;
		return;
	}

	__int64 count;
	QueryPerformanceCounter( (LARGE_INTEGER*) &count );
	currCount = count;

	if ( prevCount != 0 )
	{
		deltaTime = ( currCount - prevCount ) * secondsPerCount * timerScale;
	}

	prevCount = currCount;

	if ( deltaTime < 0.0 )deltaTime = 0.0;
	totalTime += deltaTime;
}

void GameTimer::Reset()
{
	__int64 count;
	QueryPerformanceCounter( (LARGE_INTEGER*) &count );

	currCount = count;
	prevCount = count;
	totalTime = 0.0;
	timerScale = 1.0f;
}

float GameTimer::DeltaTime() const
{
	return (float) deltaTime;
}

float GameTimer::TotalTime() const
{
	return (float) totalTime;
}
