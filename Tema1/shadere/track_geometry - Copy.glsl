#version 330
layout( lines_adjacency ) in;
layout( triangle_strip, max_vertices=200) out;

uniform unsigned int num_points;
uniform mat4 view_matrix, projection_matrix;
const float width =2.5;

/*
 * Calculeaza coeficientii pentru un polinom de grad 4
 *   p(s) = c0 + c1*s + c2*s^2 + c3*s^3
 * a.i
 *   p(0) = x0, p(1) = x1
 *   p'(0) = t0, p'(1) = t1.
 */
vec4 init_cubic_poly(float x0, float x1, float t0, float t1)
{
	vec4 res;
    res.x = x0;
    res.y = t0;
    res.z = -3*x0 + 3*x1 - 2*t0 - t1;
    res.w = 2*x0 - 2*x1 + t0 + t1;
	return res;
}

vec3 eval_cubic_poly(mat3x4 p, float t)
{
    vec4 tv = vec4(1, t, t * t, t * t * t);
    //return p.x + p.y*t + p.z*t2 + p.w*t3;
	return tv * p;
}

vec4 catmull_rom(float x0, float x1, float x2, float x3, float dt0, float dt1, float dt2)
{
    // calculam tagentele
    float t1 = (x1 - x0) / dt0 - (x2 - x0) / (dt0 + dt1) + (x2 - x1) / dt1;
    float t2 = (x2 - x1) / dt1 - (x3 - x1) / (dt1 + dt2) + (x3 - x2) / dt2;

    // scalam in [0,1]
    t1 *= dt1;
    t2 *= dt1;

    return init_cubic_poly(x1, x2, t1, t2);
}

float vec_dist_pat(vec3 p, vec3 q)
{
    float dx = q.x - p.x;
    float dy = q.y - p.y;
	float dz = q.z - p.z;
    return dx*dx + dy*dy + dz*dz;
}

mat3x4 init_centripetal_CR(mat4x3 cp)
{
    float dt0 = pow(vec_dist_pat(cp[0], cp[1]), 0.25);
    float dt1 = pow(vec_dist_pat(cp[1], cp[2]), 0.25);
    float dt2 = pow(vec_dist_pat(cp[2], cp[3]), 0.25);

    // safety check for repeated points
    if (dt1 < 1e-4f)    dt1 = 1.0;
    if (dt0 < 1e-4f)    dt0 = dt1;
    if (dt2 < 1e-4f)    dt2 = dt1;

	mat3x4 result;
    result[0] = catmull_rom(cp[0].x, cp[1].x, cp[2].x, cp[3].x, dt0, dt1, dt2);
    result[1] = catmull_rom(cp[0].y, cp[1].y, cp[2].y, cp[3].y, dt0, dt1, dt2);
	result[2] = catmull_rom(cp[0].z, cp[1].z, cp[2].z, cp[3].z, dt0, dt1, dt2);
	return result;
}


void main()
{
	mat4x3 cp = mat4x3(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz,
		gl_in[2].gl_Position.xyz, gl_in[3].gl_Position.xyz);

	unsigned int actual_num = 36u;
	float d = float(actual_num) + 1.0;

	vec3 dir, prev;
	vec3 up = vec3(0, 1, 0);
	mat3x4 cr = init_centripetal_CR(cp);

	prev = cp[0];
	dir = normalize(cross(up, eval_cubic_poly(cr, 1.0 / d) - cp[1]));

	gl_Position = projection_matrix * view_matrix * vec4(cp[1], 1);
	EmitVertex();
	gl_Position = projection_matrix * view_matrix * vec4(cp[1] - dir * width, 1);
	EmitVertex();

	prev = cp[1];
	for (unsigned int i = 1u; i <= actual_num; ++i) {
		vec3 wp = eval_cubic_poly(cr, float(i) / d);
		dir = normalize(cross(up, wp - prev));		
		gl_Position = projection_matrix * view_matrix * vec4(wp, 1);
		EmitVertex();
		gl_Position = projection_matrix * view_matrix * vec4(wp - dir * width, 1);
		EmitVertex();
		prev = wp;
	}

	dir = normalize(cross(up, cp[2] - prev));
	gl_Position = projection_matrix * view_matrix * vec4(cp[2], 1);
	EmitVertex();
	gl_Position = projection_matrix * view_matrix * vec4(cp[2] - dir * width, 1);
	EmitVertex();
	EndPrimitive();
}