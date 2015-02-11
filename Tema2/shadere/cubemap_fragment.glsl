#version 330
layout(location = 0) out vec3 out_color;

uniform samplerCube texture_cubemap;
uniform vec3 camera_position;
in vec3 world_position;
in vec3 world_normal;

vec3 myReflect(){
	vec3 r = normalize(reflect(normalize(world_position-camera_position), normalize(world_normal)));
	return texture(texture_cubemap, r).xyz;
}

void main(){
	
	const float darken_factor = 0.5;
    out_color = myReflect() * darken_factor;
}