#include "chunk.h"

using namespace godot;
using namespace std;

void Chunk::_register_methods() {
	register_method("_ready", &Chunk::_ready);
	register_method("_process", &Chunk::_process);

	register_method("get_voxel", &Chunk::GetVoxel);
	register_method("set_voxel", &Chunk::SetVoxel);
	register_property<Chunk, float>("time_to_optimise", &Chunk::time_to_optimise, 3.0);
}

Chunk::Chunk() {
}

Chunk::~Chunk() {
}

void Chunk::_init() {
	time_since_change = 0;
	mesh_outdated = false;
	mesh_optimised = true;
	time_to_optimise = 3.0;

	mesh_index_offset = 0;

	//generate voxels (should maybe be function)
	rng = *RandomNumberGenerator::_new();
	input = Input::get_singleton();

	//voxel_tex3D = Texture3D::_new();
	voxel_tex3D.instance();
	voxel_tex3D->create(width, width, width, Image::FORMAT_R8, 0);

	for (int i = 0; i < volume; i++) {
		voxels[i] = 0;
		//voxels[i] = (char)(rng.randf() > 0.4);

		//voxels[i] = i % 2 * rng.randi_range(1, 255);

		//voxels[i] = rng.randi_range(0, 16)*15;
		// torus
		Vector3 pos = IndexToPos(i) - Vector3(1,1,1)*16;
		voxels[i] = (int)(i/area)%2 * (int)((i/width) % width) % 2 * (i % width)%2 * rng.randi_range(32, 255);
		Vector2 q = Vector2(Vector2(pos.x, pos.z).length() - 12.0, pos.y);
		if (q.length() - 5 < 0) {
			voxels[i] = rng.randi_range(32, 255);
		}
	}
}

void Chunk::_ready() {
	array_mesh = *ArrayMesh::_new();
	// init mesh child
	MeshInstance *mesh_instance = get_node<MeshInstance>("ChunkMesh");
	CRASH_COND(mesh_instance == nullptr);
	mesh_instance->set_mesh(&array_mesh);
	
	ShaderMaterial *mesh_mat = static_cast<ShaderMaterial*>(*mesh_instance->get_material_override());
	
	mesh_mat->set_shader_param("voxels", voxel_tex3D);
	mesh_mat->set_shader_param("chunk_width", width);
	
	CollisionShape *collision_shape = get_node<CollisionShape>("ChunkCollider");
	CRASH_COND(collision_shape == nullptr);
	collider = *ConcavePolygonShape::_new();
	collision_shape->set_shape(&collider);

	MeshGreedy();
	UpdateTex3D();
}

void Chunk::_process(float delta) {
	if (input->is_action_just_pressed("f3")) {
		MeshSimple();
	}
	if (input->is_action_just_pressed("f4")) {
		MeshGreedy();
	}
	if (mesh_outdated) {
		mesh_optimised = true;
		//time_since_change = 0;
		//MeshSimple();
		MeshGreedy();
		mesh_outdated = false;
	}
	if (!mesh_optimised) {
		time_since_change += delta;
		if (time_since_change >= time_to_optimise) {
			MeshGreedy();
			mesh_optimised = true;
		}
	}
}

void Chunk::ClearMeshData() {
	mesh_index_offset = 0;
	mesh_vertex.resize(0);
	mesh_normal.resize(0);
	mesh_index.resize(0);
	collider_vertex.resize(0);
#ifdef VERT_COLOR
	mesh_color.resize(0);
#endif
#ifdef UV_MAP
	mesh_uv.resize(0);
#endif
}

void Chunk::ApplyMeshData() {
	Array mesh_array;
	mesh_array.resize(Mesh::ARRAY_MAX);
	mesh_array[Mesh::ARRAY_VERTEX] = mesh_vertex;
	mesh_array[Mesh::ARRAY_NORMAL] = mesh_normal;
	mesh_array[Mesh::ARRAY_INDEX] = mesh_index;
#ifdef VERT_COLOR
	mesh_array[Mesh::ARRAY_COLOR] = mesh_color;
#endif
#ifdef UV_MAP
	mesh_array[Mesh::ARRAY_TEX_UV] = mesh_uv;
#endif
	
	if (array_mesh.get_surface_count() > 0) {
		array_mesh.surface_remove(0);
	}
#ifdef DEBUG_TIME
	int64_t start_time = OS::get_singleton()->get_system_time_msecs();
	array_mesh.add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);
	Godot::print("applying mesh");
	Godot::print(String::num(OS::get_singleton()->get_system_time_msecs() - start_time));
	collider.set_faces(collider_vertex);
	Godot::print("applying collider");
	Godot::print(String::num(OS::get_singleton()->get_system_time_msecs() - start_time));
