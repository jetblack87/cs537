attribute   vec4 vPosition;
attribute   vec3 vNormal;

// output values that will be interpretated per-fragment
varying  vec3 fN;
varying  vec3 fE;
varying  vec3 fL0;
varying  vec3 fL1;

uniform mat4 ModelView;
uniform vec4 Light0Position;
uniform vec4 Light1Position;
uniform mat4 Projection;

void main()
{
    fN = vNormal;
    fE = vPosition.xyz;
    fL0 = (ModelView*Light0Position).xyz;
    fL1 = (ModelView*Light1Position).xyz;
    
    if( Light0Position.w != 0.0 ) {
	fL0 = Light0Position.xyz - vPosition.xyz;
    }
    if( Light1Position.w != 0.0 ) {
	fL1 = Light1Position.xyz - vPosition.xyz;
    }

    gl_Position = Projection*ModelView*vPosition;
}
