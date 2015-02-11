#version 330
layout(location = 0) out vec4 out_color;
in vec2 texCoord;
uniform sampler2D texUnit;

in vec3 world_pos;
in vec3 world_normal;


uniform vec3 light_position1;
uniform vec3 light_position2;
uniform vec3 light_direction;

vec3 get_spot( vec3 vWorldPos, vec3 light_pos, vec3 light_dir)
{
  const float coneCos = 0.9971; //cos 45
  const vec3 light_color = vec3(1.0, 1.0, 1.0);
  const float lin_att = 0.0055;
  
  //Distance from fragment's position
  float fDistance = distance(vWorldPos, light_pos);
  
  // Get direction vector to fragment
  vec3 vDir = vWorldPos-light_pos;
  vDir = normalize(vDir);
  
  // Cosine between spotlight direction and directional vector to fragment
  float fCosine = dot(light_dir, vDir);
  
  // Difference between max cosine and current cosine
  float fDif = 1.0-coneCos;
  
  // This is how strong light is depending whether its nearer to the center of
  // cone or nearer to its borders (onway factor in article), clamp to 0.0 and 1.0
  float fFactor = clamp((fCosine-coneCos)/fDif, 0.0, 1.0);
    
  // If we're inside cone, calculate color
  //if(fCosine > coneCos)
    return vec3(light_color)*fFactor/(fDistance*lin_att);
  
  // No color otherwise
  return vec3(0.0, 0.0, 0.0);
}

void main()
{
	const vec3 ambient_light = vec3(0.3, 0.3, 0.3); 
	
	//out_color = vec4(texture(texUnit,  texCoord).xyz * (vec3(ambient_light) + get_spot(world_pos, light_position)), 1);
	out_color = vec4(texture(texUnit,  texCoord).xyz * (ambient_light + get_spot(world_pos, light_position1, light_direction) +  get_spot(world_pos, light_position2, light_direction)), 1);
}