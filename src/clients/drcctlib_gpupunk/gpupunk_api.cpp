#include "gpupunk_api.h"

using namespace gpupunk;
gpupunk::LockableMap<uint64_t, MemoryMap> memory_snapshot;

static int32_t host_op_id_start = 0xf0000000;

gp_result_t
gpupunk_memory_register(int32_t memory_id, uint64_t host_op_id, uint64_t start,
                        uint64_t end)
{
    PRINT("\nredshow->Enter redshow_memory_register\nmemory_id: %d\nhost_op_id: "
          "%lu\nstart: %lu\nend: "
          "%lu\n",
          memory_id, host_op_id, start, end);
    gp_result_t result = GPUPUNK_SUCCESS;
    MemoryMap memory_map;
    MemoryRange memory_range(start, end);
    memory_snapshot.lock();
    if (memory_snapshot.size() == 0) {
        auto memory = std::make_shared<Memory>(host_op_id, memory_id, memory_range);
        // First snapshot
        memory_map[memory_range] = memory;
        memory_snapshot[host_op_id] = memory_map;
    } else {
        auto iter = memory_snapshot.prev(host_op_id);
        if (iter != memory_snapshot.end()) {
            // Take a snapshot
            memory_map = iter->second;
            if (!memory_map.has(memory_range)) {
                auto memory = std::make_shared<Memory>(host_op_id, memory_id, memory_range);
                memory_map[memory_range] = memory;
                memory_snapshot[host_op_id] = memory_map;
            } else {
                result = GPUPUNK_ERROR_DUPLICATE_ENTRY;
            }
        } else {
            result = GPUPUNK_ERROR_NOT_EXIST_ENTRY;
        }
    }
    memory_snapshot.unlock();
    if (result == GPUPUNK_SUCCESS) {
        PRINT("Register memory_id %d\n", memory_id);
    }
    return result;
}
gp_result_t
gpupunk_memory_unregister(uint64_t host_op_id, uint64_t start, uint64_t end){
    PRINT("\nredshow->Enter redshow_memory_unregister\nhost_op_id: %lu\nstart: %lu\nend: %lu\n",
          host_op_id, start, end);
    gp_result_t result = GPUPUNK_SUCCESS;
    MemoryMap memory_map;
    MemoryRange memory_range(start, end);
    memory_snapshot.lock();
    auto snapshot_iter = memory_snapshot.prev(host_op_id);
    if (snapshot_iter != memory_snapshot.end()) {
        // Take a snapshot
        memory_map = snapshot_iter->second;
        auto memory_map_iter = memory_map.find(memory_range);
        if (memory_map_iter != memory_map.end()) {
            memory_map.erase(memory_map_iter);
            memory_snapshot[host_op_id] = memory_map;
            result = GPUPUNK_SUCCESS;
        } else {
            result = GPUPUNK_ERROR_NOT_EXIST_ENTRY;
        }
    } else {
        result = GPUPUNK_ERROR_NOT_EXIST_ENTRY;
    }
    memory_snapshot.unlock();

    return result;
}