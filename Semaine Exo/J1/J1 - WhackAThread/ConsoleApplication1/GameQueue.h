#ifndef GAMEQUEUE_H
#define GAMEQUEUE_H

#include "Message.h"

#ifdef __cplusplus
extern "C" {
#endif

// Opaque pointer to C++ SafeQueue (C-compatible)
typedef void* GameMessageQueue;

// Create message queue
GameMessageQueue CreateMessageQueue();

// Destroy message queue
void DestroyMessageQueue(GameMessageQueue queue);

// Push message (blocking)
void PushMessage(GameMessageQueue queue, Message msg);

// Try push message (non-blocking)
int TryPushMessage(GameMessageQueue queue, Message msg);

// Pop message (blocking)
Message PopMessage(GameMessageQueue queue);

// Try pop message (non-blocking, returns 0 if empty)
int TryPopMessage(GameMessageQueue queue, Message* msg);

// Get queue size
int GetMessageQueueSize(GameMessageQueue queue);

#ifdef __cplusplus
}
#endif

#endif // GAMEQUEUE_H
