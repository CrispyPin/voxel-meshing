extends Label


# Declare member variables here. Examples:
# var a: int = 2
# var b: String = "text"
var paused := false

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	#get_node("/root").transparent_bg = true
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	text = str(Engine.get_frames_per_second())
	if Input.is_action_just_pressed("pause"):
		paused = !paused
		if paused:
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
