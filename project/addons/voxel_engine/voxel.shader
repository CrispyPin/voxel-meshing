shader_type spatial;

uniform sampler3D voxels;
uniform float chunk_width;

void fragment() {
	vec3 surface_pos = (CAMERA_MATRIX * vec4(VERTEX, 1.0)).xyz;
	vec3 cam_pos = CAMERA_MATRIX[3].xyz;
	vec3 ray_dir = normalize(surface_pos - cam_pos);
	
	vec3 world_pos = surface_pos + ray_dir*0.001;
	world_pos /= chunk_width;
	vec4 type = texture(voxels, world_pos);
	ALBEDO = type.xyz;
}
