#include "boost/optional.hpp"
#include <mutex>
#include <queue>

template <typename T>
class TaskQueue
{
public:
    /**
   * 終了条件となる進捗のゴールを登録
   */
    explicit TaskQueue(std::size_t target_progress);

    /**
   * 要素を挿入
   */
    void push(T val) noexcept;

    /**
   * 要素を取得しつつ削除
   */
    boost::optional<T> front_pop() noexcept;

    /**
   * 進捗を登録
   */
    void update(std::size_t progress) noexcept;

    /**
   * タスクの終了判定
   */
    bool terminate() noexcept;

private:
    std::queue<T> m_que;
    std::mutex m_mutex;
    std::size_t m_progress;
    std::size_t m_target_progress;
};

template <typename T>
TaskQueue<T>::TaskQueue(const std::size_t target_counter)
    : m_progress(0), m_target_progress(target_counter) {}

template <typename T>
void TaskQueue<T>::push(const T val) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_que.push(val);
}

template <typename T>
boost::optional<T> TaskQueue<T>::front_pop() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_que.empty())
    {
        return boost::none;
    }
    else
    {
        auto ret = m_que.front();
        m_que.pop();
        return ret;
    }
}

template <typename T>
void TaskQueue<T>::update(const std::size_t counter) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_progress += counter;
}

template <typename T>
bool TaskQueue<T>::terminate() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_progress == m_target_progress;
}