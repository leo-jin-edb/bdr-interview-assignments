#pragma once

// C++ Standard Library
#include <string>
#include <vector>
#include <stdexcept>    // runtime_error

// POSIX
#include <sys/types.h>  // ssize_t


/**
 * Database operation type.
 */
typedef enum class OperationType:uint8_t {
  PING = 0,
  INSERT = 1,
  SEARCH = 2,
  DELETE = 3
} dictdb_op_type_t;

/**
 * Request message.
 */
typedef struct {
  dictdb_op_type_t type;
  std::string word;
} dictdb_request_t;

/**
 * Operation result.
 */
typedef enum class OperationResult:uint8_t {
  /**
   * Server error.
   */
  ERROR = 0,

  /**
   * Ping: Server is alive and can process requests.
   * Insert: Word inserted.
   * Delete: Word deleted.
   * Search: Word found.
   */
  SUCCESS = 1,

  /**
   * Ping: Server is not able to process requests.
   * Insert: Word already present.
   * Delete: Word already absent.
   * Search: Word not found.
   */
  FAILURE = 2
} dictdb_op_result_t;

/**
 * Response message.
 */
typedef struct {
  dictdb_op_result_t result;
} dictdb_response_t;

/**
 * Encode a request message into bytes.
 */
void encode_request(std::vector<std::byte>& buffer, const dictdb_request_t& request);

/**
 * Decode a request message from bytes.
 */
void decode_request(std::vector<std::byte>& buffer, dictdb_request_t& request);

/**
 * Encode a response message into bytes.
 */
void encode_response(std::vector<std::byte>& buffer, const dictdb_response_t& response);

/**
 * Decode a response message from bytes.
 */
void decode_response(std::vector<std::byte>& buffer, dictdb_response_t& response);

/**
 * Write a message from a buffer to a file.
 */
void send_message(int fd, std::vector<std::byte>& buffer);

/**
 * Read a message from a file into a buffer.
 */
void receive_message(int fd, std::vector<std::byte>& buffer);
