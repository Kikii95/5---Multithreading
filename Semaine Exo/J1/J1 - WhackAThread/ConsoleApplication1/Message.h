#ifndef MESSAGE_H
#define MESSAGE_H

enum MessageType {
    MSG_MOLE_SPAWNED,   
    MSG_MOLE_KILLED,   
    MSG_MOLE_EXPIRED    
};

struct Message {
    MessageType type;
    int moleId;
    char key; 

    Message() : type(MSG_MOLE_SPAWNED), moleId(0), key('\0') {}

    Message(MessageType t, int id, char k = '\0')
        : type(t), moleId(id), key(k) {}
};

#endif // MESSAGE_H
