/**
 * TODO: legal stuff
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#include <string.h>
#include <glib/gstdio.h>

#include "config.h"

#include "prpl.h"
#include "accountopt.h"
#include "debug.h"
#include "version.h"

#include "defines.h"
#include "pt.h"
#include "pt_prefs.h"
#include "pt_blist.h"

PurpleChat *_pt_blist_chat_find(PurpleAccount * account, PtChatType chat_type, const char *component_key, const char *component_value)
{
    const char     *node_chat_name;
    gint            node_chat_type = 0;
    const char     *node_chat_type_str;
    PurpleChat     *chat;
    PurpleBlistNode *node,
                   *group;
    char           *normname;
    PurpleBuddyList *purplebuddylist = purple_get_blist();
    GHashTable     *components;

    g_return_val_if_fail(purplebuddylist != NULL, NULL);
    g_return_val_if_fail((component_value != NULL) && (*component_value != '\0'), NULL);

    normname = g_strdup(purple_normalize(account, component_value));
    purple_debug_info(purple_account_get_protocol_id(account), "Account %s searching for chat %s type %d\n", account->username, component_value == NULL ? "NULL" : component_value, chat_type);

    if (normname == NULL) {
        return NULL;
    }
    for (group = purplebuddylist->root; group != NULL; group = group->next) {
        for (node = group->child; node != NULL; node = node->next) {
            if (PURPLE_BLIST_NODE_IS_CHAT(node)) {

                chat = (PurpleChat *) node;

                if (account != chat->account)
                    continue;

                components = purple_chat_get_components(chat);
                if (components != NULL) {
                    node_chat_type_str = g_hash_table_lookup(components, "chat_type");
                    node_chat_name = g_hash_table_lookup(components, component_key);
                    node_chat_type = node_chat_type_str == NULL ? 0 : strtol(node_chat_type_str, NULL, 10);

                    if (node_chat_name != NULL && node_chat_type == chat_type && !strcmp(purple_normalize(account, node_chat_name), normname)) {
                        g_free(normname);
                        return chat;
                    }
                }
            }
        }
    }

    g_free(normname);
    return NULL;
}

PurpleChat     *pt_blist_chat_find_timeline(PurpleAccount * account, gint timeline_id)
{
    char           *tmp = g_strdup_printf("%d", timeline_id);
    PurpleChat     *chat = _pt_blist_chat_find(account, PT_CHAT_TIMELINE, "timeline_id", tmp);
    g_free(tmp);
    return chat;
}

PurpleChat     *pt_blist_chat_find_list(PurpleAccount * account, const char *name)
{
    PurpleChat     *chat = _pt_blist_chat_find(account, PT_CHAT_LIST, "list_name", name);
    return chat;
}

GList          *pt_blist_node_menu(PurpleBlistNode * node)
{
    GList          *menu = NULL;
#if 0
    if (PURPLE_BLIST_NODE_IS_CHAT(node)) {
        GList          *submenu = NULL;
        PurpleChat     *chat = PURPLE_CHAT(node);
        GHashTable     *components = purple_chat_get_components(chat);
        char           *label = g_strdup_printf(_("Automatically open chat on new tweets (Currently: %s)"), (pt_blist_chat_is_auto_open(chat) ? _("On") : _("Off")));
        const char     *chat_type_str = g_hash_table_lookup(components, "chat_type");
        PtChatType chat_type = chat_type_str == NULL ? 0 : strtol(chat_type_str, NULL, 10);

        PurpleMenuAction *action = purple_menu_action_new(label,
                                                          PURPLE_CALLBACK(pt_blist_chat_auto_open_toggle),
                                                          NULL, /* userdata passed to the callback */
                                                          NULL);    /* child menu items */
        g_free(label);
        purple_debug_info(purple_account_get_protocol_id(purple_chat_get_account(PURPLE_CHAT(node))), "providing buddy list context menu item\n");
        menu = g_list_append(menu, action);
        if (chat_type == PT_CHAT_SEARCH) {
            PT_ATTACH_SEARCH_TEXT cur_attach_search_text = pt_blist_chat_attach_search_text(chat);

            label = g_strdup_printf(_("No%s"), cur_attach_search_text == PT_ATTACH_SEARCH_TEXT_NONE ? _(" (set)") : "");
            action = purple_menu_action_new(label, PURPLE_CALLBACK(pt_blist_char_attach_search_toggle), (gpointer) PT_ATTACH_SEARCH_TEXT_NONE, NULL);
            g_free(label);
            submenu = g_list_append(submenu, action);

            label = g_strdup_printf(_("Prepend%s"), cur_attach_search_text == PT_ATTACH_SEARCH_TEXT_PREPEND ? _(" (set)") : "");
            action = purple_menu_action_new(label, PURPLE_CALLBACK(pt_blist_char_attach_search_toggle), (gpointer) PT_ATTACH_SEARCH_TEXT_PREPEND, NULL);
            g_free(label);
            submenu = g_list_append(submenu, action);

            label = g_strdup_printf(_("Append%s"), cur_attach_search_text == PT_ATTACH_SEARCH_TEXT_APPEND ? _(" (set)") : "");
            action = purple_menu_action_new(label, PURPLE_CALLBACK(pt_blist_char_attach_search_toggle), (gpointer) PT_ATTACH_SEARCH_TEXT_APPEND, NULL);
            g_free(label);
            submenu = g_list_append(submenu, action);

            label = g_strdup_printf(_("Tag all chats with search term:"));
            action = purple_menu_action_new(label, NULL, NULL, submenu);
            g_free(label);
            menu = g_list_append(menu, action);
        }
    } else if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
        PurpleMenuAction *action;
        purple_debug_info(purple_account_get_protocol_id(purple_buddy_get_account(PURPLE_BUDDY(node))), "providing buddy list context menu item\n");
        if (pt_option_default_dm(purple_buddy_get_account(PURPLE_BUDDY(node)))) {
            action = purple_menu_action_new(_("@Message"), PURPLE_CALLBACK(pt_blist_buddy_at_msg), NULL,   /* userdata passed to the callback */
                                            NULL);  /* child menu items */
        } else {
            action = purple_menu_action_new(_("Direct Message"), PURPLE_CALLBACK(pt_blist_buddy_dm), NULL, /* userdata passed to the callback */
                                            NULL);  /* child menu items */
        }
        menu = g_list_append(menu, action);
        action = purple_menu_action_new(_("Clear Reply Marker"), PURPLE_CALLBACK(pt_blist_buddy_clear_reply), NULL, NULL);
        menu = g_list_append(menu, action);
    } else {
    }
