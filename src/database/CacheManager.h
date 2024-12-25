#pragma once
#include <hiredis.h>
#include <thread>
#include <functional>
#include <string>

class CacheManager {
public:
    using MessageHandler = std::function<void(int, std::string)>;
    CacheManager();
    ~CacheManager();

    bool connect();

    bool publish(int channel, std::string message);

    bool subscribe(int channel);

    bool unsubscribe(int channel);

    void observer_channel_message();

    void init_notify_handler(MessageHandler fn);

private:
    redisContext* m_publish_context{ nullptr };
    redisContext*   m_subcribe_context{ nullptr };

    MessageHandler  m_notify_message_handler;
};

