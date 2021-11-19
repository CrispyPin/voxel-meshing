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
var m_colors := PoolColorArray()

var mesh_array := [] # contains vertexes, normals and indexes etc.
var type_colors := PoolColorArray()

onready var chunk := get_parent()


func _ready() -> void:
	randomize()
	mesh = ArrayMesh.new()
	clear_mesh_arrays()

	for _c in range(256):# assign colours to all types
		#var a = c/256.0
		#type_colors.append(Color(a, a, a))
		type_colors.append(Color(randf(),randf(),randf()))


func clear_mesh_arrays():
	m_verts.resize(0)
	m_normals.resize(0)
	m_indexes.resize(0)
	m_colors.resize(0)
#	m_uvs.resize(0)


func apply_mesh():
	mesh_array.resize(Mesh.ARRAY_MAX)
	mesh_array[Mesh.ARRAY_VERTEX] = m_verts
	mesh_array[Mesh.ARRAY_NORMAL] = m_normals
	mesh_array[Mesh.ARRAY_INDEX]  = m_indexes
	mesh_array[Mesh.ARRAY_COLOR]  = m_colors

	if mesh.get_surface_count():
		mesh.surface_remove(0)
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, mesh_array)


func generate_mesh():
	print("Generating mesh with strips")
	clear_mesh_arrays()

	for y in range(chunk.width):

		var quads := [] # each quad is a list of x1,x2,y1,y2 coords (aligned with current plane)
		# create quads for each face
		for z in range(chunk.width):
			for x in range(chunk.width):
				var voxel = chunk.get_voxel(Vector3(x, y, z))
				var top = chunk.get_voxel(Vector3(x, y+1, z))
				if voxel and top == 0:
					quads.append([x, x+1, z, z+1])

		# merge with +x neighbors in strips
		var i = 0
		while i < len(quads)-1:
			var quad = quads[i]
			var next = quads[i+1]
			if quad[1] == next[0] and quad[2] == next[2] and quad[3] == next[3]: # x2 of current = x1 of next AND y=y
				quads.remove(i+1)
				quads[i][1] = next[1]
			else:
				i += 1

		# apply quads
		for q in quads:
			_add_quad([
				Vector3(q[0], y+1, q[2]),
				Vector3(q[1], y+1, q[2]),
				Vector3(q[1], y+1, q[3]),
				Vector3(q[0], y+1, q[3]),
			])

	apply_mesh()
	print("indexes: ", len(m_indexes))

func _add_quad(verts, f=2):
	var i = len(m_verts)# offset for new tris/indexes

	var col = Color(randf(), randf(), randf())
	# add the 4 corner verts of this face
	for v in range(4):
		m_verts.append(verts[v])
		m_normals.append(face_normals[f])
		m_colors.append(col)

	for v in [0, 1, 2, 2, 3, 0]:
		m_indexes.append(i+v)

