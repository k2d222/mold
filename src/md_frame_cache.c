#include "md_frame_cache.h"

#include "core/md_allocator.h"
#include "core/md_simd.h"
#include "core/md_log.h"
#include "core/md_sync.h"
#include "core/md_array.inl"
#include "md_trajectory.h"
#include "md_util.h"

#include <string.h>

#define CACHE_MAGIC 0x7bacacababca8181
#define CACHE_ASSOCIATIVITY 2
#define CACHE_MEM_ALIGNMENT 16

typedef struct md_slot_header_t {
    uint32_t frame_index;
    uint32_t access_count;
} md_slot_header_t;

bool md_frame_cache_init(md_frame_cache_t* cache, md_trajectory_i* traj, md_allocator_i* alloc, int64_t num_cached_frames) {
    ASSERT(cache);
    ASSERT(traj);
    ASSERT(alloc);
    ASSERT(num_cached_frames >= 0);

    if (num_cached_frames == 0) {
        num_cached_frames = md_trajectory_num_frames(traj);
    }

    // We want to ensure that there are enough padding for each frame to avoid overlap if one wants to do full-width simd stores.
    const int64_t num_atoms = ROUND_UP(md_trajectory_num_atoms(traj), md_simd_widthf); 
    const int64_t bytes_per_frame = sizeof(md_semaphore_t) + sizeof(md_slot_header_t) + sizeof(md_frame_data_t) + num_atoms * sizeof(float) * 3;
    const int64_t num_slots = ROUND_UP(num_cached_frames, CACHE_ASSOCIATIVITY); // This needs to be divisible by N for N-way associativity.
    const int64_t total_bytes = num_slots * bytes_per_frame + CACHE_MEM_ALIGNMENT;

    cache->alloc = alloc;
    cache->traj = traj;
    md_array_resize(cache->buf, total_bytes, alloc);
    if (!cache->buf) {
        md_print(MD_LOG_TYPE_ERROR, "Failed to allocate requested memory for frame_cache.");
        memset(cache, 0, sizeof(md_frame_cache_t));
        return false;
    }
    cache->slot.count  = num_slots;
    cache->slot.lock   = (md_semaphore_t*)NEXT_ALIGNED_ADRESS(cache->buf, CACHE_MEM_ALIGNMENT);
    cache->slot.header = (md_slot_header_t*)(cache->slot.lock + num_slots);
    cache->slot.data   = (md_frame_data_t*)(cache->slot.header + num_slots);

    cache->magic = CACHE_MAGIC;

    float* coord_data = (float*)(cache->slot.data + num_slots);
    for (int64_t i = 0; i < num_slots; ++i) {
        md_semaphore_init(&cache->slot.lock[i], 1);

        cache->slot.header[i].frame_index = 0xFFFFFFFF;
        cache->slot.header[i].access_count = 0;

        cache->slot.data[i].header.num_atoms = (int)num_atoms;
        cache->slot.data[i].x = coord_data + (num_slots * num_atoms) * 0 + num_atoms * i;
        cache->slot.data[i].y = coord_data + (num_slots * num_atoms) * 1 + num_atoms * i;
        cache->slot.data[i].z = coord_data + (num_slots * num_atoms) * 2 + num_atoms * i;
    }

    return true;
}

void md_frame_cache_free(md_frame_cache_t* cache) {
    ASSERT(cache);
    ASSERT(cache->magic == CACHE_MAGIC);
    if (cache->buf) {
        for (int64_t i = 0; i < cache->slot.count; ++i) {
            md_semaphore_destroy(&cache->slot.lock[i]);
        }

        ASSERT(cache->alloc);
        md_array_free(cache->buf, cache->alloc);
    }
    memset(cache, 0, sizeof(md_frame_cache_t));
}

#include <stdio.h>

static inline void md_frame_cache_frame_lock_aquire(struct md_frame_cache_lock_t* lock) {
    ASSERT(lock);
    bool success = md_semaphore_aquire((md_semaphore_t*)lock);
#if 0
    char buf[512] = {0};
    md_thread_id_t id = md_thread_id();
    snprintf(buf, 512, "thread-id: %llu", id);
    md_printf(MD_LOG_TYPE_DEBUG, "AQUIRE LOCK \t%llu: %s", (uint64_t)lock, buf);
#endif
    ASSERT(success);
}

void md_frame_cache_frame_lock_release(struct md_frame_cache_lock_t* lock) {
    ASSERT(lock);
    bool success = md_semaphore_release((md_semaphore_t*)lock);
#if 0
    md_printf(MD_LOG_TYPE_DEBUG, "RELEASE\t%llu\n", (uint64_t)lock);
#endif
    ASSERT(success);
}

