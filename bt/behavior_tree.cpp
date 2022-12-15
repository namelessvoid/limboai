/* behavior_tree.cpp */

#include "behavior_tree.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/templates/list.h"
#include "core/variant/variant.h"

void BehaviorTree::init() {
	List<BTTask *> stack;
	BTTask *task = root_task.ptr();
	while (task != nullptr) {
		for (int i = 0; i < task->get_child_count(); i++) {
			task->get_child(i)->parent = task;
			stack.push_back(task->get_child(i).ptr());
		}
		task = nullptr;
		if (!stack.is_empty()) {
			task = stack.front()->get();
			stack.pop_front();
		}
	}
}

Ref<BehaviorTree> BehaviorTree::clone() const {
	Ref<BehaviorTree> copy = duplicate(false);
	copy->set_path("");
	if (root_task.is_valid()) {
		copy->root_task = root_task->clone();
	}
	return copy;
}

Ref<BTTask> BehaviorTree::instance(Object *p_agent, const Ref<Blackboard> &p_blackboard) const {
	ERR_FAIL_COND_V_MSG(root_task == nullptr, memnew(BTTask), "Trying to instance a behavior tree with no valid root task.");
	Ref<BTTask> inst = root_task->clone();
	inst->initialize(p_agent, p_blackboard);
	return inst;
}

void BehaviorTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_description", "p_value"), &BehaviorTree::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &BehaviorTree::get_description);
	ClassDB::bind_method(D_METHOD("set_root_task", "p_value"), &BehaviorTree::set_root_task);
	ClassDB::bind_method(D_METHOD("get_root_task"), &BehaviorTree::get_root_task);
	ClassDB::bind_method(D_METHOD("init"), &BehaviorTree::init);
	ClassDB::bind_method(D_METHOD("clone"), &BehaviorTree::clone);
	ClassDB::bind_method(D_METHOD("instance", "p_agent", "p_blackboard"), &BehaviorTree::instance);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root_task", PROPERTY_HINT_RESOURCE_TYPE, "BTTask"), "set_root_task", "get_root_task");
}