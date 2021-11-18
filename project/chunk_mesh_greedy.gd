extends MeshInstance


#faces: +x -x +y -y +z -z
const face_normals = [
	Vector3(1, 0, 0), Vector3(-1, 0, 0),
	Vector3(0, 1, 0), Vector3(0, -1, 0),
	Vector3(0, 0, 1), Vector3(0, 0, -1)]


var m_verts := PoolVector3Array()
var m_normals := PoolVector3Array()
var m_indexes := PoolIntArray()
#var m_uvs := PoolVector2Array()
var m_colors := PoolColorArray()

var mesh_array := [] # contains vertexes, normals and indexes etc.
var type_colors := PoolColorArray()

onready var chunk := get_parent()


var t = 0

func _ready() -> void:
	randomize()
	mesh = ArrayMesh.new()
	clear_mesh_arrays()

	for _c in range(256):# assign colours to all types
		type_colors.append(Color(randf(),randf(),randf()))

func _process(delta: float) -> void:
	t += delta
	if t >= 1:
		t = 0
		generate_mesh()


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
	print("Generating 'greedy' mesh")
	var merge_counter = 0
	clear_mesh_arrays()

	var normal = 2# up
	for layer in range(chunk.width):
#	for y in range(chunk.width):
		var quads := [] # each quad is a list of x1,x2,y1,y2 coords (aligned with current plane)
		for slice in range(chunk.width):
#		for z in range(chunk.width):
			var strip_active := false

			for offset in range(chunk.width+1):
#			for x in range(chunk.width+1):
#				var voxel = chunk.get_voxel(Vector3(x,y,z))
#				var top = chunk.get_voxel(Vector3(x,y,z)+face_normals[normal])
#				var voxel = _get_voxel_rel(normal, y, z, x)
				var voxel = _get_voxel_rel(normal, layer, slice, offset)
				var top = _get_voxel_rel(normal, layer, slice, offset, true)

				if !strip_active:
					if voxel and !top: # start new strip
						strip_active = true
						quads.append([offset, offset+1, slice, slice+1])
				elif !voxel: # end strip
					quads[-1][1] = offset
					strip_active = false

		### merge adjacent strips
		var a = 0
		var b = 1
		while a < len(quads) - 1:
			var quad_b = quads[b]
			var quad_a = quads[a]
			if quad_a[3] == quad_b[2] and quad_a[0] == quad_b[0] and quad_a[1] == quad_b[1]: # y2 of current = y1 of next AND x=x
				quads.remove(b)
				quads[a][3] = quad_b[3]
				merge_counter += 1
			else:
				b += 1

			if quad_a[3] < quad_b[2]: # b is too far away
				a += 1
				b = a+1
			elif b == len(quads): # b is last quad
				a += 1


		_add_quads(quads, layer)

	apply_mesh()
	print("Merged ", merge_counter, " strips")
	print("Total tris: ", len(m_indexes)/3)


func _get_voxel_rel(face, layer, slice, offset, top=false):
	var pos: Vector3
	match face:
		0:
			pos = Vector3(layer, slice, offset)

		2:
			pos = Vector3(offset, layer, slice)

	if top:
		return chunk.get_voxel(pos + face_normals[face])
	else:
		return chunk.get_voxel(pos)



func _add_quads(quads, y):
	for q in quads:
		_add_quad([
			Vector3(q[0], y+1, q[2]),
			Vector3(q[1], y+1, q[2]),
			Vector3(q[1], y+1, q[3]),
			Vector3(q[0], y+1, q[3]),
		])


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

