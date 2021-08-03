// C++ standard library
#include <memory>       // shared_ptr, unique_ptr
#include <string>
#include <iostream>     // cout, endl
#include <vector>

// C standard library
#include <signal.h>     // sigaction, signal

// POSIX
#include <sys/socket.h> // socket, AF_*?
#include <sys/un.h>     // sockaddr_un
#include <unistd.h>     // close
#include <poll.h>       // pollfd, poll

// CxxOpts
#include <cxxopts.hpp>

// DictDB
#include "data.h"
#include "worker.h"


// Flag to indicate if the process should continue running.
bool running = false;

// Process signals.
void handle_signal(int sig) {
  switch (sig) {
    // XXX: reload daemon configuration files and reopen file descriptors
    case SIGHUP:
      break;

    // stop running
    case SIGTERM:
    case SIGINT:
      running = false;
      break;
  }
}

// Application entry point.
int main(int argc, char* argv[]) {
  // parse command line arguments
  cxxopts::Options options("dictdb", "dictionary database server");
  options.add_options()
    ("h,help", "Display usage help", cxxopts::value<bool>()->default_value("false"))
    ("s,socket", "Socket file", cxxopts::value<std::string>()->default_value("dictdb.socket"))
    ("b,backlog", "Connection backlog", cxxopts::value<uint8_t>()->default_value("32"))
    ("w,workers", "Worker threads", cxxopts::value<uint8_t>()->default_value("4"));

  auto args = options.parse(argc, argv);
  if (args.count("help")) {
    std::cout << options.help() << std::endl;
    exit(EXIT_SUCCESS);
  }

  auto socket_file = args["socket"].as<std::string>();
  auto socket_backlog = args["backlog"].as<uint8_t>();
  auto worker_threads = args["workers"].as<uint8_t>();

  if (worker_threads == 0) {
    std::cerr << "cannot run with zero worker threads" << std::endl;
    exit(EXIT_FAILURE);
  }

  // register signal handler for relevant signals
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = handle_signal;

  if (sigaction(SIGHUP, &sa, NULL) < 0) {
    std::cerr << "failed to install SIGHUP signal handler" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (sigaction(SIGTERM, &sa, NULL) < 0) {
    std::cerr << "failed to install SIGTERM signal handler"  << std::endl;
    exit(EXIT_FAILURE);
  }

  if (sigaction(SIGINT, &sa, NULL) < 0) {
    std::cerr << "failed to install SIGINT signal handler"  << std::endl;
    exit(EXIT_FAILURE);
  }

  // initialize the database
  std::shared_ptr<dictdb_t> db = std::make_shared<dictdb_t>();

  // create the server socket
  int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket == -1) {
    std::cerr << "failed to create socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  // bind the server socket
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_file.c_str(), sizeof(addr.sun_path) - 1);

  int rv = bind(server_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
  if (rv == -1) {
    std::cerr << "failed to bind socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  rv = listen(server_socket, socket_backlog);
  if (rv == -1) {
    std::cerr << "failed to listen on socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  // create a pool of worker threads
  auto client_sockets = std::make_shared<dictdb_socket_queue_t>();
  std::vector<std::shared_ptr<dictdb_worker_context_t>> workers;

  for (int i = 0; i < worker_threads; i++) {
    auto context = std::make_shared<dictdb_worker_context_t>();
    context->db = db;
    context->client_sockets = client_sockets;
    context->cancel = false;
    context->thread = std::make_shared<std::thread>(worker, context);

    workers.push_back(context);
  }

  // initialize poll data to monitor the server socket for new connections
  // https://www.man7.org/linux/man-pages/man2/poll.2.html
  struct pollfd server_poll;
  server_poll.fd = server_socket;
  server_poll.events = POLLIN;
  server_poll.revents = 0;

  // accept new client connections while the server is running
  running = true;
  while (running) {
    // poll for client connections
    server_poll.revents = 0;
    rv = poll(&server_poll, 1, 200);

    if ((server_poll.revents & POLLERR) != 0) {
      std::cerr << "server socket error" << std::endl;
      running = false;
      break;
    }

    if ((server_poll.revents & POLLIN) == 0) {
      continue;
    }

    // accept the connection and enqueue it for a worker thread to handle
    int client_socket = accept(server_socket, NULL, NULL);
    client_sockets->push(client_socket);
  }

  // stop worker threads
  for (auto context : workers) {
    context->cancel = true;
    context->thread->join();
  }

  // close and remaining open client sockets
  int client_socket;
  while (client_sockets->try_pop(client_socket)) {
    rv = close(client_socket);
    if (rv == -1) {
      std::cerr << "failed to close client socket" << std::endl;

      // don't exit to ensure proper cleanup
      // exit(EXIT_FAILURE);
    }
  }

  // close the server socket
  rv = close(server_socket);
  if (rv == -1) {
    std::cerr << "failed to close server socket" << std::endl;

    // don't exit to ensure we clean up the socket files
    // exit(EXIT_FAILURE);
  }

  // remove the socket file
  rv = remove(socket_file.c_str());
  if (rv == -1) {
    std::cerr << "failed to remove socket file" << std::endl;
    exit(EXIT_FAILURE);
  }

  // successful exit
  exit(EXIT_SUCCESS);
}