#else
	array_mesh.add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);
	collider.set_faces(collider_vertex);
#endif

}

void Chunk::ResizeMeshData(int quad_count) {
	int vert_count = mesh_vertex.size() + quad_count * 4;
	int index_count = mesh_index.size() + quad_count * 6;
	
	mesh_vertex.resize(vert_count);
	mesh_normal.resize(vert_count);
	mesh_index.resize(index_count);
	collider_vertex.resize(index_count);
#ifdef VERT_COLOR
	mesh_color.resize(vert_count);
#endif
#ifdef UV_MAP
	mesh_uv.resize(vert_count);
#endif
}


void Chunk::UpdateTex3D() {
	Ref<Image> voxel;
	voxel.instance();
	voxel->create(1, 1, false, Image::FORMAT_R8);
	voxel->lock();

	for (int i = 0; i < volume; i++) {
		Vector3 pos = IndexToPos(i);
		Color col = Color(GetVoxel(pos)/256.0, 0, 0);
		voxel->set_pixel(0, 0, col);
		voxel_tex3D->set_data_partial(voxel, (int)pos.x, (int)pos.y, (int)pos.z);
	}
	
}

void Chunk::MeshSimple() {
#ifdef DEBUG_TIME
	Godot::print("Generating easy mesh");
	int64_t start_time = OS::get_singleton()->get_system_time_msecs();
#endif
	ClearMeshData();
	int quad_capacity = 0;

	for (int i = 0; i < volume; i++) {
		if (voxels[i] != 0) {
			if (mesh_index_offset > quad_capacity - 6) {
				ResizeMeshData(64);
				quad_capacity += 64;
			}
			for (int face = 0; face < 6; face++) {
				Vector3 pos = IndexToPos(i);
				Vector3 normal = face_normals[face];
				if (GetVoxel(pos + normal) != 0)
					continue;
				
				Vector3 verts[4];
				for (int i = 0; i < 4; i++) {
					verts[i] = pos + face_verts[face][i];
				}
				MeshQuad(verts, face);
			}
		}
	}
	ResizeMeshData(mesh_index_offset - quad_capacity); // remove extra allocated quads
#ifdef DEBUG_TIME
	Godot::print("generating time:");
	Godot::print(String::num(OS::get_singleton()->get_system_time_msecs() - start_time));
#endif
	ApplyMeshData();
#ifdef DEBUG_TIME
	Godot::print("total time:");
	Godot::print(String::num(OS::get_singleton()->get_system_time_msecs() - start_time));
#endif
}

