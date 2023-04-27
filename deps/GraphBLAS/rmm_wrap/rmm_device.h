
#ifndef RMM_META_H
#define RMM_META_H

typedef struct
{
    char    name [256] ;
    size_t  total_global_memory ;
    int  number_of_sms ;
    int  compute_capability_major;
    int  compute_capability_minor;
    bool use_memory_pool;
    int  pool_size;             // TODO: should this be size_t?
    int  max_pool_size;         // TODO: should this be size_t?
    void *memory_resource;
}
rmm_device ;      // TODO: rename this?

#endif
