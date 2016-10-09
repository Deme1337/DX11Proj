#pragma once
#ifndef TIMER_H
#define TIMER_H




class Timer
{
public:
	Timer();
	static void StartTimer();
	static double GetTime();
	static double GetFrameTime();
	static void SetDeltaTime(double dt);
	static double GetDeltaTime();
	
	~Timer();

private:


};
static double countsPerSecond = 0;
static double frameTimeOld = 0;
static double CounterStart = 0;
static double DeltaTime = 0;

static double MeshRenderTime = 0;

#endif