#include "CacheManager.h"
#include "AsyncLog.h"
#include <iostream>

CacheManager::CacheManager(){}

CacheManager::~CacheManager() {
    if (m_publish_context != nullptr) {
        redisFree(m_publish_context);
    }

    if (m_subcribe_context != nullptr) {
        redisFree(m_subcribe_context);
    }
}

bool CacheManager::connect() {
    // connect for publishment
    m_publish_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == m_publish_context) {
        LOGE("connect redis failed!");
        return false;
    }

    // connect for subcribe
    m_subcribe_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == m_subcribe_context) {
        LOGE("connect redis failed!");
        return false;
    }

    // listen message from redis pip, then do callback function to handle it
    std::thread listen_channel_thread(std::bind(&CacheManager::observer_channel_message, this));
    listen_channel_thread.detach();

    LOGI("connect redis-server success!");

    return true;
}

// send message to channel, the format: redisCommand = redisAppendCommand + redisBufferWrite + redisGetReply
bool CacheManager::publish(int channel, std::string message) {
    redisReply* reply = (redisReply*)redisCommand(m_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply) {
        LOGE("publish command failed!");
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// subscribe message from specified channel
bool CacheManager::subscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->m_subcribe_context, "SUBSCRIBE %d", channel)) {
        LOGE("subscribe command failed!");
        return false;
    }

    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->m_subcribe_context, &done)) {
            LOGE("subscribe command failed!");
            return false;
        }
    }

    return true;
}

// unsubscribe message
bool CacheManager::unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->m_subcribe_context, "UNSUBSCRIBE %d", channel)) {
        LOGE("unsubscribe command failed!");
        return false;
    }

    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->m_subcribe_context, &done)) {
            LOGE("unsubscribe command failed!");
            return false;
        }
    }
    return true;
}

// observer
void CacheManager::observer_channel_message() {
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(this->m_subcribe_context, (void**)&reply)) {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            m_notify_message_handler(std::stoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }
}

void CacheManager::init_notify_handler(MessageHandler fn) {
    this->m_notify_message_handler = fn;
}