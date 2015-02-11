#version 430
layout(location = 0) out vec4 out_color;

uniform sampler2D textura;

in vec2 g_texcoord;
void main(){

	vec3 tex1 = texture(textura, g_texcoord).rgb;
	out_color = vec4(tex1 * 0.45, 1);
}