attribute   vec4 vPosition;
attribute   vec3 vNormal;

// output values that will be interpretated per-fragment
varying  vec3 fN;
varying  vec3 fE;
varying  vec3 fL;
varying  vec4 color;

uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform mat4 Projection;
uniform bool FlatShading;
uniform vec4 MaterialDiffuse;
uniform vec4 LightDiffuse;

void main()
{
    fN = vNormal;
    fE = vPosition.xyz;
    fL = (ModelView*LightPosition).xyz;
    
    if( LightPosition.w != 0.0 ) {
	fL = LightPosition.xyz - vPosition.xyz;
    }

    if (FlatShading) {
        color = (MaterialDiffuse * LightDiffuse *
                           max(dot(vec4(vNormal,1.0),
                                   normalize(LightPosition)),
                               0.0));
    }

    gl_Position = Projection*ModelView*vPosition;
}
