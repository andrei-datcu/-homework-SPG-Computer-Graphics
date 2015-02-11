#version 330

layout(location = 0) in vec3 in_position;		
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 model_matrix, view_matrix, projection_matrix;
uniform float time;

out vec2 texcoord;

void main(){

	texcoord = in_texcoord;

	const float amplitude = 5;
	const float frequency = 0.02;

	vec3 pos = in_position;

	pos.z = pos.z + amplitude * sin(0.2* pos.x + frequency * time);

	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(pos,1); 
}
