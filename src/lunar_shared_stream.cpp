#include "lunar_shared_stream.hpp"

namespace lunar {

extern "C" {

void
make_shared_stream(shared_stream *p, stream_t srm)
{
    p->stream = srm;
    p->flag   = shared_stream::READ | shared_stream::WRITE;
    
    p->shared_data = new shared_stream::shared_data_t;
    
    p->shared_data->flag_shared = 0;
    p->shared_data->refcnt = 1;
}

// if shared_stream will be transfered to another thread,
// is_mt must be set true
void
make_shared_write_only_stream(shared_stream *dst,
                              shared_stream *src, bool is_mt)
{
    if (src->shared_data->flag_shared & shared_stream::MT) {
        spin_lock_acquire lock(src->shared_data->lock);
        
        dst->stream      = src->stream;
        dst->flag        = shared_stream::WRITE;
        dst->shared_data = src->shared_data;
        dst->shared_data->refcnt++;
    } else {
        dst->stream      = src->stream;
        dst->flag        = shared_stream::WRITE;
        dst->shared_data = src->shared_data;
        dst->shared_data->refcnt++;
        
        if (is_mt) {
            dst->shared_data->flag_shared |= shared_stream::MT;
        }
    }
}

bool
deref_shared_stream(shared_stream *ptr)
{
    if (ptr->shared_data->flag_shared & shared_stream::MT) {
        spin_lock_acquire lock(ptr->shared_data->lock);
        
        ptr->shared_data->refcnt--;
        if (ptr->shared_data->refcnt == 0)
            return true;
    } else {
        ptr->shared_data->refcnt--;
        if (ptr->shared_data->refcnt == 0)
            return true;
    }

    return false;
}

} // extern "C"

} // namespace lunar