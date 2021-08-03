#include "worker.h"

// C++ Standard Library
#include <thread>       // this_thread::sleep_for
#include <chrono>       // chrono::duration

// POSIX
#include <unistd.h>     // close

// DictDB
#include <libdictdb/protocol.h>


// Worker entry point.
void worker(std::shared_ptr<dictdb_worker_context_t> context) {
  std::vector<std::byte> buffer;

  while (!context->cancel) {
    // retrieve the next client socket
    int client_socket;
    while (!context->client_sockets->try_pop(client_socket)) {
      if (context->cancel) {
        break;
      } else {
        // sleep for a bit and try again
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }

    if (context->cancel) {
      break;
    }

    // process client requests
    while (!context->cancel) {
      // TODO: poll socket for data before reading to avoid blocking on read()
      // and not being able to quit if the server shuts down

      try {
        // receive and decode the request message
        receive_message(client_socket, buffer);
        dictdb_request_t request;
        decode_request(buffer, request);

        // process the request operation and generate a response
        dictdb_response_t response;
        switch (request.type) {
          case OperationType::PING: {
            response.result = OperationResult::SUCCESS;
            break;
          }

          case OperationType::INSERT: {
            auto value = std::pair<std::string, bool>(request.word, true);
            response.result = context->db->words.insert(value) ?
              OperationResult::SUCCESS :
              OperationResult::FAILURE;

            break;
          }

          case OperationType::SEARCH: {
            auto count = context->db->words.count(request.word);
            response.result = (count > 0) ?
              OperationResult::SUCCESS :
              OperationResult::FAILURE;

            break;
          }

          case OperationType::DELETE: {
            response.result = context->db->words.erase(request.word) ?
              OperationResult::SUCCESS :
              OperationResult::FAILURE;

            break;
          }
        }

        // encode and send the response message
        encode_response(buffer, response);
        send_message(client_socket, buffer);
      }
      catch (std::runtime_error& e) {
        // protocol error or socket is closed
        break;
      }
    }

    // close the client socket
    close(client_socket);
  }
}
