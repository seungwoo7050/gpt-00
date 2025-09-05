// [SEQUENCE: MVP2-32]
#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

// Forward declaration to avoid circular dependency
struct log_server;

void handle_query_connection(struct log_server* server);

#endif // QUERY_HANDLER_H
