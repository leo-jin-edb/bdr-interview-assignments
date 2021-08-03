#pragma once

// C++ Standard Library
#include <memory>

// Intel TBB
#include <oneapi/tbb/concurrent_queue.h>

// DictDB
#include "data.h"


/**
 * An unbounded lock-free concurrent queue for storing client socket fds.
 *
 * @see https://spec.oneapi.io/versions/latest/elements/oneTBB/source/containers/concurrent_queue_cls.html
 * @see https://software.intel.com/content/www/us/en/develop/documentation/onetbb-documentation/top/onetbb-developer-guide/containers/concurrent-queue-classes.html
 */
typedef tbb::concurrent_queue<int> dictdb_socket_queue_t;

/**
 * Worker thread context.
 */
typedef struct {
  std::shared_ptr<std::thread> thread;
  std::shared_ptr<dictdb_t> db;
  std::shared_ptr<dictdb_socket_queue_t> client_sockets;
  bool cancel;
} dictdb_worker_context_t;

/**
 * Worker thread entry point.
 */
void worker(std::shared_ptr<dictdb_worker_context_t> context);
