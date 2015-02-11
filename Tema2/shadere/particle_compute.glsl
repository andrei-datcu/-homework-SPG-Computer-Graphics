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
layout (local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

uniform vec3 building_pos;
uniform float building_radius;

bool on_top_of_building(vec3 pos)
{
	if ( ( pos.x - building_pos.x) *  ( pos.x - building_pos.x) +
		 ( pos.z - building_pos.z) *  ( pos.z - building_pos.z) < building_radius * building_radius)

		 if (pos.y < building_pos.y)
			return true;
		 else
			return false;
}

void main(){

	const float miny = -300;

	uint gid = gl_GlobalInvocationID.x;

	vec3 pos = data[gid].position.xyz;
	vec3 spd = data[gid].speed.xyz;

	float dt =0.1;

	pos = pos + spd * dt + vec3(0,-0.9,0)*dt*dt/2.0f ;
	spd = spd + vec3(0,-0.9,0)*dt;
	if(pos.y < miny || on_top_of_building(pos)){
		pos = data[gid].iposition.xyz;
		spd = data[gid].ispeed.xyz;
	}

	data[gid].position.xyz =  pos;
	data[gid].speed.xyz =  spd;
}