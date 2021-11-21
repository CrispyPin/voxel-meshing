#ifndef CHUNK_H
#define CHUNK_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Spatial.hpp>

#include <Input.hpp>
#include <RandomNumberGenerator.hpp>

#include <MeshInstance.hpp>
#include <ArrayMesh.hpp>
#include <Material.hpp>

#include <StaticBody.hpp>
#include <CollisionShape.hpp>

#define Voxel unsigned char

namespace godot {

class Chunk : public Spatial {
	GODOT_CLASS(Chunk, Spatial)

private:

	bool PosInBounds(int x, int y, int z);
	
	void MeshSimple();
	void MeshSimpleFace(Vector3 pos, char face);

	void MeshGreedy();
	Voxel GetVoxelRelative(char face, int layer, int slice, int offset, bool top);
	void MeshGreedyTransformQuad(int quad[4], int layer, char face);
	void MeshGreedyQuad(Vector3 verts[4], char face);

	void ClearMeshData();
	void ApplyMeshData();

	MeshInstance *mesh_instance;
	ArrayMesh array_mesh;
	
	PoolVector3Array mesh_vertex;
	PoolVector3Array mesh_normal;
	PoolIntArray mesh_index;
	PoolColorArray mesh_color;
	PoolVector2Array mesh_uv;

	RandomNumberGenerator rng;
	Input *input;

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

	Voxel GetVoxelXYZ(int x, int y, int z);
	Voxel GetVoxelVec(Vector3 pos);
	void SetVoxel(Voxel type, int x, int y, int z);

	int PosToIndex(int x, int y, int z);
	int PosToIndex(Vector3 pos);
	Vector3 IndexToPos(int i);

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
};

}

#endif // CHUNK_H