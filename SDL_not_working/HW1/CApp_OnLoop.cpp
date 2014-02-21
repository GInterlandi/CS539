#include "stdafx.h"
#include "CApp.h"

void CApp::OnLoop() {

	std::chrono::time_point<std::chrono::system_clock>  end;
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds;
	int dir=1;

	if(key_z ) 
	{
		elapsed_seconds = end-t_roll;
		// positive or negative rotation depending on upper or lower case letter
		roll = roll + dir*SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_y) 
	{
		elapsed_seconds = end-t_yaw;
		yaw = yaw + dir*SCALE_ANGLE*elapsed_seconds.count();
	}

	if(key_x ) 
	{
		elapsed_seconds = end-t_pitch;
		pitch = pitch + dir*SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_w) 
	{
		elapsed_seconds = end-t_trans;

		eye =Translate(0,0,-SCALE_TRANS*elapsed_seconds.count()) *eye; 
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;

	}
	if(key_s) 
	{
		elapsed_seconds = end-t_trans;
		eye =Translate(0,0,SCALE_TRANS*elapsed_seconds.count()) *eye; 
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;
	}
	if(key_a ) 
	{
		elapsed_seconds = end-t_rot;
		eye =Translate(-SCALE_TRANS*elapsed_seconds.count(),0,0) *eye;
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;
	}
	if(key_d ) 
	{	
		elapsed_seconds = end-t_rot;
		eye =Translate(SCALE_TRANS*elapsed_seconds.count(),0,0) *eye;
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;
	}

	t_roll = end;
	t_pitch = end;
	t_yaw=end;
	t_rot = end;
	t_trans = end;

}