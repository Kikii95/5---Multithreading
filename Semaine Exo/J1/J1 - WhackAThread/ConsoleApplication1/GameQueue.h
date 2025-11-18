#ifndef GAMEQUEUE_H
#define GAMEQUEUE_H

#include "Message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* GameMessageQueue;

GameMessageQueue CreateMessageQueue();

void DestroyMessageQueue(GameMessageQueue queue);

void PushMessage(GameMessageQueue queue, Message msg);

int TryPushMessage(GameMessageQueue queue, Message msg);

Message PopMessage(GameMessageQueue queue);

int TryPopMessage(GameMessageQueue queue, Message* msg);

int GetMessageQueueSize(GameMessageQueue queue);

#ifdef __cplusplus
}
#endif

#endif // GAMEQUEUE_H
