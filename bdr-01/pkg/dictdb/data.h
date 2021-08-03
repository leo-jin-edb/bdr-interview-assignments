#pragma once

// Intel TBB
#include <oneapi/tbb/concurrent_hash_map.h>


/**
 * Words hash map.
 *
 * The words are stored as keys and the values don't matter.
 * @see https://spec.oneapi.io/versions/latest/elements/oneTBB/source/containers/concurrent_hash_map_cls.html
 */
typedef tbb::concurrent_hash_map<std::string, bool> dictdb_word_map_t;

/**
 * Dictionary in-memory database.
 */
typedef struct {
  dictdb_word_map_t words;
} dictdb_t;
