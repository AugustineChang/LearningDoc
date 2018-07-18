#include "GameTimer.h"
#include "SimpleMath.h"
#include "Callable.h"

#include <windows.h>
#include <sstream>
#include <GL/glut.h>
#include <iostream>
#include <stdlib.h>

GameTimer::GameTimer() :
	secondsPerCount( 0.0 ) , deltaTime( 0.0 ) , timerScale( 1.0f ) , currCount( 0 ) , prevCount( 0 ) ,
	 totalTime( 0.0 ) , pauseStatus( false ) , timer( 0.0f ) , timerInterval( 0.0f )
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

	deltaTime = ( currCount - prevCount ) * secondsPerCount * timerScale;
	prevCount = currCount;

	if ( deltaTime < 0.0 )deltaTime = 0.0;
	totalTime += deltaTime;

	//Timer
	if ( timerInterval > 0.0f )
	{
		timer += deltaTime;
		if ( timer >= timerInterval )
		{
			timer -= timerInterval;
			if ( callback != nullptr )
			{
				( *callback )( );
			}
		}
	}
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

void GameTimer::CalcFrameStat()
{
	fpsCount++;

	float total_float = float( totalTime );
	float passedTime = total_float - fpsBaseTime;
	if ( passedTime >= 1.0f )
	{
		float fps = fpsCount / passedTime;
		float mspf = passedTime / fpsCount * 1000.0f;

		std::ostringstream ss;
		ss.setf( std::ios::fixed , std::ios::floatfield );
		ss.precision( 2 );
		if ( fps > 1000.0f )
		{
			ss << "FPS:" << ( fps / 1000.0f ) << "K ";
			ss << "FrameTime:" << ( mspf * 1000.0f ) << "us ";
		}
		else
		{
			ss << "FPS:" << fps << " ";
			ss << "FrameTime:" << mspf << "ms ";
		}
		glutSetWindowTitle( ss.str().c_str() );

		fpsCount = 0;
		fpsBaseTime = total_float;
	}
}

void GameTimer::SetTimer( float interval , Callable *cb )
{
	timerInterval = interval;
	callback = cb;
}

void GameTimer::DelTimer()
{
	timerInterval = 0.0f;
	callback = nullptr;
}

void GameTimer::beginDebugTime()
{
	debugStart = clock();
}

void GameTimer::endDebugTime()
{
	debugEnd = clock();
}

void GameTimer::showDebugTime( const char *str )
{
	std::cout << str << debugEnd - debugStart << std::endl;
}

void GameTimer::clearDebug()
{
	system( "cls" );
}
