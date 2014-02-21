#include "stdafx.h"
#include "CApp.h"

void CApp::OnEvent(SDL_Event* Event) {

	if(Event->type == SDL_QUIT) {
		Running = false;
	}
	if(Event->type == SDL_KEYDOWN) {
		switch(Event->key.keysym.sym){
		case SDLK_z: // roll counterclockwise in the xy plane
			if(!key_Z) t_roll = std::chrono::system_clock::now();
			key_Z=1;
			break;
		case SDLK_x: // pitch down
			if(!key_x) t_pitch = std::chrono::system_clock::now();
			key_x=1;
			break;
		case SDLK_y: // yaw down
			if(!key_y) t_yaw = std::chrono::system_clock::now();
			key_y=1;
			break;
		case SDLK_w: // go in front
			if(!key_w) t_trans = std::chrono::system_clock::now();
			key_w=1;
			break;
		case SDLK_s: // go in front
			if(!key_s) t_trans = std::chrono::system_clock::now();
			key_s=1;
			break;
		case SDLK_a: // go in front
			if(!key_a) t_rot = std::chrono::system_clock::now();
			key_a=1;
			break;
		case SDLK_d: // go in front
			if(!key_d) t_rot = std::chrono::system_clock::now();
			key_d=1;
			break;
		case 033: // Escape Key
		case SDLK_q: 
			Running = false;
			break;
		case SDLK_LSHIFT: 
			dir=-1;
			break;
			//case '1':
			//	glActiveTexture( GL_TEXTURE0 );
			//	glBindTexture( GL_TEXTURE_2D, textures[0] );
			//	break;

			//case '2':
			//	glActiveTexture( GL_TEXTURE0 );
			//	glBindTexture( GL_TEXTURE_2D, textures[1] );
			//	break;

			//case '3':
			//	glActiveTexture( GL_TEXTURE0 );
			//	glBindTexture( GL_TEXTURE_2D, textures[2] );
			//	break;
		}
	}

	if(Event->type == SDL_KEYUP) {
		switch(Event->key.keysym.sym){
		case SDLK_z: // roll counterclockwise in the xy plane
			key_z=0;
			break;
		case SDLK_x: // pitch down
			key_x=0;
			break;
		case SDLK_y: // yaw down
			key_y=0;
			break;
		case SDLK_w: // go in front
			key_w=0;
			break;
		case SDLK_s: // go in front
			key_s=0;
			break;
		case SDLK_a: // go in front
			key_a=0;
			break;
		case SDLK_d: // go in front
			key_d=0;
			break;
		case 033: // Escape Key
		case 'q': case 'Q':
			Running = false;
			break;
		case SDLK_LSHIFT: 
			dir=1;
			break;
		}
	}
}