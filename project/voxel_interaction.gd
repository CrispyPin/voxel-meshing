extends RayCast


onready var chunk := get_node("../../Chunk")
onready var indicator = get_node("../../Indicator")

func _ready() -> void:
#	chunk = get_node("../../Chunk")
	pass


func _physics_process(_delta: float) -> void:
	indicator.translation = get_pos() + Vector3(0.5, 0.5, 0.5)
	indicator.visible = is_colliding()
	if is_colliding():
		if Input.is_action_just_pressed("place"):
#		if Input.is_action_pressed("place"):
			_place_voxel()
		if Input.is_action_just_pressed("remove"):
#		if Input.is_action_pressed("remove"):
			_remove_voxel()


func _place_voxel():
	chunk.set_voxel(127, get_pos())


func _remove_voxel():
	chunk.set_voxel(0, get_pos(-1))


func get_pos(offset_multiplier = 1):
	var pos := get_collision_point()
	pos += offset_multiplier * get_collision_normal() * 0.1
	pos = Vector3(floor(pos.x), floor(pos.y), floor(pos.z))
	return pos

