#version 330

//OUTPUTEAZA LA FRAMEBUFFERUL DEFAULT (cel de la freeglut)
layout(location = 0) out vec4 out_color;

uniform sampler2D textura_color;
uniform int screen_width, screen_height;
uniform int shader_based;
uniform int do_blur;
uniform int do_grayscale;

in vec2 texcoord;


vec3 color(){
	return texture(textura_color, texcoord).xyz;
}

vec3 grayscale()
{
	vec3 c = texture(textura_color, texcoord).xyz;
	float r = c.x;
	float g = c.y;
	float b = c.z;
	float gray = 0.21 * r + 0.71 * g + 0.07 * b; 
	return vec3(gray,gray,gray);
}

vec3 blur(){
	float dx = 1.0f/screen_width;
	float dy = 1.0f/screen_height;
	vec3 sum = vec3(0,0,0);
	for(int i=0;i<5;i++) for(int j=0;j<5;j++) sum += texture(textura_color, texcoord + vec2((i-2)*dx, (j-2)*dy)).xyz;
	return sum/25;
}


void main(){
	
	out_color = vec4(color(),1);
}