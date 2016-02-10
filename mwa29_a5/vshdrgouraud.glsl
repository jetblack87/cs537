#version 120

// vertex shader
attribute vec3 vPosition;
attribute vec3 vNormal;
varying vec3 color;  //vertex shade
// Light and material properties. Light color * surface color
uniform vec3 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec3 LightPosition;
uniform float Shininess;
void main()
{
  // Transform vertex  position into eye coordinates
  vec3 pos = (ModelView * vec4(vPosition,1.0)).xyz;
  // Light defined in camera frame
  vec3 L = normalize( LightPosition - pos );
  vec3 E = normalize( -pos );
  vec3 H = normalize( L + E );
  // Transform vertex normal into eye coordinates
  vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
  // Compute terms in the illumination equation
  vec3 ambient = AmbientProduct;
  float Kd = max( dot(L, N), 0.0 );
  vec3  diffuse = Kd*DiffuseProduct;
  float Ks = pow( max(dot(N, H), 0.0), Shininess );
  vec3  specular = Ks * SpecularProduct;
  if( dot(L, N) < 0.0 )  specular = vec3(0.0, 0.0, 0.0);
  gl_Position = Projection * ModelView * vec4(vPosition, 1.0);
  color = ambient + diffuse + specular;
}
