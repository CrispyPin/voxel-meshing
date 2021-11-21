#include "chunk.h"

using namespace godot;
using namespace std;

void Chunk::_register_methods() {
	register_method("_ready", &Chunk::_ready);
	register_method("get_voxel", &Chunk::GetVoxelVec);
	register_method("_process", &Chunk::_process);
	//register_method("set_voxel", &Chunk::SetVoxel);
}

Chunk::Chunk() {
}

Chunk::~Chunk() {
}

void Chunk::_init() {
	//generate voxels (should maybe be function)
	rng = *RandomNumberGenerator::_new();
	input = Input::get_singleton();

	for (int i = 0; i < volume; i++) {
		voxels[i] = 0;
		//voxels[i] = (char)(rng.randf() > 0.4);
		//torus
		Vector3 pos = IndexToPos(i) - Vector3(1,1,1)*16;
		Vector2 q = Vector2(Vector2(pos.x, pos.z).length() - 12.0, pos.y);
		if (q.length() - 5 < 0) {
			voxels[i] = 1;
		}
	}

}

void Chunk::_ready() {
	// init child mesh
	//ArrayMesh mesh = *ArrayMesh::_new();
	array_mesh = *ArrayMesh::_new();

	//MeshInstance *mi = get_node<MeshInstance>("ChunkMesh");
	mesh_instance = get_node<MeshInstance>("ChunkMesh");
	CRASH_COND(mesh_instance == nullptr);

	//mesh_instance = MeshInstance::_new();
	//mesh_instance->set_name("ChunkMesh");
	//add_child(mesh_instance);
	mesh_instance->set_mesh(&array_mesh);

}

void Chunk::_process(float delta) {
	if (input->is_action_just_pressed("f3")){
		MeshSimple();
	}
	if (input->is_action_just_pressed("f4")){
		MeshGreedy();
	}
}


void Chunk::ClearMeshData() {
	mesh_vertex.resize(0);
	mesh_normal.resize(0);
	mesh_index.resize(0);
	mesh_color.resize(0);
	mesh_uv.resize(0);
}

void Chunk::ApplyMeshData() {
	Array mesh_array;
	mesh_array.resize(Mesh::ARRAY_MAX);
	mesh_array[Mesh::ARRAY_VERTEX] = mesh_vertex;
	mesh_array[Mesh::ARRAY_NORMAL] = mesh_normal;
	mesh_array[Mesh::ARRAY_INDEX] = mesh_index;
	mesh_array[Mesh::ARRAY_COLOR] = mesh_color;
	mesh_array[Mesh::ARRAY_TEX_UV] = mesh_uv;

	//ArrayMesh mesh = mesh_instance->get_mesh();
	
	if (array_mesh.get_surface_count() > 0) {
		array_mesh.surface_remove(0);
	}
	array_mesh.add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);
}

void Chunk::MeshSimple() {
	Godot::print("Generating simple mesh with c++");
	ClearMeshData();

	for (int i = 0; i < volume; i++) {
		if (voxels[i] != 0) {
			for (int f = 0; f < 6; f++) {
				MeshSimpleFace(IndexToPos(i), f);
			}
		}
	}
	ApplyMeshData();
	Godot::print(String::num(mesh_index.size()));
}

void Chunk::MeshSimpleFace(Vector3 pos, char face) {
	Vector3 normal = face_normals[face];
	if (GetVoxelVec(pos + normal) != 0)
		return;
	
	int index_start = mesh_vertex.size();

	Color col = Color(rng.randf(), rng.randf(), rng.randf());

	for (int v = 0; v < 4; v++) {
		mesh_vertex.append(pos + face_verts[face][v]);
		mesh_normal.append(normal);
		mesh_color.append(col);
	}

	mesh_index.append(index_start);
	mesh_index.append(index_start+1);
	mesh_index.append(index_start+2);
	mesh_index.append(index_start+2);
	mesh_index.append(index_start+3);
	mesh_index.append(index_start);
	mesh_uv.append(Vector2(0, 1));
	mesh_uv.append(Vector2(0, 0));
	mesh_uv.append(Vector2(1, 0));
	mesh_uv.append(Vector2(1, 1));
}

void Chunk::MeshGreedy() {
	Godot::print("Generating greedy mesh with c++");
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
					Voxel voxel = GetVoxelRelative(face, layer, slice, offset, false);
					Voxel top = GetVoxelRelative(face, layer, slice, offset, true);

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
						if (voxel == 0) { // end strip
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
					
					q_slice_end.set(a, q_slice_end[b]);

					if (q_offset_end_min[b] > q_offset_end_min[a])
						q_offset_end_min.set(a, q_offset_end_min[b]);
					if (q_offset_end_max[b] < q_offset_end_max[a])
						q_offset_end_max.set(a, q_offset_end_max[b]);

					q_offset_start.remove(b);
					q_offset_end_min.remove(b);
					q_slice_start.remove(b);
					q_slice_end.remove(b);
					q_offset_end_max.remove(b);
				}
				else {
					b++;
				}

				if (b >= q_offset_start.size() || q_slice_end[a] < q_slice_start[b]) {
					a++;
					b = a;
				}
			}

			for (int i = 0; i < q_offset_start.size(); i++) {
				//PoolIntArray raw = (PoolIntArray)quads[i];
				int q[4] = {q_offset_start[i], q_offset_end_min[i], q_slice_start[i], q_slice_end[i]};
				MeshGreedyTransformQuad(q, layer, face);
			}
		}
	}
	
	ApplyMeshData();
	Godot::print(String::num(mesh_index.size()));
}

Voxel Chunk::GetVoxelRelative(char face, int layer, int slice, int offset, bool top) {
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
		return GetVoxelVec(pos + face_normals[face]);
	}
	return GetVoxelVec(pos);
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
	MeshGreedyQuad(verts, face);
}

void Chunk::MeshGreedyQuad(Vector3 verts[4], char face) {
	int index_start = mesh_vertex.size();
	Color col = Color(rng.randf(), rng.randf(), rng.randf());

	for (int v = 0; v < 4; v++) {
		mesh_vertex.append(verts[v]);
		mesh_normal.append(face_normals[face]);
		mesh_color.append(col);
	}
	mesh_index.append(index_start);
	mesh_index.append(index_start+1);
	mesh_index.append(index_start+2);
	mesh_index.append(index_start+2);
	mesh_index.append(index_start+3);
	mesh_index.append(index_start);

	mesh_uv.append(Vector2(0, 1));
	mesh_uv.append(Vector2(0, 0));
	mesh_uv.append(Vector2(1, 0));
	mesh_uv.append(Vector2(1, 1));
}

Voxel Chunk::GetVoxelVec(Vector3 pos) {
	return GetVoxelXYZ(pos.x, pos.y, pos.z);
}

Voxel Chunk::GetVoxelXYZ(int x, int y, int z) {
	if (PosInBounds(x, y, z))
		return voxels[PosToIndex(x, y, z)];
	return 0;
}

void Chunk::SetVoxel(Voxel type, int x, int y, int z) {
	if (PosInBounds(x, y, z))
		voxels[PosToIndex(x, y, z)] = type;
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
