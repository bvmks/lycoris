#ifndef _MS_NODE_H
#define _MS_NODE_H

struct ms_node {
    unsigned char id_priv[64]; 
    unsigned char id_pub[32];

    unsigned char pass_master_pub[32]; 
    unsigned char pass_master_priv[64];

    char node_name[64];
    
    // 3. База доверия
    struct trusted_node* contacts; // Список тех, чьи id_pub мы знаем
};

#endif
