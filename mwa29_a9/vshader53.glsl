#version 120

attribute vec4 vPosition;
attribute vec3 vNormal;
varying   vec4 color;

uniform mat4 ModelView;
uniform mat4 Projection;

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

void main()
{
  gl_Position = Projection * ModelView * vPosition;

  // Transform vertex  position into eye coordinates
  vec3 pos = (ModelView * vPosition).xyz;

  // Transform vertex normal into eye coordinates
  vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;

  vec4 ambient = vec4(0.0);
  vec4 diffuse = vec4(0.0);
  vec4 specular = vec4(0.0);


  // Light0
  vec4 ambient0_product  = Light0Ambient  * MaterialAmbient;
  vec4 diffuse0_product  = Light0Diffuse  * MaterialDiffuse;
  vec4 specular0_product = Light0Specular * MaterialSpecular;

  vec3 L0 = normalize( (ModelView*Light0Position).xyz - pos );
  vec3 E0 = normalize( -pos );
  vec3 H0 = normalize( L0 + E0 );

  // Compute terms in the illumination equation
  ambient += ambient0_product;

  float Kd0 = max( dot(L0, N), 0.0 );
  diffuse += Kd0*diffuse0_product;

  float Ks0 = pow( max(dot(N, H0), 0.0), Shininess );
  specular += Ks0 * specular0_product;

  if( dot(L0, N) < 0.0 ) {
    specular = vec4(0.0, 0.0, 0.0, 1.0);
  }
  // End Light0

  // Light1
  vec4 ambient1_product  = Light1Ambient  * MaterialAmbient;
  vec4 diffuse1_product  = Light1Diffuse  * MaterialDiffuse;
  vec4 specular1_product = Light1Specular * MaterialSpecular;

  vec3 L1 = normalize( (ModelView*Light1Position).xyz - pos );
  vec3 E1 = normalize( -pos );
  vec3 H1 = normalize( L1 + E1 );

  // Compute terms in the illumination equation
  ambient += ambient1_product;

  float Kd1 = max( dot(L1, N), 0.0 );
  diffuse += Kd1 * diffuse1_product;

  float Ks1 = pow( max(dot(N, H1), 0.0), Shininess );
  specular += Ks1 * specular1_product;

  if( dot(L1, N) < 0.0 ) {
    specular = vec4(0.0, 0.0, 0.0, 1.0);
  }
  // End Light1



  color = ambient + diffuse + specular;
  color.a = 1.0;
}
