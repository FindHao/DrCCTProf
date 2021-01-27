//
// Created by findhao on 1/20/21.
//

#ifndef DRCCTLIB_GPUPUNK_GPUPUNK_API_H
#define DRCCTLIB_GPUPUNK_GPUPUNK_API_H
#define DEBUG 1
#ifdef DEBUG
#    define PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#    define PRINT(...)
#endif
#ifdef __cplusplus
#    define EXTERNC extern "C"
#else
#    define EXTERNC
#endif
#include "memory.h"

typedef gpupunk::Map<gpupunk::MemoryRange, std::shared_ptr<gpupunk::Memory>> MemoryMap;
typedef enum gp_result {
    GPUPUNK_SUCCESS = 0,
    GPUPUNK_ERROR_NOT_IMPL = 1,
    GPUPUNK_ERROR_NOT_EXIST_ENTRY = 2,
    GPUPUNK_ERROR_DUPLICATE_ENTRY = 3,
    GPUPUNK_ERROR_NOT_REGISTER_CALLBACK = 4,
    GPUPUNK_ERROR_NO_SUCH_FILE = 5,
    GPUPUNK_ERROR_FAILED_ANALYZE_CUBIN = 6,
    GPUPUNK_ERROR_FAILED_ANALYZE_TRACE = 7,
    GPUPUNK_ERROR_NO_SUCH_APPROX = 8,
    GPUPUNK_ERROR_NO_SUCH_DATA_TYPE = 9,
    GPUPUNK_ERROR_NO_SUCH_ANALYSIS = 10
} gp_result_t;

gp_result_t
gpupunk_memory_register(int32_t memory_id, uint64_t host_opt_id, uint64_t start,
                        uint64_t end);

#endif // DRCCTLIB_GPUPUNK_GPUPUNK_API_H
