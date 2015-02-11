#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in float in_texcoord;
layout(location = 2) in vec3 in_normal;

out float p_texcoord;
out vec3 p_normal;

uniform mat4 model_matrix;
//uniform mat4 view_matrix, projection_matrix;

void main()
{
	gl_Position = model_matrix*vec4(in_position,1);
	p_texcoord = in_texcoord; 
	p_normal = in_normal;
}