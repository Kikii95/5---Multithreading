#include "GameQueue.h"
#include "SafeQueue.h"

// C++ implementation using SafeQueue template
typedef SafeQueue<Message, 100> MessageQueue;

extern "C" {

    GameMessageQueue CreateMessageQueue() {
        return new MessageQueue();
    }

    void DestroyMessageQueue(GameMessageQueue queue) {
        delete static_cast<MessageQueue*>(queue);
    }

    void PushMessage(GameMessageQueue queue, Message msg) {
        static_cast<MessageQueue*>(queue)->push(msg);
    }

    int TryPushMessage(GameMessageQueue queue, Message msg) {
        return static_cast<MessageQueue*>(queue)->tryPush(msg) ? 1 : 0;
    }

    Message PopMessage(GameMessageQueue queue) {
        return static_cast<MessageQueue*>(queue)->pop();
    }

    int TryPopMessage(GameMessageQueue queue, Message* msg) {
        return static_cast<MessageQueue*>(queue)->tryPop(*msg) ? 1 : 0;
    }

    int GetMessageQueueSize(GameMessageQueue queue) {
        return static_cast<MessageQueue*>(queue)->size();
    }

}
