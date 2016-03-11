#version 120

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

// assignment 9
varying vec4 vPosition_world;
uniform float scale = 1;

// Taken from https://github.com/ashima/webgl-noise
//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }

/// END Ashima Arts code



vec4
threeDTexture(vec4 pos) {
  vec3 v_objCoords = vec3(pos.x, pos.y, pos.z);
  float value = snoise( scale*sin(v_objCoords) );
  value = 0.75 + value*0.25; // map to the range 0.5 to 1.0
  return vec4(1.0,value,1.0,1.0);
}

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

    // assignment 9
    diffuse = diffuse * threeDTexture(vPosition_world);

    gl_FragColor = ambient + diffuse + specular;
    gl_FragColor.a = 1.0;
} 