void Chunk::MeshGreedy() {
#ifdef DEBUG_TIME
	Godot::print("Generating greedy mesh");
	int64_t start_time = OS::get_singleton()->get_system_time_msecs();
#endif
	ClearMeshData();

	for (int face = 0; face < 6; face++) {
		for (int layer = 0; layer < width; layer++) {
			// list of [offset_start, offset_end_min, slice_start, slice_end, offset_end_max]
			// slice is sideways relative to strips, offset along strip length
			// offset_end is a range because it might end under voxels, so the exact lenght is not set
			// relative coords of quads
			
			PoolIntArray q_offset_start;
			PoolIntArray q_offset_end_min;
			PoolIntArray q_offset_end_max;
			PoolIntArray q_slice_start;
			PoolIntArray q_slice_end;
			//Array quads;
			for (int slice = 0; slice < width; slice++) {
				bool strip_active = false;
				int offset_end_min = 0;
				Voxel prev_top = 0;

				for (int offset = 0; offset < width + 1; offset++) {
					Voxel voxel = GetVoxelLayered(face, layer, slice, offset, false);
					Voxel top = GetVoxelLayered(face, layer, slice, offset, true);

					if (!strip_active) {
						if (voxel != 0 && top == 0) { // start strip
							strip_active = true;
							offset_end_min = 0;
							//int q[5] = {offset, offset+1, slice, slice+1, width};
							//quads.append(q);
							q_offset_start.append(offset);
							q_offset_end_min.append(offset+1);
							q_slice_start.append(slice);
							q_slice_end.append(slice+1);
							q_offset_end_max.append(width);
						}
					}
					else {
						if (top != 0 && prev_top == 0) {// start of strip above
							offset_end_min = offset;
						}
						if (voxel == 0 && top == 0) { // end strip
							if (prev_top == 0)
								offset_end_min = offset;
							
							//quads[-1][1] = offset_end_min;
							//quads[-1][4] = offset;
							int i = q_offset_end_max.size() - 1;
							q_offset_end_min.set(i, offset_end_min);
							q_offset_end_max.set(i, offset);
							strip_active = false;
						}
					}
					prev_top = top;
				}
			}

			// merge adjacent aligned/overlapping strips
			int a = 0;
			int b = 0;
			while (a < q_offset_start.size() - 1) {
				// if a,b are adjacent and start equal and the end ranges overlap, merge
				if (q_slice_end[a] == q_slice_start[b] && q_offset_start[a] == q_offset_start[b] && (
					(q_offset_end_min[a] >= q_offset_end_min[b] && q_offset_end_min[a] <= q_offset_end_max[b]) // a range starts within b range
					|| (q_offset_end_min[b] >= q_offset_end_min[a] && q_offset_end_min[b] <= q_offset_end_max[a]) // b range starts within a range
				)) {
					// set width of merged strip
					q_slice_end.set(a, q_slice_end[b]);

					// set end range
					if (q_offset_end_min[b] > q_offset_end_min[a])
						q_offset_end_min.set(a, q_offset_end_min[b]);
					if (q_offset_end_max[b] < q_offset_end_max[a])
						q_offset_end_max.set(a, q_offset_end_max[b]);

					// remove B strip
					q_offset_start.remove(b);
					q_offset_end_min.remove(b);
					q_slice_start.remove(b);
					q_slice_end.remove(b);
					q_offset_end_max.remove(b);
				}
				else {
					b++;
				}
				// b is last quad or there is a gap between a and b in the slice width direction
				if (b >= q_offset_start.size() || q_slice_end[a] < q_slice_start[b]) {
					a++;
					b = a;
				}
			}
			ResizeMeshData(q_offset_start.size());
			for (int i = 0; i < q_offset_start.size(); i++) {
				//PoolIntArray raw = (PoolIntArray)quads[i];
				int q[4] = {q_offset_start[i], q_offset_end_min[i], q_slice_start[i], q_slice_end[i]};
				MeshGreedyTransformQuad(q, layer, face);
			}
		}
	}
#ifdef DEBUG_TIME
	Godot::print("generating time:");
	Godot::print(String::num(OS::get_singleton()->get_system_time_msecs() - start_time));
#endif
	ApplyMeshData();
#ifdef DEBUG_TIME
	Godot::print("total time:");
	Godot::print(String::num(OS::get_singleton()->get_system_time_msecs() - start_time));
#endif
}

Voxel Chunk::GetVoxelLayered(char face, int layer, int slice, int offset, bool top) {
	Vector3 pos;
	switch (face) {
		case 0:
		case 1:
			pos = Vector3(layer, slice, offset);
			break;
		case 2:
		case 3:
			pos = Vector3(offset, layer, slice);
			break;
		case 4:
		case 5:
			pos = Vector3(slice, offset, layer);
			break;
		default:
			Godot::print("Invalid face index");
			CRASH_COND(true);
	}
	if (top) {
		return GetVoxel(pos + face_normals[face]);
	}
	return GetVoxel(pos);
}

