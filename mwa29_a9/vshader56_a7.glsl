attribute   vec4 vPosition;
attribute   vec3 vNormal;
attribute  vec2 vTexCoord;

// output values that will be interpretated per-fragment
varying  vec3 fN;
varying  vec3 fE;
varying  vec3 fL;
varying  vec4 color;
varying vec2 texCoord;
varying  float SelectedControlPoint;

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

    texCoord    = vTexCoord;

    gl_Position = Projection*ModelView*vPosition;
    if (vPosition.w == 1.2345) {
      SelectedControlPoint = 1.0;
    } else {
      SelectedControlPoint = 0.0;
    }
}
