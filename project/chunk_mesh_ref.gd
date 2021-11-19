extends MeshInstance


const face_normals = [
	Vector3(1, 0, 0), Vector3(-1, 0, 0),
	Vector3(0, 1, 0), Vector3(0, -1, 0),
	Vector3(0, 0, 1), Vector3(0, 0, -1)]

#faces: +x -x +y -y +z -z
const face_verts = [
	[Vector3(1,0,1), Vector3(1,1,1), Vector3(1,1,0), Vector3(1,0,0)],
	[Vector3(0,0,0), Vector3(0,1,0), Vector3(0,1,1), Vector3(0,0,1)],

	[Vector3(0,1,0), Vector3(1,1,0), Vector3(1,1,1), Vector3(0,1,1)],
	[Vector3(0,0,1), Vector3(1,0,1), Vector3(1,0,0), Vector3(0,0,0)],

	[Vector3(0,0,1), Vector3(0,1,1), Vector3(1,1,1), Vector3(1,0,1)],
	[Vector3(1,0,0), Vector3(1,1,0), Vector3(0,1,0), Vector3(0,0,0)]
]

var m_verts := PoolVector3Array()
var m_normals := PoolVector3Array()
var m_indexes := PoolIntArray()
#var m_uvs := PoolVector2Array()
#var m_colors := PoolColorArray()

var mesh_array := [] # contains vertexes, normals and indexes etc.
#var type_colors := PoolColorArray()

onready var chunk := get_parent()


func _ready() -> void:
	randomize()
	mesh = ArrayMesh.new()
	clear_mesh_arrays()

#	for _c in range(256):# assign colours to all types
		#var a = c/256.0
		#type_colors.append(Color(a, a, a))
#		type_colors.append(Color(randf(),randf(),randf()))


func clear_mesh_arrays():
	m_verts.resize(0)
	m_normals.resize(0)
	m_indexes.resize(0)
#	m_colors.resize(0)
#	m_uvs.resize(0)


func apply_mesh():#colours=true, uvs=true):
	mesh_array.resize(Mesh.ARRAY_MAX)
	mesh_array[Mesh.ARRAY_VERTEX] = m_verts
	mesh_array[Mesh.ARRAY_NORMAL] = m_normals
	mesh_array[Mesh.ARRAY_INDEX]  = m_indexes
#	if colours:
#		mesh_array[Mesh.ARRAY_COLOR]  = m_colors
#	if uvs:
#		mesh_array[Mesh.ARRAY_TEX_UV] = m_uvs

	if mesh.get_surface_count():
		mesh.surface_remove(0)
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, mesh_array)


func generate_mesh():
	print("Generating reference mesh")
	clear_mesh_arrays()

	for vi in range(len(chunk.voxels)):
		if chunk.voxels[vi]:
#			for f in [2]:
#			for f in [0,1,3,4,5]:
			for f in range(6):
				_update_mesh_face_1(chunk._i_to_pos(vi), f)
#					for _c in range(4):
#						m_colors.append(type_colors[chunk.voxels[vi]])
	apply_mesh()
	print("indexes: ", len(m_indexes))
	print("tris: ", len(m_indexes)/3)


func _update_mesh_face_1(pos, f):
	if chunk.get_voxel(pos + face_normals[f]):
		return false

	var i = len(m_verts)# offset for new tris/indexes

	# add the 4 corner verts of this face
	for v in range(4):
		m_verts.append((pos + face_verts[f][v]) * chunk.voxel_size)
		m_normals.append(face_normals[f])
		#m_colors.append(Color(randf(),randf(),randf()))

	#m_uvs.append(Vector2(0, 1))
	#m_uvs.append(Vector2(0, 0))
	#m_uvs.append(Vector2(1, 0))
	#m_uvs.append(Vector2(1, 0)) # hide corner
	#m_uvs.append(Vector2(1, 1))

	# connect into 2 tris:
	for v in [0, 1, 2, 2, 3, 0]:
		m_indexes.append(i+v)

	return true

