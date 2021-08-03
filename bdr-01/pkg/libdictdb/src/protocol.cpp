#include "protocol.h"

// C++ Standard Library
#include <cassert>    // assert
#include <limits>     // std::numeric_limits

// POSIX
#include <unistd.h>   // read, write

// Encode a request message into bytes.
void encode_request(std::vector<std::byte>& buffer, const dictdb_request_t& request) {
  assert((2 + request.word.size()) < std::numeric_limits<uint8_t>::max());  // Message size

  buffer.clear(); // O(n) for n = buffer.size()
  buffer.reserve(2 + request.word.size()); // O(n) for n = buffer.size()
  buffer.push_back(static_cast<std::byte>(2 + request.word.size()));  // ~O(1)
  buffer.push_back(static_cast<std::byte>(request.type)); // ~O(1)

  // word.size() * ~O(1) -> O(n) for n = word.size()
  std::transform(
    request.word.begin(),
    request.word.end(),
    std::back_inserter(buffer),
    [](unsigned char c) { return std::byte{c}; });
}

// Decode a request message from bytes.
void decode_request(std::vector<std::byte>& buffer, dictdb_request_t& request) {
  assert(buffer.size() > 1);  // Size and Operation Type bytes
  assert(static_cast<uint8_t>(buffer[0]) == buffer.size()); // Message size

  request.type = OperationType{buffer[1]}; // O(1)
  request.word = std::string(
    reinterpret_cast<const char *>(&buffer[2]),
    static_cast<uint8_t>(buffer[0]) - 2); // O(n) for n = buffer[1]
}

// Encode a response message into bytes.
void encode_response(std::vector<std::byte>& buffer, const dictdb_response_t& response) {
  buffer.clear(); // O(n) for n = buffer.size()
  buffer.reserve(2);  // O(n) for n = buffer.size()
  buffer.push_back(static_cast<std::byte>(2));  // ~O(1)
  buffer.push_back(static_cast<std::byte>(response.result));  // ~(O1)
}

// Decode a response message from bytes.
void decode_response(std::vector<std::byte>& buffer, dictdb_response_t& response) {
  assert(buffer.size() == 2); // Size and Result bytes
  assert(static_cast<uint8_t>(buffer[0]) == buffer.size()); // Message size

  response.result = OperationResult{buffer[1]}; // O(1)
}

// Write a message from a buffer to a file.
void send_message(int fd, std::vector<std::byte>& buffer) {
  assert(buffer.size() > 1);  // At least the size byte and one more byte after

  uint32_t index = 0;
  ssize_t bytes = 0;

  do {
    // https://man7.org/linux/man-pages/man2/write.2.html
    bytes = write(fd, &buffer[index], buffer.size() - index);
    if (bytes == 0) {
      throw std::runtime_error("socket closed");
    }

    index += bytes;
  } while (bytes != -1 && index < buffer.size());

  if (bytes == -1) {
    throw std::runtime_error("socket error");
  }
}

// Read a message from a file into a buffer.
void receive_message(int fd, std::vector<std::byte>& buffer) {
  buffer.resize(std::numeric_limits<uint8_t>::max()); // O(1) (n=255)

  uint32_t index = 0;
  ssize_t bytes = 0;

  do {
    // https://man7.org/linux/man-pages/man2/read.2.html
    bytes = read(fd, &buffer[index], buffer.size() - index);
    if (bytes == 0) {
      throw std::runtime_error("socket closed");
    }

    if (index == 0) {
      // resize the buffer based on the message size received as the first byte
      buffer.resize(static_cast<uint8_t>(buffer[0])); // O(n) for n = buffer[0]
    }

    index += bytes;
  } while (bytes != -1 && index < buffer.size());

  if (bytes == -1) {
    throw std::runtime_error("socket error");
  }
}
