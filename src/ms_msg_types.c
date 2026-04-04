#include "ms_msg_types.h"
#include "stdio.h"

enum {
    ptype_buf_len = 60,
};

char* ms_pc(unsigned char ctrl)
{
    if(ctrl > ms_msg_ctrl_types_num)
        return "unknown";
    #define _MS_T_IN(name) #name,
    static char* list[] = {_MS_CTRL_TYPES_LIST};
    #undef _MS_T_IN
    return list[ctrl];
}

char* ms_pt(unsigned char type)
{
    if(type > ms_msg_types_num)
        return "unknown";
    #define _MS_T_IN(name) #name,
    static char* list[] = {_MS_TYPES_LIST};
    #undef _MS_T_IN
    return list[type];
}

char* ms_type2a(struct ms_type type)
{
    static char buf[ptype_buf_len];
    if(type.type == mst_ctrl) {
        sprintf(buf, "%s: %s", ms_pt(type.type), ms_pc(type.opt));
    }
    else {
        sprintf(buf, "%s", ms_pt(type.type));
    }
    return buf;
}
