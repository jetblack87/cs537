attribute  vec4 vPosition;

uniform mat4 Projection;
uniform mat4 ModelView;

void main() 
{
    gl_Position = Projection * ModelView * vPosition;
} 
