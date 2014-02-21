vec4 vPInst;

attribute   vec4 vPosition;
attribute   vec3 vNormal;
attribute   vec3 vShift;

varying  vec3 fN;
varying  vec3 fE;
varying  vec3 fL;

uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform mat4 Projection;

void main()
{
  
    fN = vNormal;
    
    vPInst = vPosition;

    vPInst.xyz = vShift.z*vPInst.xyz;
    
    //Ground The Scaled Instance
    vPInst.y += -1.0 + vShift.z; 
    
    
    vPInst.x += vShift.x;
    vPInst.z -= vShift.y;
    
    fE = vPInst.xyz;
    fL = LightPosition.xyz;
    
    if( LightPosition.w != 0.0 ) {
	fL = LightPosition.xyz - vPInst.xyz;
    }

   
    gl_Position = Projection*ModelView*vPInst;
}
