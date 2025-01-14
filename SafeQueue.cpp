#include "SafeQueue.h"



SafeQueue::SafeQueue() {}
SafeQueue::~SafeQueue() {}

void SafeQueue::push(UrlCrowlingTask task)
{
    // добавить новую задачу в очередь

    std::lock_guard lock_guard(mutex_);
    queue_.push(task);
    // уведомление, что очередь не пуста
    data_cond.notify_all();
}

void SafeQueue::pop(UrlCrowlingTask &task)
{
    // вынуть задачу из очереди

    std::unique_lock<std::mutex> lk(mutex_);
    // ожидание, что очередь не пуста
    data_cond.wait(lk, [this] {return !queue_.empty(); });
    task = std::move(queue_.front());
    queue_.pop();
}

bool SafeQueue::isEmpty()
{
    // проверить, пуста ли очередь
    return queue_.empty();
}
