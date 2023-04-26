/* limbo_debugger_view.h */

#ifdef TOOLS_ENABLED

#include "limbo_debugger_plugin.h"
#include "core/debugger/engine_debugger.h"
#include "core/math/math_defs.h"
#include "core/object/callable_method_pointer.h"
#include "core/os/memory.h"
#include "core/string/print_string.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "editor/editor_scale.h"
#include "editor/plugins/editor_debugger_plugin.h"
#include "limbo_debugger.h"
#include "modules/limboai/debugger/behavior_tree_data.h"
#include "modules/limboai/debugger/behavior_tree_view.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/split_container.h"
#include "scene/gui/texture_rect.h"

/////////////////////// LimboDebuggerTab

void LimboDebuggerTab::start_session() {
	bt_player_list->clear();
	bt_view->clear();
	alert_box->hide();
	info_message->set_text(TTR("Pick a player from the list to display behavior tree."));
	info_message->show();
	session->send_message("limboai:start_session", Array());
}

void LimboDebuggerTab::stop_session() {
	bt_player_list->clear();
	bt_view->clear();
	alert_box->hide();
	info_message->set_text(TTR("Run project to start debugging."));
	info_message->show();
	session->send_message("limboai:stop_session", Array());
}

void LimboDebuggerTab::update_active_bt_players(const Array &p_node_paths) {
	active_bt_players.clear();
	for (int i = 0; i < p_node_paths.size(); i++) {
		active_bt_players.push_back(p_node_paths[i]);
	}
	_update_bt_player_list(active_bt_players, filter_players->get_text());
}

String LimboDebuggerTab::get_selected_bt_player() {
	if (!bt_player_list->is_anything_selected()) {
		return "";
	}
	return bt_player_list->get_item_text(bt_player_list->get_selected_items()[0]);
}

void LimboDebuggerTab::update_behavior_tree(const BehaviorTreeData &p_data) {
	bt_view->update_tree(p_data);
	info_message->hide();
}

void LimboDebuggerTab::_show_alert(const String &p_message) {
	alert_message->set_text(p_message);
	// alert_icon->set_texture(get_theme_icon(SNAME("NodeInfo"), SNAME("EditorIcons")));
	alert_icon->set_texture(get_theme_icon(SNAME("StatusWarning"), SNAME("EditorIcons")));
	alert_box->set_visible(!p_message.is_empty());
}

void LimboDebuggerTab::_update_bt_player_list(const List<String> &p_node_paths, const String &p_filter) {
	// Remember selected item.
	String selected_player = "";
	if (bt_player_list->is_anything_selected()) {
		selected_player = bt_player_list->get_item_text(bt_player_list->get_selected_items().get(0));
	}

	bt_player_list->clear();
	int select_idx = -1;
	bool selection_filtered_out = false;
	for (const String &p : p_node_paths) {
		if (p_filter.is_empty() || p.contains(p_filter)) {
			int idx = bt_player_list->add_item(p);
			// Make item text shortened from the left, e.g ".../Agent/BTPlayer".
			bt_player_list->set_item_text_direction(idx, TEXT_DIRECTION_RTL);
			if (p == selected_player) {
				select_idx = idx;
			}
		} else if (p == selected_player) {
			selection_filtered_out = true;
		}
	}

	// Restore selected item.
	if (select_idx > -1) {
		bt_player_list->select(select_idx);
	} else if (!selected_player.is_empty()) {
		if (selection_filtered_out) {
			session->send_message("limboai:untrack_bt_player", Array());
			bt_view->clear();
			_show_alert("");
		} else {
			_show_alert("BehaviorTree player is no longer present.");
		}
	}
}

void LimboDebuggerTab::_bt_selected(int p_idx) {
	alert_box->hide();
	bt_view->clear();
	info_message->set_text(TTR("Waiting for behavior tree update."));
	info_message->show();
	NodePath path = bt_player_list->get_item_text(p_idx);
	Array data;
	data.push_back(path);
	session->send_message("limboai:track_bt_player", data);
}

void LimboDebuggerTab::_filter_changed(String p_text) {
	_update_bt_player_list(active_bt_players, p_text);
}

LimboDebuggerTab::LimboDebuggerTab(Ref<EditorDebuggerSession> p_session) {
	session = p_session;

	hsc = memnew(HSplitContainer);
	add_child(hsc);

	VBoxContainer *list_box = memnew(VBoxContainer);
	hsc->add_child(list_box);

	filter_players = memnew(LineEdit);
	filter_players->set_placeholder(TTR("Filter Players"));
	filter_players->connect(SNAME("text_changed"), callable_mp(this, &LimboDebuggerTab::_filter_changed));
	list_box->add_child(filter_players);

	bt_player_list = memnew(ItemList);
	bt_player_list->set_custom_minimum_size(Size2(240.0 * EDSCALE, 0.0));
	bt_player_list->set_h_size_flags(SIZE_FILL);
	bt_player_list->set_v_size_flags(SIZE_EXPAND_FILL);
	bt_player_list->connect(SNAME("item_selected"), callable_mp(this, &LimboDebuggerTab::_bt_selected));
	list_box->add_child(bt_player_list);

	view_box = memnew(VBoxContainer);
	hsc->add_child(view_box);

	bt_view = memnew(BehaviorTreeView);
	bt_view->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	bt_view->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	view_box->add_child(bt_view);

	alert_box = memnew(HBoxContainer);
	alert_box->hide();
	view_box->add_child(alert_box);

	alert_icon = memnew(TextureRect);
	alert_box->add_child(alert_icon);
	alert_icon->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);

	alert_message = memnew(Label);
	alert_box->add_child(alert_message);
	alert_message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);

	info_message = memnew(Label);
	info_message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	info_message->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	info_message->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	info_message->set_custom_minimum_size(Size2(100 * EDSCALE, 0));
	info_message->set_anchors_and_offsets_preset(PRESET_FULL_RECT, PRESET_MODE_KEEP_SIZE, 8 * EDSCALE);

	bt_view->add_child(info_message);

	stop_session();
}

//////////////////////// LimboDebuggerPlugin

void LimboDebuggerPlugin::setup_session(int p_idx) {
	Ref<EditorDebuggerSession> session = get_session(p_idx);
	tab = memnew(LimboDebuggerTab(session));
	tab->set_name("LimboAI");
	session->connect(SNAME("started"), callable_mp(tab, &LimboDebuggerTab::start_session));
	session->connect(SNAME("stopped"), callable_mp(tab, &LimboDebuggerTab::stop_session));
	session->add_session_tab(tab);
}

bool LimboDebuggerPlugin::capture(const String &p_message, const Array &p_data, int p_session) {
	bool captured = true;
	if (p_message == "limboai:active_bt_players") {
		tab->update_active_bt_players(p_data);
	} else if (p_message == "limboai:bt_update") {
		BehaviorTreeData data = BehaviorTreeData();
		data.deserialize(p_data);
		if (data.bt_player_path == tab->get_selected_bt_player()) {
			tab->update_behavior_tree(data);
		}
	} else {
		captured = false;
	}
	return captured;
}

bool LimboDebuggerPlugin::has_capture(const String &p_capture) const {
	return p_capture == "limboai";
}

LimboDebuggerPlugin::LimboDebuggerPlugin() {
	tab = nullptr;
}

#endif // TOOLS_ENABLED