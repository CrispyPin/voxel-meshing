extends Camera

export var speed = 10
export var sensitivity_h = 1.0
export var sensitivity_v = 1.0
var vel

func _ready():
	vel = Vector3(0,1,0)
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _input(event):
	if event is InputEventMouseMotion:
		var angle_x = -event.relative.y * sensitivity_v * 0.002
		angle_x = clamp(angle_x, -PI*0.5-rotation.x, PI*0.5-rotation.x)
		rotate_object_local(Vector3(1,0,0), angle_x)
		var angle_y = -event.relative.x * sensitivity_h * 0.002
		rotate_y(angle_y)

func _physics_process(delta):
	var dir = Vector3()
	if Input.is_key_pressed(KEY_W):
		dir += Vector3(0,0,-1)
	if Input.is_key_pressed(KEY_S):
		dir += Vector3(0,0,1)
	if Input.is_key_pressed(KEY_A):
		dir += Vector3(-1,0,0)
	if Input.is_key_pressed(KEY_D):
		dir += Vector3(1,0,0)
	if Input.is_key_pressed(KEY_Q):
		dir += Vector3(0,-1,0)
	if Input.is_key_pressed(KEY_E):
		dir += Vector3(0,1,0)

	vel = dir.normalized() * speed

	translate(vel*delta)
