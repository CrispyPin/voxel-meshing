extends Spatial


export var chunk_size = 32 # voxels per chunk
var width = chunk_size
var area = width*width
var volume = width*width*width
export var voxel_size = 1 # voxel width
var physical_size: float = chunk_size * voxel_size

var changed := false
var voxels := PoolByteArray()

func _ready() -> void:
	randomize()
	voxels.resize(volume)
	for v in range(volume):
#		voxels[v] = v%2

#		if randf() < 0.3:
#			voxels[v] = 1#randi()%254+1

		var pos:Vector3 = _i_to_pos(v)
		voxels[v] = int(torus(pos - Vector3(1,1,1)*16) < 0)
#		if (pos - Vector3(16,16,16)).length() < 32:
#			voxels[v] = 1

	$Reference.generate_mesh()
#	$Strips.generate_mesh()
	$Greedy.generate_mesh()

func torus(pos: Vector3):
	var q = Vector2(Vector2(pos.x, pos.z).length() - 12.0, pos.y)
	return q.length() - 5


func get_voxel(p):
	if _xyz_is_valid(p.x, p.y, p.z): # needed because faces check out of bounds
		return voxels[_xyz_to_i(p.x, p.y, p.z)]
	return 0


func set_voxel(p, id):
	if !_xyz_is_valid(p.x, p.y, p.z):
		return false
	var idx = _xyz_to_i(p.x, p.y, p.z)
	if voxels[idx] != id:
		voxels[idx] = id
		changed = true
	return true


func _xyz_is_valid(x, y, z):
	return !(x < 0 or x >= chunk_size or y < 0 or y >= chunk_size or z < 0 or z >= chunk_size)


func _xyz_to_i(x, y, z):
	return (x * area) + (y * width) + z


func _i_to_pos(i):
	var x = int(i/(area))
	var y = int(i/width) % width
	var z = i % width
	return Vector3(x, y, z)


func world_to_chunk(wpos):
	# localise to chunk
	var x = fposmod(wpos.x, physical_size)
	var y = fposmod(wpos.y, physical_size)
	var z = fposmod(wpos.z, physical_size)
	# scale to voxels
	x /= voxel_size
	y /= voxel_size
	z /= voxel_size
	# get closest
	x = int(x)
	y = int(y)
	z = int(z)
	return Vector3(x, y, z)
