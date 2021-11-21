//#include "chunk_mesh.h"
#include "chunk.h"

extern "C" void GDN_EXPORT voxel_engine_gdnative_init(godot_gdnative_init_options *o) {
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT voxel_engine_gdnative_terminate(godot_gdnative_terminate_options *o) {
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT voxel_engine_nativescript_init(void *handle) {
	godot::Godot::nativescript_init(handle);

	//godot::register_class<godot::ChunkMesh>();
	godot::register_class<godot::Chunk>();
}
