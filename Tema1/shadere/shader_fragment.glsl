#version 330
layout(location = 0) out vec4 out_color;

layout (std140) uniform Material {
	vec4 diffuse;
	vec4 specular;
	float shininess;
	int texCount;
};

uniform	sampler2D texUnit;
uniform vec3 light_position;
uniform vec3 eye_position;

in vec3 world_normal;
in vec3 world_pos;
in vec2 texcoord;

const float ambient_light = 0.2; 

void main(){

	vec4 light;
	
	vec3 razaincidenta = normalize(light_position - world_pos); // de la sursa de lumina la punct
	vec3 dirobservator = normalize(eye_position - world_pos); // de la punct la pozitia observatorului (eye)\

	float diffuse_light = max(0, dot(world_normal, razaincidenta)); // nu ne intereseaza [-1; 0] al cosinusului

	// specular light with blinn fong

	vec3 half_vector = normalize(razaincidenta + dirobservator);//bisectoarea dintre razaincidenta si dirobserrvator

	float specular_light = pow(max(dot(world_normal, half_vector), 0), shininess);

	
	float d = distance(world_pos, light_position);
	float factor_atenuare = 1.0 / (d * d);

	light = diffuse * (ambient_light + diffuse_light) + specular * factor_atenuare * specular_light;
	
	if (texCount == 0)
		out_color = light;
	else
		out_color = texture(texUnit, texcoord);// * light;
}