void Chunk::MeshGreedyTransformQuad(int quad[4], int layer, char face) {
	Vector3 verts[4];
	switch (face) {
		case 0:
			verts[0] = Vector3(layer+1, quad[2], quad[0]);
			verts[1] = Vector3(layer+1, quad[2], quad[1]);
			verts[2] = Vector3(layer+1, quad[3], quad[1]);
			verts[3] = Vector3(layer+1, quad[3], quad[0]);
			break;
		case 1:
			verts[0] = Vector3(layer, quad[3], quad[0]);
			verts[1] = Vector3(layer, quad[3], quad[1]);
			verts[2] = Vector3(layer, quad[2], quad[1]);
			verts[3] = Vector3(layer, quad[2], quad[0]);
			break;
		case 2:
			verts[0] = Vector3(quad[0], layer+1, quad[2]);
			verts[1] = Vector3(quad[1], layer+1, quad[2]);
			verts[2] = Vector3(quad[1], layer+1, quad[3]);
			verts[3] = Vector3(quad[0], layer+1, quad[3]);
			break;
		case 3:
			verts[0] = Vector3(quad[0], layer, quad[3]);
			verts[1] = Vector3(quad[1], layer, quad[3]);
			verts[2] = Vector3(quad[1], layer, quad[2]);
			verts[3] = Vector3(quad[0], layer, quad[2]);
			break;
		case 4:
			verts[0] = Vector3(quad[2], quad[0], layer+1);
			verts[1] = Vector3(quad[2], quad[1], layer+1);
			verts[2] = Vector3(quad[3], quad[1], layer+1);
			verts[3] = Vector3(quad[3], quad[0], layer+1);
			break;
		case 5:
			verts[0] = Vector3(quad[3], quad[0], layer);
			verts[1] = Vector3(quad[3], quad[1], layer);
			verts[2] = Vector3(quad[2], quad[1], layer);
			verts[3] = Vector3(quad[2], quad[0], layer);
			break;
		default:
			Godot::print("Invalid face index");
			CRASH_COND(true);
	}
	MeshQuad(verts, face);
}

void Chunk::MeshQuad(Vector3 verts[4], char face) {
	PoolVector3Array::Write vertex_w = mesh_vertex.write();
	PoolVector3Array::Write normal_w = mesh_normal.write();	

	PoolIntArray::Write index_w = mesh_index.write();
	PoolVector3Array::Write collider_w = collider_vertex.write();

#ifdef VERT_COLOR
	PoolColorArray::Write color_w = mesh_color.write();
	Color col = Color(rng.randf(), rng.randf(), rng.randf());
#endif

	for (int v = 0; v < 4; v++) {
		vertex_w[mesh_index_offset * 4 + v] = verts[v];
		normal_w[mesh_index_offset * 4 + v] = face_normals[face];
#ifdef VERT_COLOR
		color_w[mesh_index_offset * 4 + v] = col;
#endif
	}

	for (int i = 0; i < 6; i++) {
		index_w[mesh_index_offset * 6 + i] = mesh_index_offset * 4 + quad_offsets[i];
		collider_w[mesh_index_offset * 6 + i] = verts[quad_offsets[i]];
	}
#ifdef UV_MAP
	PoolVector2Array::Write uv_w = mesh_uv.write();
	uv_w[mesh_index_offset * 4] = Vector2(0, 1);
	uv_w[mesh_index_offset * 4+1] = Vector2(0, 0);
	uv_w[mesh_index_offset * 4+2] = Vector2(1, 0);
	uv_w[mesh_index_offset * 4+3] = Vector2(1, 1);
#endif
	mesh_index_offset++;
}

Voxel Chunk::GetVoxel(Vector3 pos) {
	return GetVoxelXYZ(pos.x, pos.y, pos.z);
}

Voxel Chunk::GetVoxelXYZ(int x, int y, int z) {
	if (PosInBounds(x, y, z))
		return voxels[PosToIndex(x, y, z)];
	return 0;
}

void Chunk::SetVoxel(Voxel type, Vector3 pos) {
	SetVoxelXYZ(type, pos.x, pos.y, pos.z);
}

void Chunk::SetVoxelXYZ(Voxel type, int x, int y, int z) {
	if (PosInBounds(x, y, z)) {
		voxels[PosToIndex(x, y, z)] = type;
		SetTex3D(type, x, y, z);
		mesh_outdated = true;
	}
}

void Chunk::SetTex3D(Voxel type, int x, int y, int z) {
	Ref<Image> voxel;
	voxel.instance();
	voxel->create(1, 1, false, Image::FORMAT_R8);
	voxel->lock();
	Color col = Color(type/256.0, 0, 0);
	voxel->set_pixel(0, 0, col);
	voxel_tex3D->set_data_partial(voxel, x, y, z);
}

int Chunk::PosToIndex(int x, int y, int z) {
	return x * area + y * width + z;
}

int Chunk::PosToIndex(Vector3 pos) {
	return pos.x * area + pos.y * width + pos.z;
}

Vector3 Chunk::IndexToPos(int i) {
	return Vector3((int)(i/area), (int)(i/width) % width, i % width);
}

bool Chunk::PosInBounds(int x, int y, int z) {
	return  x >= 0 && x < width &&
			y >= 0 && y < width &&
			z >= 0 && z < width;
}
