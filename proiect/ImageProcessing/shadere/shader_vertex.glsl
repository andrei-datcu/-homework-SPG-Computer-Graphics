#version 330

//PRIMESC COORDONATELE IN NDC !!!
layout(location = 0) in vec3 in_position;		
layout(location = 2) in vec2 in_texcoord;

out vec2 texcoord;
uniform mat4 view_matrix, projection_matrix;

void main(){

	texcoord = in_texcoord;

	gl_Position = projection_matrix*view_matrix*vec4(in_position,1); 
}
