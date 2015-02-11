#version 330
layout(location = 0) out vec4 out_color;


in vec2 texcoord;
uniform	sampler2D texUnit;

void main(){

	out_color = texture(texUnit, texcoord);// * light;
}