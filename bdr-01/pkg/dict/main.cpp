// C++ Standard Library
#include <string>         // string
#include <iostream>       // cout, cerr, endl
#include <vector>         // vector
#include <unordered_map>  // unordered_map
#include <cstddef>        // to_integer
#include <span>           // as_bytes, span

// XXX
#include <sys/socket.h>   // socket, AF_*?
#include <sys/un.h>       // sockaddr_un
#include <unistd.h>       // close

// CxxOpts
#include <cxxopts.hpp>

// DictDB
#include <libdictdb/protocol.h>


// Application entry point.
int main(int argc, char* argv[]) {
  // parse command line arguments
  cxxopts::Options options("dictdb", "dictionary database server");
  options.add_options()
    ("h,help", "Display usage help", cxxopts::value<bool>()->default_value("false"))
    ("s,socket", "Socket file", cxxopts::value<std::string>()->default_value("dictdb.socket"))
    ("c,command", "ping,insert,search,delete", cxxopts::value<std::string>()->default_value("ping"))
    ("w,word", "Word", cxxopts::value<std::vector<std::string>>());

  options.parse_positional({"command", "word"});
  options.positional_help("<ping|insert|search|delete> <word> [<word> ...]");

  auto args = options.parse(argc, argv);
  if (args.count("help")) {
    std::cout << options.help() << std::endl;
    exit(EXIT_SUCCESS);
  }

  auto socket_file = args["socket"].as<std::string>();
  auto command = args["command"].as<std::string>();
  auto words = args["word"].as<std::vector<std::string>>();

  std::unordered_map<std::string, OperationType> commands;
  commands["ping"] = OperationType::PING;
  commands["insert"] = OperationType::INSERT;
  commands["search"] = OperationType::SEARCH;
  commands["delete"] = OperationType::DELETE;

  if (commands.count(command) == 0) {
    std::cerr << "invalid command: " << command << std::endl;
    exit(EXIT_FAILURE);
  }

  for (auto word : words) {
    if (word.size() > 255) {
      std::cerr << "word is too long: " << word << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // create the socket
  int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (client_socket == -1) {
    std::cerr << "failed to create socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  // connect the socket
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_file.c_str(), sizeof(addr.sun_path) - 1);

  int rv = connect(client_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
  if (rv == -1) {
    std::cerr << "failed to connect socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  // create requests and process responses
  std::vector<std::byte> buffer;
  dictdb_response_t response;
  dictdb_request_t request;

  request.type = commands[command];
  for (auto word : words) {
    // create the request
    request.word = word;

    // encode and send the request
    encode_request(buffer, request);
    send_message(client_socket, buffer);

    // receive the response and decode it
    receive_message(client_socket, buffer);
    decode_response(buffer, response);

    // display the response if multiple words were provided
    if (words.size() > 1) {
      switch (response.result) {
        case OperationResult::ERROR:
          std::cerr << command << "\t" << word << "\tERROR" << std::endl;
          break;

        case OperationResult::SUCCESS:
          std::cout << command << "\t" << word << "\tSUCCESS" << std::endl;
          break;

        case OperationResult::FAILURE:
          std::cout << command << "\t" << word << "\tFAILURE" << std::endl;
          break;
      }
    }
  }

  // close the socket
  rv = close(client_socket);
  if (rv == -1) {
    std::cerr << "failed to close socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  // exit with an error code based on the last response received
  switch (response.result) {
    case OperationResult::ERROR:
      std::cerr << "operation failed" << std::endl;
      exit(EXIT_FAILURE);

    case OperationResult::SUCCESS:
      exit(EXIT_SUCCESS);

    case OperationResult::FAILURE:
      exit(EXIT_FAILURE);
  }
}
