#include <ros/ros.h>
#include <pigpiod_if2.h>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

struct timeval tv;

double getTime()
{
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

int coefficient = 17150;
int TRIG = 23; // gpio
int ECHO = 24; // gpio

void initializePins()
{
	gpioSetMode(TRIG, PI_OUTPUT);
	gpioSetMode(ECHO, PI_INPUT);
}

bool waitValue(int value, int limit = 100000)
{
	for(int i = 0; gpioRead(ECHO) == value; ++i)
	{
		if(i >= limit)
			return false;
	}
	return true;
}

double detectDistance()
{
	gpioWrite(TRIG, 0); // reset rangefinder state
	usleep(500000); // wait for reset state -use 10000 to speed up program
	gpioWrite(TRIG, 1); // high state runs rangefinder measurement
	usleep(10); // 10 us of hi level is necessary to start measurement
	gpioWrite(TRIG, 0); // prepare rangefinder for measurement
	
	if(waitValue(0))
	{
		double pulseStart = getTime();

		if(waitValue(1)) // wait until hi level is on echo pin
		{
			double pulseEnd = getTime();

			double duration = pulseEnd - pulseStart;
			double distance = duration * coefficient;

			return distance;
		}
	}

	printf("Measurement error!\n");
	return 0.0 / 0.0;
}

int main(int argc, char* argv[])
{
	ros::init(argc, argv, "ultrasonic_sensor");
	ros::NodeHandle nh;


	if(gpioInitialise() < 0)
	{
		printf("\npigpio failed to initiate\n");
	}
	else
	{
		printf("pigpio initialisation success!\n");
	
		initializePins();

		for(int i = 0; i < 100; ++i) // for example: 100 measurements
		{
			double distance = detectDistance();
			cout << "Distance: " << distance << "cm" << endl;
		}
	
		gpioTerminate();
	
	}

	return 0;
	ros::spin();
}
