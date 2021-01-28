//
// Created by findhao on 1/20/21.
//
#ifndef DRCCTLIB_GPUPUNK_MEMORY_H
#define DRCCTLIB_GPUPUNK_MEMORY_H

#include <memory>
#include <mutex>
#include <set>
#include <cstdint>
#include <map>
namespace gpupunk {
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
struct MemoryRange {
    uint64_t start;
    uint64_t end;

    MemoryRange() = default;

    MemoryRange(uint64_t start, uint64_t end)
        : start(start)
        , end(end)
    {
    }

    bool
    operator<(const MemoryRange &other) const
    {
        return start < other.start;
    }
};

enum OperationType {
    OPERATION_TYPE_KERNEL = 0,
    OPERATION_TYPE_MEMORY = 1,
    OPERATION_TYPE_MEMCPY = 2,
    OPERATION_TYPE_MEMSET = 3,
    OPERATION_TYPE_COUNT = 4
};

const std::string
get_operation_type(OperationType type);

struct Operation {
    u64 op_id;
    i32 ctx_id;
    OperationType type;

    Operation() = default;

    Operation(u64 op_id, i32 ctx_id, OperationType type)
        : op_id(op_id)
        , ctx_id(ctx_id)
        , type(type)
    {
    }

    virtual ~Operation() {
    }
};

typedef std::shared_ptr<Operation> OperationPtr;

struct Memory : public Operation {
    MemoryRange memory_range;
    size_t len;
    std::shared_ptr<u8[]> value;
    std::shared_ptr<u8[]> value_cache;

    Memory()
        : Operation(0, 0, OPERATION_TYPE_MEMORY)
    {
    }

    Memory(u64 op_id, i32 ctx_id)
        : Operation(op_id, ctx_id, OPERATION_TYPE_MEMORY)
        , len(0)
    {
    }

    Memory(u64 op_id, i32 ctx_id, u64 start, size_t len)
        : Operation(op_id, ctx_id, OPERATION_TYPE_MEMORY)
        , memory_range(start, start + len)
        , len(len)
    {
    }

    Memory(u64 op_id, i32 ctx_id, MemoryRange &memory_range)
        : Operation(op_id, ctx_id, OPERATION_TYPE_MEMORY)
        , memory_range(memory_range)
        , len(memory_range.end - memory_range.start)
        , value(new u8[len])
        , value_cache(new u8[len])
    {
    }

    bool
    operator<(const Memory &other) const
    {
        return this->memory_range < other.memory_range;
    }

    virtual ~Memory()
    {
    }
};

template <typename K> class Set : public std::set<K> {
public:
    Set() = default;

    typename Set<K>::iterator
    prev(const K &key)
    {
        auto iter = this->upper_bound(key);
        if (iter == this->begin()) {
            return this->end();
        } else {
            --iter;
            return iter;
        }
    }

    typename Set<K>::const_iterator
    prev(const K &key) const
    {
        auto iter = this->upper_bound(key);
        if (iter == this->begin()) {
            return this->end();
        } else {
            --iter;
            return iter;
        }
    }

    // Not conflict with "contains" in C++20
    bool
    has(const K &k) const noexcept
    {
        return this->find(k) != this->end();
    }
};

template <typename K, typename V> class Map : public std::map<K, V> {
public:
    Map() = default;

    typename Map<K, V>::iterator
    prev(const K &key)
    {
        auto iter = this->upper_bound(key);
        if (iter == this->begin()) {
            return this->end();
        } else {
            --iter;
            return iter;
        }
    }

    typename Map<K, V>::const_iterator
    prev(const K &key) const
    {
        auto iter = this->upper_bound(key);
        if (iter == this->begin()) {
            return this->end();
        } else {
            --iter;
            return iter;
        }
    }

    // Not conflict with "contains" in C++20
    bool
    has(const K &k) const noexcept
    {
        return this->find(k) != this->end();
    }
};

template <typename K, typename V> class LockableMap : public Map<K, V> {
public:
    void
    lock() const
    {
        _lock.lock();
    }
    void
    unlock() const
    {
        _lock.unlock();
    }

    LockableMap() = default;

private:
    mutable std::mutex _lock;
};

} // namespace gpupunk
#endif // DRCCTLIB_GPUPUNK_MEMORY_H
