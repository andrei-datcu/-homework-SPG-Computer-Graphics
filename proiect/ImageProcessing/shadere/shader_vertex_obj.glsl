#version 330

//3 variante: layout location pentru varianta 1 (Cea mai buna), layout location sau doar in/attribute pentru varianta 2, doar in pentru varianta 3.
//attribute e sintaxa veche (OpenGL 2) e recomandat sa folositi in

layout(location = 0) in vec3 in_position;		
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

//attribute vec3 in_position;					
//attribute vec3 in_color;								

//in vec3 in_color;
//in vec3 in_position;

uniform mat4 model_matrix, view_matrix, projection_matrix;

out vec3 world_pos;
out vec3 world_normal;
out vec2 texcoord;

void main(){

	world_pos = vec3(model_matrix * vec4(in_position, 1)); // pozitia in coordonate word
	world_normal = normalize(mat3(model_matrix) * in_normal);
	texcoord = in_texcoord;

	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(in_position,1); 
}
