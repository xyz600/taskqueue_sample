#include "task_queue.h"

#include <cassert>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// [left, right)
struct Range
{
    std::size_t left;
    std::size_t right;

    Range(std::size_t left, std::size_t right) : left(left), right(right)
    {
        assert(left < right);
    }

    std::size_t size() const noexcept { return right - left; }
};

// [left, ret), [ret, right) に分割したい。今回は適当に選ぶ
std::size_t select_best_split(const Range &range)
{
    // 適当
    return (range.right * range.left + range.right + range.left + 1) %
               (range.size() - 1) +
           range.left + 1;
}

int main()
{
    const std::size_t object_size = 16384;
    const std::size_t object_threashold_in_leaf = 16;

    std::vector<std::size_t> counter(object_size, 0);

    TaskQueue<Range> que(object_size);
    que.push(Range(0, object_size));

    constexpr std::size_t num_threads = 8;
    std::vector<std::thread> threads(num_threads);

    // 並列化処理開始
    for (auto i = 0u; i < num_threads; ++i)
    {
        threads[i] = std::thread([&]() {
            while (!que.terminate())
            {
                auto may_range = que.front_pop();

                if (!may_range)
                {
                    // 今はタスクが queue にないが、全体のタスクは終了してないので少し待つ
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                else
                {
                    // 処理対象のノードが存在した
                    const auto range = may_range.value();

                    if (range.size() <= object_threashold_in_leaf)
                    {
                        // leaf node なら、処理対象の object に mark
                        for (auto i = range.left; i < range.right; i++)
                        {
                            counter[i]++;
                        }
                        // leaf が抱えるオブジェクト数を進捗報告
                        que.update(range.size());
                    }
                    else
                    {
                        // 分割箇所を決める
                        const auto middle = select_best_split(range);
                        // 部分木を再帰的に処理
                        que.push(Range(range.left, middle));
                        que.push(Range(middle, range.right));
                    }
                }
            }
        });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }

    // 全 object 一度ずつ触れたか確認
    for (auto &c : counter)
    {
        assert(c == 1);
    }
}