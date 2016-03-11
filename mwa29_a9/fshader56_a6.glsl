// per-fragment interpolated values from the vertex shader
varying  vec3 fN;
varying  vec3 fL0;
varying  vec3 fL1;
varying  vec3 fE;

uniform mat4 ModelView;

uniform vec4 MaterialAmbient;
uniform vec4 MaterialDiffuse;
uniform vec4 MaterialSpecular;
uniform vec4 Light0Position;
uniform vec4 Light0Ambient;
uniform vec4 Light0Diffuse;
uniform vec4 Light0Specular;
uniform vec4 Light1Position;
uniform vec4 Light1Ambient;
uniform vec4 Light1Diffuse;
uniform vec4 Light1Specular;
uniform float Shininess;

varying vec3  MCposition; 

uniform sampler3D texture;
varying vec3 st;

void main() 
{ 
    // Normalize the input lighting vectors
    vec3 N = normalize(fN);
    vec3 E = normalize(fE);

    vec4 ambient = vec4(0.0);
    vec4 diffuse = vec4(0.0);
    vec4 specular = vec4(0.0);

    // Light0
    vec4 ambient0_product  = Light0Ambient  * MaterialAmbient;
    vec4 diffuse0_product  = Light0Diffuse  * MaterialDiffuse;
    vec4 specular0_product = Light0Specular * MaterialSpecular;

    vec3 L0 = normalize(fL0);

    vec3 H0 = normalize( L0 + E );
    
    ambient += ambient0_product;

    float Kd0 = max(dot(L0, N), 0.0);
    diffuse += Kd0*diffuse0_product;
    
    float Ks0 = pow(max(dot(N, H0), 0.0), Shininess);
    specular += Ks0*specular0_product;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L0, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    }
    // End Light0

    // Light1
    vec4 ambient1_product  = Light1Ambient  * MaterialAmbient;
    vec4 diffuse1_product  = Light1Diffuse  * MaterialDiffuse;
    vec4 specular1_product = Light1Specular * MaterialSpecular;

    vec3 L1 = normalize(fL1);

    vec3 H1 = normalize( L1 + E );
    
    ambient += ambient1_product;

    float Kd1 = max(dot(L1, N), 0.0);
    diffuse += Kd1*diffuse1_product;
    
    float Ks1 = pow(max(dot(N, H1), 0.0), Shininess);
    specular += Ks1*specular1_product;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L1, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    }
    // End Light1
    gl_FragColor = ambient + diffuse + specular;
    gl_FragColor = gl_FragColor * texture3D(texture, st);
    gl_FragColor.a = 1.0;
    //gl_FragColor = gl_FragColor * vec4(color,1.0);
} 
