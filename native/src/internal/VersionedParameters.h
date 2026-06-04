
namespace BRTUnity
{

//==============================================================================
template <typename Parameters>
struct VersionedParameters
{
    std::atomic<uint32_t> version {0};
    Parameters buffers[2];
    std::atomic<uint32_t> activeBuffer {0};
    
    // Main thread: update one or more fields
    template <typename Func>
    void update (Func&& f)
    {
        uint32_t newIndex = (activeBuffer.load (std::memory_order_relaxed) + 1) % 2;
        Parameters newParams = buffers[newIndex]; // Copy current state
        f (newParams); // Modify it
        buffers[newIndex] = newParams;
        activeBuffer.store (newIndex, std::memory_order_release);
        version.fetch_add (1, std::memory_order_acq_rel);
    }
    
    // Any thread: fast read-only access, no locking
    const Parameters& get() const noexcept
    {
        uint32_t index = activeBuffer.load (std::memory_order_acquire);
        return buffers[index];
    }
    
    // Any thread: check if changed since last known version
    bool hasChanged (uint32_t lastVersion) const noexcept
    {
        return version.load (std::memory_order_acquire) != lastVersion;
    }
    
    uint32_t getVersion() const noexcept
    {
        return version.load (std::memory_order_acquire);
    }
};

} // namespace BRTUnity
