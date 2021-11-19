extends Spatial


export var width = 32 # voxels per chunk
var area = width*width
var volume = width*width*width
export var voxel_size = 1 # voxel width
var physical_size: float = width * voxel_size

var voxels := PoolByteArray()

var test_case := [
	Vector3(0,0,0),
	Vector3(0,0,1),
	Vector3(0,0,2),
	Vector3(1,0,0),
	Vector3(1,0,1),
	Vector3(1,0,2),
	Vector3(2,0,0),
	Vector3(2,0,1),
	Vector3(2,0,2),
#	Vector3(3,0,0),
	Vector3(3,0,1),
	Vector3(3,0,2),

#	Vector3(3,1,0),
	Vector3(3,1,1),
	Vector3(3,1,2),

	Vector3(1,1,1),
	Vector3(2,1,2),
	Vector3(2,1,1),
]

func _ready() -> void:
	randomize()
	voxels.resize(volume)
	for v in range(volume):
		voxels[v] = 0

		# random
#		if randf() < 0.5:
#			voxels[v] = 1

		# torus
		var pos:Vector3 = _i_to_pos(v)
		voxels[v] = int(torus(pos - Vector3(1,1,1)*16) < 0)

		# sphere
#		var pos:Vector3 = _i_to_pos(v)
#		if (pos - Vector3(16,16,16)).length() < 15:
#			voxels[v] = 1

	# test case
	for v in test_case:
		set_voxel(v, 1)

	$Reference.generate_mesh()
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
	return true


func _xyz_is_valid(x, y, z):
	return !(x < 0 or x >= width or y < 0 or y >= width or z < 0 or z >= width)


func _xyz_to_i(x, y, z):
	return (x * area) + (y * width) + z


func _i_to_pos(i):
	var x = int(i/(area))
	var y = int(i/width) % width
	var z = i % width
	return Vector3(x, y, z)

