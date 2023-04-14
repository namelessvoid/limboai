@tool
extends BTAction

@export var animation_name: String
@export var animation_player: NodePath

var _player: AnimationPlayer


func _generate_name() -> String:
	return "StartAnimation \"%s\"" % animation_name


func _setup() -> void:
	_player = agent.get_node(animation_player)


func _tick(p_delta: float) -> int:
	_player.play(animation_name)
	return SUCCESS
