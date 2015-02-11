#version 430
layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

uniform mat4  view_matrix, projection_matrix; 

const float billboard_size_x = 0.10f;
const float billboard_size_y = 0.45f;

out vec2 g_texcoord;
void main(){

	vec3 pos = (view_matrix * vec4(gl_in[0].gl_Position.xyz, 1)).xyz;
	vec3 to_camera = normalize(-pos);

	vec3 up = vec3(0, 1.0, 0);
	vec3 right = cross(to_camera, up) * billboard_size_x;
	up *= billboard_size_y;

	gl_Position = projection_matrix * vec4(pos - right, 1);
	g_texcoord = vec2(0.0f, 0.0f);
	EmitVertex();

	gl_Position = projection_matrix * vec4(pos - right + up, 1);
	g_texcoord = vec2(0.0f, 1.0f);
	EmitVertex();

	gl_Position = projection_matrix * vec4(pos + right, 1);
	g_texcoord = vec2(1.0f, 0.0f);
	EmitVertex();

	gl_Position = projection_matrix * vec4(pos + right + up, 1);
	g_texcoord = vec2(1.0f, 1.0f);
	EmitVertex();	
}