#endif
    return menu;
}

PurpleChat     *pt_blist_chat_find(PurpleAccount * account, const char *name)
{
    PurpleChat     *c = NULL;
#if 0
    static char    *timeline = "Timeline: ";
    static char    *search = "Search: ";
    static char    *list = "List: ";
    if (strlen(name) > strlen(timeline) && !strncmp(timeline, name, strlen(timeline))) {
        c = pt_blist_chat_find_timeline(account, 0);
    } else if (strlen(name) > strlen(search) && !strncmp(search, name, strlen(search))) {
        c = pt_blist_chat_find_search(account, name + strlen(search));
    } else if (strlen(name) > strlen(list) && !strncmp(list, name, strlen(list))) {
        c = pt_blist_chat_find_list(account, name + strlen(list));
    } else {
        purple_debug_error(purple_account_get_protocol_id(account), "Invalid call to %s; assuming \"search\" for %s\n", G_STRFUNC, name);
        c = pt_blist_chat_find_search(account, name);
    }
#endif
    return c;
}

PurpleChat *pt_blist_chat_timeline_new(PurpleAccount * account, gint timeline_id)
{
    PurpleGroup    *g;
    PurpleChat     *c = pt_blist_chat_find_timeline(account, timeline_id);
    GHashTable     *components;
    if (c != NULL) {
        return c;
    }
    /* No point in making this a preference (yet?)
     * the idea is that this will only be done once, and the user can move the
     * chat to wherever they want afterwards */
    g = purple_find_group(PT_PREF_DEFAULT_TIMELINE_GROUP);
    if (g == NULL)
        g = purple_group_new(PT_PREF_DEFAULT_TIMELINE_GROUP);

    components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    //TODO: fix all of this
    //1) FIXED: search shouldn't be set, but is currently a hack to fix purple_blist_find_chat (persistent chat, etc)
    //2) need this to work with multiple timelines.
    //3) this should be an option. Some people may not want the home timeline
    g_hash_table_insert(components, "interval", "120"); // DREM g_strdup_printf("%d", pt_option_timeline_timeout(account)));
    g_hash_table_insert(components, "chat_type", g_strdup_printf("%d", PT_CHAT_TIMELINE));
    g_hash_table_insert(components, "timeline_id", g_strdup_printf("%d", timeline_id));

    c = purple_chat_new(account, "Home Timeline", components);
    purple_blist_add_chat(c, g, NULL);
    return c;
}

PurpleChat *pt_blist_chat_list_new(PurpleAccount * account, const char *list_name, const char *owner, long long list_id)
{
    PurpleGroup    *g;
    PurpleChat     *c = pt_blist_chat_find_list(account, list_name);
    GHashTable     *components;
    if (c != NULL) {
        return c;
    }
    /* No point in making this a preference (yet?)
     * the idea is that this will only be done once, and the user can move the
     * chat to wherever they want afterwards */
    g = purple_find_group(PT_PREF_DEFAULT_LIST_GROUP);
    if (g == NULL)
        g = purple_group_new(PT_PREF_DEFAULT_LIST_GROUP);

    components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

    //TODO: fix all of this
    //1) FIXED: search shouldn't be set, but is currently a hack to fix purple_blist_find_chat (persistent chat, etc)
    //2) need this to work with multiple timelines.
    //3) this should be an option. Some people may not want the home timeline
    g_hash_table_insert(components, "interval", "120"); // DREM g_strdup_printf("%d", pt_option_list_timeout(account)));
    g_hash_table_insert(components, "chat_type", g_strdup_printf("%d", PT_CHAT_LIST));
    g_hash_table_insert(components, "list_name", g_strdup(list_name));
    g_hash_table_insert(components, "owner", g_strdup(owner));
    g_hash_table_insert(components, "list_id", g_strdup_printf("%lld", list_id));

    c = purple_chat_new(account, list_name, components);
    purple_blist_add_chat(c, g, NULL);
    return c;
}
