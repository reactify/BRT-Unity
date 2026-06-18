
#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <unordered_set>
#include <functional>

class IdPool {
public:
    int acquire() {
        std::lock_guard<std::mutex> lock(mutex_);

        int id;
        if (!free_ids_.empty()) {
            id = free_ids_.top();
            free_ids_.pop();
        } else {
            id = next_id_++;
        }

        active_.insert(id);
        return id;
    }

    void release(int id) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = active_.find(id);
        if (it == active_.end()) return; // ignore invalid/double release

        active_.erase(it);
        free_ids_.push(id);
    }

    template <typename Func>
    void for_each_active(Func&& fn) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (int id : active_) {
            fn(id);
        }
    }

private:
    std::mutex mutex_;
    std::priority_queue<int, std::vector<int>, std::greater<int>> free_ids_;
    std::unordered_set<int> active_;
    int next_id_ = 0;
};
