#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <queue>
#include <mutex>


struct UrlCrowlingTask {
    // Структура, описывающая задачу на обход ресурса
    std::string domain;
    std::string path;
    unsigned short depth;
};


class SafeQueue
{
public:
    SafeQueue();
    ~SafeQueue();
    void push(UrlCrowlingTask task);
    void pop(UrlCrowlingTask& task);
    bool isEmpty();

private:
    // внутренняя очередь для хранения задач
    std::queue<UrlCrowlingTask> queue_;
    // мьютекс для блокировки на момент добалвения новой задачи в очередь
    std::mutex mutex_;
    // переменная, используемая для ожидания непустой очереди и уведомления о непустоте
    std::condition_variable data_cond;
};

#endif // SAFEQUEUE_H