void md_frame_cache_clear(md_frame_cache_t* cache) {
    ASSERT(cache);
    ASSERT(cache->magic == CACHE_MAGIC);

    // If the frame is already in cache -> return data
    for (int64_t i = 0; i < cache->slot.count; ++i) {
        md_frame_cache_frame_lock_aquire((struct md_frame_cache_lock_t*)&cache->slot.lock[i]);
        cache->slot.header[i].frame_index = 0xFFFFFFFFU;
        cache->slot.header[i].access_count = 0;
        md_frame_cache_frame_lock_release((struct md_frame_cache_lock_t*)&cache->slot.lock[i]);
    }
}

bool md_frame_cache_find_or_reserve(md_frame_cache_t* cache, int64_t frame_idx, md_frame_data_t** frame_data, struct md_frame_cache_lock_t** frame_lock) {
    ASSERT(cache);
    ASSERT(cache->magic == CACHE_MAGIC);
    ASSERT(cache->traj);
    ASSERT(frame_idx >= 0);
    ASSERT(frame_lock);

    const int64_t start_slot = ((frame_idx % cache->slot.count) / CACHE_ASSOCIATIVITY) * CACHE_ASSOCIATIVITY;

    int64_t max_count_idx = -1;
    int64_t max_count = -1;

    // If the frame is already in cache -> return data
    for (int64_t i = start_slot; i < start_slot + CACHE_ASSOCIATIVITY; ++i) {
        md_frame_cache_frame_lock_aquire((struct md_frame_cache_lock_t*)&cache->slot.lock[i]);
        if (cache->slot.header[i].frame_index == (uint32_t)frame_idx) {
            cache->slot.header[i].access_count = 0;
            *frame_lock = (struct md_frame_cache_lock_t*)&cache->slot.lock[i];
            if (frame_data) *frame_data = &cache->slot.data[i];
            return true;
        }

        cache->slot.header[i].access_count += 1;
        if (cache->slot.header[i].access_count > max_count) {
            max_count = cache->slot.header[i].access_count;
            max_count_idx = i;
        }
        md_frame_cache_frame_lock_release((struct md_frame_cache_lock_t*)&cache->slot.lock[i]);
    }

    int64_t slot_idx = max_count_idx;
    ASSERT(slot_idx != -1);

    md_frame_cache_frame_lock_aquire((struct md_frame_cache_lock_t*)&cache->slot.lock[slot_idx]);

    cache->slot.header[slot_idx].access_count = 0;
    cache->slot.header[slot_idx].frame_index = (uint32_t)frame_idx;
    cache->slot.data[slot_idx].header.index = (int32_t)frame_idx;

    *frame_lock = (struct md_frame_cache_lock_t*)&cache->slot.lock[slot_idx];
    if (frame_data) *frame_data = &cache->slot.data[slot_idx];

    return false;
}

bool md_frame_cache_load_frame_data(md_frame_cache_t* cache, int64_t frame_idx, float* x, float* y, float* z, float box[3][3], double* timestamp) {
    ASSERT(cache);
    ASSERT(cache->magic == CACHE_MAGIC);
    ASSERT(cache->traj);

    if (frame_idx < 0 || md_trajectory_num_frames(cache->traj) <= frame_idx) return false;

    md_frame_data_t* data = NULL;
    struct md_frame_cache_lock_t* lock = NULL;
    if (!md_frame_cache_find_or_reserve(cache, frame_idx, &data, &lock)) {
        md_trajectory_frame_header_t header = {0};
        if (md_trajectory_load_frame(cache->traj, frame_idx, &data->header, data->x, data->y, data->z)) {
            data->header.index = frame_idx;
        }
        (void)header;
        ASSERT(data && (int64_t)data->header.index == frame_idx);
    }

    if (x || y || z || box || timestamp) {
        ASSERT(data);
        ASSERT(lock);

        if (x)   memcpy(x,   data->x,   data->header.num_atoms * sizeof(float));
        if (y)   memcpy(y,   data->y,   data->header.num_atoms * sizeof(float));
        if (z)   memcpy(z,   data->z,   data->header.num_atoms * sizeof(float));
        if (box) memcpy(box, data->header.box, sizeof(data->header.box));
        if (timestamp) *timestamp = data->header.timestamp;
    }

    md_frame_cache_frame_lock_release(lock);
    return true;
}
