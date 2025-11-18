#ifndef MESSAGE_H
#define MESSAGE_H

// Message types for game events
enum MessageType {
    MSG_MOLE_SPAWNED,   // Mole was created
    MSG_MOLE_KILLED,    // Mole was killed by user (+1 point)
    MSG_MOLE_EXPIRED    // Mole died from timeout (-1 point)
};

// Message structure for inter-thread communication
struct Message {
    MessageType type;
    int moleId;
    char key;  // Key associated with the mole (optional)

    Message() : type(MSG_MOLE_SPAWNED), moleId(0), key('\0') {}

    Message(MessageType t, int id, char k = '\0')
        : type(t), moleId(id), key(k) {}
};

#endif // MESSAGE_H
