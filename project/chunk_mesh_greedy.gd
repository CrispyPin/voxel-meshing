extends MeshInstance


#faces: +x -x +y -y +z -z
const face_normals = [
	Vector3(1, 0, 0), Vector3(-1, 0, 0),
	Vector3(0, 1, 0), Vector3(0, -1, 0),
	Vector3(0, 0, 1), Vector3(0, 0, -1)]


var m_verts   := PoolVector3Array()
var m_normals := PoolVector3Array()
var m_indexes := PoolIntArray()
var m_colors  := PoolColorArray()
var m_uvs     := PoolVector2Array()

var mesh_array := [] # contains vertexes, normals and indexes etc.

onready var chunk := get_parent()


func _ready() -> void:
	randomize()
	mesh = ArrayMesh.new()
	mesh_array.resize(Mesh.ARRAY_MAX)
	clear_mesh_arrays()


func clear_mesh_arrays():
	m_verts.resize(0)
	m_normals.resize(0)
	m_indexes.resize(0)
	m_colors.resize(0)
	m_uvs.resize(0)


func apply_mesh():
	mesh_array[Mesh.ARRAY_VERTEX] = m_verts
	mesh_array[Mesh.ARRAY_NORMAL] = m_normals
	mesh_array[Mesh.ARRAY_INDEX]  = m_indexes
	mesh_array[Mesh.ARRAY_COLOR]  = m_colors
	mesh_array[Mesh.ARRAY_TEX_UV] = m_uvs

	if mesh.get_surface_count():
		mesh.surface_remove(0)
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, mesh_array)


func generate_mesh():
	print("Generating optimised mesh")
	var merge_counter = 0
	clear_mesh_arrays()

	for face in range(6):
		for layer in range(chunk.width):
			# list of [slice start, slice end, offset start, offset end]
			# describing relative coords of quads
			var quads := []

			for slice in range(chunk.width):
				var strip_active := false

				for offset in range(chunk.width+1):
					var voxel = _get_voxel_rel(face, layer, slice, offset)
					var top = _get_voxel_rel(face, layer, slice, offset, true)

					if !strip_active:
						if voxel and !top: # start new strip
							strip_active = true
							quads.append([offset, offset+1, slice, slice+1])
					elif !voxel: # end strip
						quads[-1][1] = offset
						strip_active = false

			# merge adjacent aligned strips
			var a = 0
			var b = 1
			while a < len(quads) - 1:
				var quad_b = quads[b]
				var quad_a = quads[a]
				# if  slice_end of A = slice_start of B and offsets are equal (strips are adjacent and aligned)
				if quad_a[3] == quad_b[2] and quad_a[0] == quad_b[0] and quad_a[1] == quad_b[1]:
					quads.remove(b)
					quads[a][3] = quad_b[3]
					merge_counter += 1 # just interesting info
				else:
					b += 1

				if quad_a[3] < quad_b[2] or b == len(quads): # b is too far away or is last quad
					a += 1
					b = a

			for q in quads:
				_convert_quad(q, layer, face)

	apply_mesh()
	print("merged ", merge_counter, " strips")
	print("indexes: ", len(m_indexes))
	print("tris: ", len(m_indexes)/3)


func _get_voxel_rel(face, layer, slice, offset, top=false):
	var pos: Vector3
	match face:
		0,1:
			pos = Vector3(layer, slice, offset)
		2,3:
			pos = Vector3(offset, layer, slice)
		4,5:
			pos = Vector3(slice, offset, layer)

	if top:
		return chunk.get_voxel(pos + face_normals[face])
	else:
		return chunk.get_voxel(pos)


func _convert_quad(quad, layer, face):
	var points = quad + [layer, layer, layer+1, layer+1]
	match face:
		0:
			_add_quad([
				Vector3(layer+1, quad[2], quad[0]),
				Vector3(layer+1, quad[2], quad[1]),
				Vector3(layer+1, quad[3], quad[1]),
				Vector3(layer+1, quad[3], quad[0]),
			], face)
		1:
			_add_quad([
				Vector3(layer, quad[3], quad[0]),
				Vector3(layer, quad[3], quad[1]),
				Vector3(layer, quad[2], quad[1]),
				Vector3(layer, quad[2], quad[0]),
			], face)
		2:
			_add_quad([
				Vector3(quad[0], layer+1, quad[2]),
				Vector3(quad[1], layer+1, quad[2]),
				Vector3(quad[1], layer+1, quad[3]),
				Vector3(quad[0], layer+1, quad[3]),
			], face)
		3:
			_add_quad([
				Vector3(quad[0], layer, quad[3]),
				Vector3(quad[1], layer, quad[3]),
				Vector3(quad[1], layer, quad[2]),
				Vector3(quad[0], layer, quad[2]),
			], face)
		4:
			_add_quad([
				Vector3(quad[2], quad[0], layer+1),
				Vector3(quad[2], quad[1], layer+1),
				Vector3(quad[3], quad[1], layer+1),
				Vector3(quad[3], quad[0], layer+1),
			], face)
		5:
			_add_quad([
				Vector3(quad[3], quad[0], layer),
				Vector3(quad[3], quad[1], layer),
				Vector3(quad[2], quad[1], layer),
				Vector3(quad[2], quad[0], layer),
			], face)


func _add_quad(verts, face):
	var i = len(m_verts)# offset for new tris/indexes

#	var col = Color(randf(), randf(), randf())
	var col = Color(0.1, 0.6, 0.3)

	# add the 4 corner verts of this face
	for v in range(4):
		m_verts.append(verts[v])
		m_normals.append(face_normals[face])
		m_colors.append(col)

	m_uvs.append(Vector2(0, 1))
	m_uvs.append(Vector2(0, 0))
	m_uvs.append(Vector2(1, 0))
	m_uvs.append(Vector2(1, 1))

	for v in [0, 1, 2, 2, 3, 0]:
		m_indexes.append(i+v)

