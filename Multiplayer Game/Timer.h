#ifndef __TIMER_H__
#define __TIMER_H__


class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	void Reset();
	void Pause();
	void Resume();

	int Read() const; // 
	float ReadSeconds() const;

private:

	bool	running = false;
	bool	reset = false;
	int	started_at;
	int	stopped_at;
	int time_paused;
};

#endif //__TIMER_H__