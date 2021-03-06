// Copyright 2006-2016 Coppelia Robotics GmbH. All rights reserved. 
// marc@coppeliarobotics.com
// www.coppeliarobotics.com
// 
// -------------------------------------------------------------------
// THIS FILE IS DISTRIBUTED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
// WARRANTY. THE USER WILL USE IT AT HIS/HER OWN RISK. THE ORIGINAL
// AUTHORS AND COPPELIA ROBOTICS GMBH WILL NOT BE LIABLE FOR DATA LOSS,
// DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF LOSS WHILE USING OR
// MISUSING THIS SOFTWARE.
// 
// You are free to use/modify/distribute this file for whatever purpose!
// -------------------------------------------------------------------
//
// This file was automatically created for V-REP release V3.3.1 Rev1 on May 17th 2016

// Make sure to have the server side running in V-REP!
// Start the server from a child script with following command:
// simExtRemoteApiStart(portNumber) -- starts a remote API server service on the specified port

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <iostream>
#define DEBUG
#include "robot.h"

extern "C" {
    #include "extApi.h"
}

int MoveandRotate(float LinVel, float AngVel, float radius, float lengthWheelAxis, int clientID, int leftMotorHandle, int rightMotorHandle)
{
       float leftMotorAngVel=(LinVel-AngVel*(lengthWheelAxis/2))/radius;
       float rightMotorAngVel=(LinVel+AngVel*(lengthWheelAxis/2))/radius;
       std::cout << AngVel << ", " << leftMotorAngVel << ", " << rightMotorAngVel << "\n";
       simxSetJointTargetVelocity(clientID,leftMotorHandle,leftMotorAngVel,simx_opmode_oneshot);			
       simxSetJointTargetVelocity(clientID,rightMotorHandle,rightMotorAngVel,simx_opmode_oneshot);
}

int MovetoPoint(float *GoalPosition, float minDistance, int clientID, int leftMotorHandle, int rightMotorHandle, int cuboidHandle)
{
	float radius=0.25;
	float axis=0.5;
	float P=0.7;
	float LinVel=0.3;
	float AngVel;

	float ObjectPosition[3];
	float ObjectOrientation[3]; //rotation about Z --> ObjectOrientation[2]
	float OrientationError;

	simxGetObjectPosition(clientID,cuboidHandle,-1,ObjectPosition,simx_opmode_oneshot_wait);
	
	float distance=sqrt(pow(ObjectPosition[0]-GoalPosition[0],2)+pow(ObjectPosition[1]-GoalPosition[1],2)+pow(ObjectPosition[2]-GoalPosition[2],2));
	auto snow = std::chrono::system_clock::now();
	auto sduration = snow.time_since_epoch();
	auto smillis = std::chrono::duration_cast<std::chrono::milliseconds>(sduration).count();
	
	while (distance>minDistance)
	{
		simxGetObjectOrientation(clientID, cuboidHandle, -1, ObjectOrientation, simx_opmode_streaming);
		simxGetObjectPosition(clientID,cuboidHandle,-1,ObjectPosition,simx_opmode_oneshot_wait);


		OrientationError=orientationError(ObjectPosition[0], ObjectPosition[1], ObjectOrientation[2], GoalPosition[0], GoalPosition[1]);
		
		AngVel=P*OrientationError;
		MoveandRotate(LinVel, AngVel, radius, axis, clientID, leftMotorHandle, rightMotorHandle);
		printf("Distance: %f Robot: %f Error: %f\n", distance, ObjectOrientation[2], OrientationError); 

		distance=sqrt(pow(ObjectPosition[0]-GoalPosition[0],2)+pow(ObjectPosition[1]-GoalPosition[1],2));
		
		//printing data to file
		
	
		
		
		auto now = std::chrono::system_clock::now();
		auto duration = now.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		
		//std::cout << millis-smillis << "\t" << ObjectPosition[0] << "\t" << ObjectPosition[1] << std::endl;
		
		
	}
	MoveandRotate(0, 0, radius, axis, clientID, leftMotorHandle, rightMotorHandle);
}



int main(int argc,char* argv[])
{
	int portNb=0;
	int leftMotorHandle;
	int rightMotorHandle;
	int cuboidHandle;
	int goalHandle;

	if (argc>=6)
	{
		portNb=atoi(argv[1]);
		leftMotorHandle=atoi(argv[2]);
		rightMotorHandle=atoi(argv[3]);
		cuboidHandle=atoi(argv[4]);
		goalHandle=atoi(argv[5]);
	}
	else
	{
		extApi_sleepMs(5000);
		return 0;
	}

	int clientID=simxStart((simxChar*)"127.0.0.1",portNb,true,true,2000,5);
	if (clientID!=-1)
	{
		int driveBackStartTime=-99000;
		float motorSpeeds[2];
		float leftMotorAngle;
		float ObjectPosition[3];
		float GoalPosition[3];
		float ObjectOrientation[3];
		float minDistance=0.3;

		if (simxGetConnectionId(clientID)!=-1)
		{  
			simxUChar sensorTrigger=0;

			simxGetObjectHandle(clientID, argv[2], &leftMotorHandle, simx_opmode_blocking);
            simxGetObjectHandle(clientID, argv[3], &rightMotorHandle, simx_opmode_blocking);
			simxGetObjectHandle(clientID, argv[4], &cuboidHandle, simx_opmode_blocking);
			simxGetObjectHandle(clientID, argv[5], &goalHandle, simx_opmode_blocking);


			simxGetObjectPosition(clientID,goalHandle,-1,GoalPosition,simx_opmode_oneshot_wait);
			MovetoPoint(GoalPosition, minDistance, clientID, leftMotorHandle, rightMotorHandle, cuboidHandle);
			simxSetJointTargetVelocity(clientID,leftMotorHandle,0,simx_opmode_blocking);			
      		simxSetJointTargetVelocity(clientID,rightMotorHandle,0,simx_opmode_blocking);
			extApi_sleepMs(5);

		}
		printf("Fin!\n");
		simxFinish(clientID);
	}
	return(0);
}

