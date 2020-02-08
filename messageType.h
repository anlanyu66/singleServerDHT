//
// Created by Anlan Yu on 2/7/20.
//

#ifndef DHT_CLIENT_MESSAGETYPE_H
#define DHT_CLIENT_MESSAGETYPE_H
enum OperationType {
    PUT = 1,
    GET,
};

template <typename T>
struct RequestInfo {
    long operation_type;
    int hash_key;
    T hash_value;
};

template <typename T>
struct ResponseInfo {
    long operation_type;
    int hash_key;
    int status;
    T hash_value;
};
#endif //DHT_CLIENT_MESSAGETYPE_H
