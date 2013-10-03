typedef enum {
    PT_CHAT_SEARCH = 0,
    PT_CHAT_TIMELINE = 1,
    PT_CHAT_LIST = 2,
    PT_CHAT_UNKNOWN = 3                     //keep this as the last element
} PtChatType;

GList          *pt_blist_node_menu (PurpleBlistNode * node);
PurpleChat     *pt_blist_chat_find (PurpleAccount * account, const char *name);
PurpleChat     *pt_blist_chat_timeline_new(PurpleAccount * account, gint timeline_id);
