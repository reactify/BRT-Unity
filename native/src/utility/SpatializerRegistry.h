
#pragma once

struct SpatializerState
{
    CMonoBuffer<float> buffer;
    Common::CTransform sourceTransform;
    Common::CTransform listenerTransform;
};


//============================================================================
class SpatializerRegistry
{
public:
    using Map = std::unordered_map<int, SpatializerState>;
    using Snapshot = std::shared_ptr<Map>;

    static SpatializerRegistry& instance()
    {
        static SpatializerRegistry r;
        return r;
    }

    Snapshot get() const noexcept
    {
        return std::atomic_load_explicit (&current, std::memory_order_acquire);
    }

    void set (int id, SpatializerState s)
    {
        auto old = std::atomic_load_explicit (&current, std::memory_order_acquire);

        if (! old)
            old = std::make_shared<Map>();

        auto next = std::make_shared<Map> (*old);
        (*next)[id] = std::move (s);

        std::atomic_store_explicit (&current, next, std::memory_order_release);
    }

    void erase (int id)
    {
        auto old = std::atomic_load_explicit (&current, std::memory_order_acquire);
        if (!old)
            return;

        auto next = std::make_shared<Map> (*old);
        next->erase (id);

        std::atomic_store_explicit (&current, next, std::memory_order_release);
    }

private:
    std::shared_ptr<Map> current = std::make_shared<Map>();
};
