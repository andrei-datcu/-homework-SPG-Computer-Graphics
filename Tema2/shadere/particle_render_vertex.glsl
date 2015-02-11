#version 430

struct particle{
	vec4 position;
	vec4 speed;
	vec4 iposition;
	vec4 ispeed;
};

layout(std140,binding=0) buffer particles{
		particle data[];
};

uniform mat4 model_matrix;

void main(){
	gl_Position = model_matrix*vec4(data[gl_VertexID].position.xyz,1); 
}
