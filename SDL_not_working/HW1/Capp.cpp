#include "stdafx.h"
#include "CApp.h"

CApp::CApp() {

	vertices[0] = point4( -0.5, -0.5, 0.5, 1.0 );
	vertices[1] = point4( -0.5, 0.5, 0.5, 1.0 );
	vertices[2] = point4( 0.5, 0.5, 0.5, 1.0 );
	vertices[3] = point4( 0.5, -0.5, 0.5, 1.0 );
	vertices[4] = point4( -0.5, -0.5, -0.5, 1.0 );
	vertices[5] = point4( -0.5, 0.5, -0.5, 1.0 );
	vertices[6] = point4( 0.5, 0.5, -0.5, 1.0 );
	vertices[7] = point4( 0.5, -0.5, -0.5, 1.0 );

	Index = 0; //Cube Variables initialization

	model_view = mat4(1.0);
	projmat    = mat4(1.0); //GL variables initialization

	// Setup some lights
	light_ambient  = vec4 ( 0.5, 0.5, 0.0, 1.0 );
	light_diffuse  = vec4( 1.0, 1.0, 1.0, 1.0 );
	light_specular = vec4( 1.0, 1.0, 1.0, 1.0 );
	//LIGHT POSITION
	light_position = vec4(0.0, 1.0, 3.0, 1.0);

	material_ambient   = vec4( 0.8, 0.36, 0.36, 1.0 );
	material_diffuse   = vec4( 0.88, 0.34, 0.2, 1.0 );
	material_specular  = vec4( 1.0, 0.84, 0.77, 1.0 );
	material_shininess = 100.0;

	ambient_product =light_ambient* material_ambient;
	diffuse_product = light_diffuse* material_diffuse;
	specular_product = light_specular* material_specular; //Lights and materials variables initialization

 key_s=0,key_w=0,key_a=0,key_d=0;
 key_x=0,key_X=0,key_y=0,key_Y=0,key_z=0,key_Z=0; // keys initialization

  u = vec4(1, 0, 0, 0);
 v = vec4(0, 1, 0, 0);
 n = vec4(0, 0, 1, 0);
 eye = vec4(0,0,2,1);
 bases = mat4(n,u,v,vec4(0.0,0.0,0.0,1.0));  //Camera matrices initialization
	dir=1;
	Running = true;
}

int CApp::OnExecute() {
	if(OnInit() == false) {
		return -1;
	}

	SDL_Event Event;

	while(Running) {
		while(SDL_PollEvent(&Event)) {
			OnEvent(&Event);
		}

		OnLoop();
		OnRender();
	}

	OnCleanup();

	return 0;
}

int main(int argc, char* argv[]) {
	CApp theApp;

	return theApp.OnExecute();
}