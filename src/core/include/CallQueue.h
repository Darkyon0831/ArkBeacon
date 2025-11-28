#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace ArkBeacon
{
    template <typename T>
    class CallQueue
    {
    public:
    
        struct CallRequest {
            std::function<void(std::shared_ptr<T>)> call;
        };

        CallQueue() = default;
        ~CallQueue() = default;

        void Init(std::shared_ptr<T> handler)
        {
            m_handler = handler;
            m_stopped = false;

            if (m_start_callback)
                m_start_callback();
        }

        void Push(const CallRequest& request)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            m_queue.push(request);
            m_condition.notify_one();
        }

        bool Pop(CallRequest& request)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [&]{ return !m_queue.empty() || m_stopped; });
            if (m_queue.empty())
                return false;

            request = m_queue.front();
            m_queue.pop();
            return true;
        }

        void Stop()
        {
            if (m_stop_callback)
                m_stop_callback();
            
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopped = true;
            m_condition.notify_all();
        }

        void SetStartCallback(std::function<void()> start_callback)
        {
            m_start_callback = start_callback;
        }

        void SetStopCallback(std::function<void()> stop_callback)
        {
            m_stop_callback = stop_callback;
        }

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<CallRequest> m_queue;
        bool m_stopped;

        std::function<void()> m_start_callback;
        std::function<void()> m_stop_callback;

        std::shared_ptr<T> m_handler;
    };
} // namespace ArkBeacon