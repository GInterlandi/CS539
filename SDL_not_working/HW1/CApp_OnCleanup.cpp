#include "stdafx.h"
#include "CApp.h"
 
void CApp::OnCleanup() {
	SDL_DestroyWindow( Main_window );
	SDL_GL_DeleteContext(GLcontext);
    SDL_Quit();
}