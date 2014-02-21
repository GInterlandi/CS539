#include "stdafx.h"
#include "CApp.h"

// Camera Coordianate System


void CApp::OnRender() {

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mat4 modelViewCamera = LookAt(eye, eye-n, v);

	glUseProgram(program);

	// Bind the Ka/Kd/Ks to the shader
	glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),1, ambient_product);
	glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),1, diffuse_product);
	glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),1,specular_product);	
	glUniform1f( glGetUniformLocation(program, "mat_shininess"), material_shininess);
	glUniform4fv( glGetUniformLocation(program, "lightPosition"),1, light_position);

	// Retrieve transformation uniform variable locations
	ModelViewCam = glGetUniformLocation( program, "modelView" );
	ModelViewObj = glGetUniformLocation(program, "ModelViewObj");
	Projection = glGetUniformLocation( program, "projection" );
	NormalTransformation = glGetUniformLocation( program, "normalTransformation" );

	glUniformMatrix4fv(ModelViewCam, 1, GL_TRUE, modelViewCamera);

	mat4 modelViewObject = RotateZ(roll)*RotateY(yaw)*RotateX(pitch)*Scale(1.0, 2.2, 1.0);
	mat4 normalMatrix = modelViewCamera*RotateZ(roll)*RotateY(yaw)*RotateX(pitch)*Scale(1.0, 1.0/2.2, 1.0);
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);

	glBindVertexArray( vao );
	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	SDL_GL_SwapWindow(Main_window);

	glUseProgram(0);
	glBindVertexArray(0);


}