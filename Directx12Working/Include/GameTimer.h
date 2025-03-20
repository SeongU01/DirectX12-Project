//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H
#include "Base.h"
class GameTimer :public Base, public SingleTon<GameTimer>
{
public:
  GameTimer();

  float TotalTime() const; // in seconds
  float DeltaTime() const; // in seconds

  void Reset(); // Call before message loop.
  void Start(); // Call when unpaused.
  void Stop();  // Call when paused.
  void Tick();  // Call every frame.

private:
  double mSecondsPerCount;
  double mDeltaTime;

  __int64 mBaseTime;
  __int64 mPausedTime;
  __int64 mStopTime;
  __int64 mPrevTime;
  __int64 mCurrTime;

  bool mStopped;

  // Base을(를) 통해 상속됨
  void Free() override;
};

#endif // GAMETIMER_H