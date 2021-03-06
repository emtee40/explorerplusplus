// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "APIBinding.h"
#include "MenuApi.h"
#include "TabsAPI.h"
#include "UiApi.h"

void BindTabsAPI(sol::state &state, TabContainerInterface *tabContainer);
void BindMenuApi(sol::state &state, Plugins::PluginMenuManager *pluginMenuManager);
void BindUiApi(sol::state &state, UiTheming *uiTheming);
sol::table MarkTableReadOnly(sol::state &state, sol::table &table);
int deny(lua_State *state);

void Plugins::BindAllApiMethods(sol::state &state, TabContainerInterface *tabContainer,
	Plugins::PluginMenuManager *pluginMenuManager, UiTheming *uiTheming)
{
	BindTabsAPI(state, tabContainer);
	BindMenuApi(state, pluginMenuManager);
	BindUiApi(state, uiTheming);
}

void BindTabsAPI(sol::state &state, TabContainerInterface *tabContainer)
{
	std::shared_ptr<Plugins::TabsApi> tabsApi = std::make_shared<Plugins::TabsApi>(tabContainer);

	sol::table tabsTable = state.create_named_table("tabs");
	sol::table tabsMetaTable = MarkTableReadOnly(state, tabsTable);

	tabsMetaTable.set_function("getAll", &Plugins::TabsApi::getAll, tabsApi);
	tabsMetaTable.set_function("get", &Plugins::TabsApi::get, tabsApi);
	tabsMetaTable.set_function("create", &Plugins::TabsApi::create, tabsApi);
	tabsMetaTable.set_function("move", &Plugins::TabsApi::move, tabsApi);
	tabsMetaTable.set_function("close", &Plugins::TabsApi::close, tabsApi);

	sol::table onCreatedTable = tabsMetaTable.create_named("onCreated");
	sol::table onCreatedMetaTable = MarkTableReadOnly(state, onCreatedTable);

	onCreatedMetaTable.set_function("addListener", &Plugins::TabsApi::addTabCreatedObserver, tabsApi);
	onCreatedMetaTable.set_function("removeListener", &Plugins::TabsApi::removeTabCreatedObserver, tabsApi);

	tabsMetaTable.new_usertype<Plugins::TabsApi::Tab>("Tab",
		"id", &Plugins::TabsApi::Tab::id,
		"location", &Plugins::TabsApi::Tab::location,
		"name", &Plugins::TabsApi::Tab::name,
		"viewMode", &Plugins::TabsApi::Tab::viewMode,
		"locked", &Plugins::TabsApi::Tab::locked,
		"addressLocked", &Plugins::TabsApi::Tab::addressLocked,
		"__tostring", &Plugins::TabsApi::Tab::toString);

	tabsMetaTable.new_enum("ViewMode",
		"details", ViewMode::VM_DETAILS,
		"extraLargeIcons", ViewMode::VM_EXTRALARGEICONS,
		"icons", ViewMode::VM_ICONS,
		"largeIcons", ViewMode::VM_LARGEICONS,
		"list", ViewMode::VM_LIST,
		"smallIcons", ViewMode::VM_SMALLICONS,
		"thumbnails", ViewMode::VM_THUMBNAILS,
		"tiles", ViewMode::VM_TILES);
}

void BindMenuApi(sol::state &state, Plugins::PluginMenuManager *pluginMenuManager)
{
	std::shared_ptr<Plugins::MenuApi> menuApi = std::make_shared<Plugins::MenuApi>(pluginMenuManager);

	sol::table menuTable = state.create_named_table("menu");
	sol::table metaTable = MarkTableReadOnly(state, menuTable);

	metaTable.set_function("create", &Plugins::MenuApi::create, menuApi);
	metaTable.set_function("remove", &Plugins::MenuApi::remove, menuApi);
}

void BindUiApi(sol::state &state, UiTheming *uiTheming)
{
	std::shared_ptr<Plugins::UiApi> uiApi = std::make_shared<Plugins::UiApi>(uiTheming);

	sol::table uiTable = state.create_named_table("ui");
	sol::table metaTable = MarkTableReadOnly(state, uiTable);

	metaTable.set_function("setListViewColors", &Plugins::UiApi::setListViewColors, uiApi);
	metaTable.set_function("setTreeViewColors", &Plugins::UiApi::setTreeViewColors, uiApi);
}

sol::table MarkTableReadOnly(sol::state &state, sol::table &table)
{
	sol::table metaTable = state.create_table();

	metaTable[sol::meta_function::new_index] = deny;
	metaTable[sol::meta_function::index] = metaTable;

	table[sol::metatable_key] = metaTable;

	return metaTable;
}

int deny(lua_State *state)
{
	return luaL_error(state, "Attempt to modify read-only table");
}