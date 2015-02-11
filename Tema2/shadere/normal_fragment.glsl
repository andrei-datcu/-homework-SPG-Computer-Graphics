#version 330
layout(location = 0) out vec4 out_color;

uniform samplerCube texture_cubemap;
in vec3 texcoord;

void main(){

	float darken_factor = 0.8f;
    vec3 tex = texture(texture_cubemap, texcoord).xyz;
	out_color = vec4(tex * darken_factor, 1) ;
}