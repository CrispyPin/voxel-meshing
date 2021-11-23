#ifndef CHUNK_H
#define CHUNK_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Spatial.hpp>

#include <Input.hpp>
#include <RandomNumberGenerator.hpp>

#include <MeshInstance.hpp>
#include <ArrayMesh.hpp>
#include <ShaderMaterial.hpp>
#include <Texture3D.hpp>

#include <StaticBody.hpp>
#include <CollisionShape.hpp>
#include <ConcavePolygonShape.hpp>

#define Voxel unsigned char
#define UV_MAP
#define VERT_COLOR

namespace godot {

class Chunk : public StaticBody {
	GODOT_CLASS(Chunk, StaticBody)

private:

	bool PosInBounds(int x, int y, int z);
	
	void MeshSimple();
	void MeshSimpleFace(Vector3 pos, char face);

	void MeshGreedy();
	Voxel GetVoxelRelative(char face, int layer, int slice, int offset, bool top);
	void MeshGreedyTransformQuad(int quad[4], int layer, char face);

	void MeshQuad(Vector3 verts[4], char face);
	void ResizeMeshData(int quad_count);
	void ClearMeshData();
	void ApplyMeshData();

	void SetTex3D(Voxel type, int x, int y, int z);
	void UpdateTex3D(); // full texture

	ArrayMesh array_mesh;
	ConcavePolygonShape collider;
	
	int mesh_index_offset;
	PoolVector3Array mesh_vertex;
	PoolVector3Array mesh_normal;
	PoolIntArray mesh_index;
	PoolVector3Array collider_vertex;
#ifdef VERT_COLOR
	PoolColorArray mesh_color;
#endif
#ifdef UV_MAP
	PoolVector2Array mesh_uv;
#endif
	RandomNumberGenerator rng;
	Input *input;

	bool mesh_outdated;
	bool mesh_optimised;
	float time_since_change;

	const Vector3 face_normals[6] = {
		Vector3(1, 0, 0), Vector3(-1, 0, 0),
		Vector3(0, 1, 0), Vector3(0, -1, 0),
		Vector3(0, 0, 1), Vector3(0, 0, -1)};
	const Vector3 face_verts[6][4] = {
		{Vector3(1,0,1), Vector3(1,1,1), Vector3(1,1,0), Vector3(1,0,0)},
		{Vector3(0,0,0), Vector3(0,1,0), Vector3(0,1,1), Vector3(0,0,1)},

		{Vector3(0,1,0), Vector3(1,1,0), Vector3(1,1,1), Vector3(0,1,1)},
		{Vector3(0,0,1), Vector3(1,0,1), Vector3(1,0,0), Vector3(0,0,0)},

		{Vector3(0,0,1), Vector3(0,1,1), Vector3(1,1,1), Vector3(1,0,1)},
		{Vector3(1,0,0), Vector3(1,1,0), Vector3(0,1,0), Vector3(0,0,0)}};
	const int quad_offsets[6] = {0, 1, 2, 2, 3, 0};

public:
	static void _register_methods();
	Chunk();
	~Chunk();
	void _init();
	void _ready();
	void _process(float delta);

	static const int width = 32;
	static const int area = width * width;
	static const int volume = width * width * width;

	Voxel voxels[volume];
	Ref<Texture3D> voxel_tex3D;

	Voxel GetVoxel(Vector3 pos);
	Voxel GetVoxelXYZ(int x, int y, int z);
	void SetVoxel(Voxel type, Vector3 pos);
	void SetVoxelXYZ(Voxel type, int x, int y, int z);

	int PosToIndex(int x, int y, int z);
	int PosToIndex(Vector3 pos);
	Vector3 IndexToPos(int i);

	float time_to_optimise; // time since change before generating optimised mesh
};

}

#endif // CHUNK_H