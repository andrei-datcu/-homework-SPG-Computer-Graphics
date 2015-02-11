#version 330
layout( lines) in;
layout( triangle_strip, max_vertices=100) out;
uniform mat4 view_matrix, projection_matrix;

in float p_texcoord[];
in vec3 p_normal[];
out vec2 texCoord;
out vec3 world_pos;
out vec3 world_normal;

const float width =20.0;
const int strip_count = 10;

void output_stripe(vec3 p1, vec3 p2, vec3 right1, vec3 right2, float texc1, float texc2)
{
	int i;
	float tex_num = float(strip_count) * 2.0;
	vec3 norm1 = cross(right1, p_normal[0]);
	vec3 norm2 = cross(right2, p_normal[1]);
	for (i = strip_count; i >= 0; --i) {
		world_pos = p1 - i * right1 * width;
		gl_Position = projection_matrix * view_matrix * vec4(world_pos, 1);
		texCoord = vec2(float(strip_count - i) /tex_num, texc1);
		world_normal = norm1;
		EmitVertex();
		world_pos = p2 - i * right2 * width;
		gl_Position = projection_matrix * view_matrix * vec4(world_pos, 1);
		texCoord = vec2(float(strip_count - i) /tex_num, texc2);
		world_normal = norm2;
		EmitVertex();
	}

	for (i = 1; i < strip_count; ++i) {
		world_pos = p1 + i * right1 * width;
		gl_Position = projection_matrix * view_matrix * vec4(world_pos, 1);
		texCoord = vec2(float(strip_count + i) /tex_num, texc1);
		world_normal = norm1;
		EmitVertex();
		world_pos = p2 + i * right2 * width;
		gl_Position = projection_matrix * view_matrix * vec4(world_pos, 1);
		texCoord = vec2(float(strip_count + i) /tex_num, texc2);
		world_normal = norm2;
		EmitVertex();
	}

	EndPrimitive();
}

void main()
{
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;
	vec3 up = vec3(0, 1, 0);
	
	vec3 right01 = normalize(cross(up, p_normal[0]));
	vec3 right02 = normalize(cross(up, p_normal[1]));
	output_stripe(p0, p1, right01, right02, p_texcoord[0], p_texcoord[1]);

}