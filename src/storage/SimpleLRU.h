#ifndef AFINA_STORAGE_SIMPLE_LRU_H
#define AFINA_STORAGE_SIMPLE_LRU_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <algorithm>

#include <afina/Storage.h>

namespace Afina {
namespace Backend {

/**
 * # Map based implementation
 * That is NOT thread safe implementaiton!!
 */
class SimpleLRU : public Afina::Storage {
public:
    SimpleLRU(size_t max_size = 1024) : _max_size(max_size), _curr_size(0) {}

    ~SimpleLRU() {
        _lru_index.clear();
        _lru_head.reset(); // TODO: Here is stack overflow
    }

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) const override;

private:
    // LRU cache node
    using lru_node = struct lru_node {
        std::string key;
        std::string value;
        std::unique_ptr<lru_node> prev;
        lru_node* next;
        size_t using_count = 1;
    };
  using map_node = std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>>;

static bool rare(const map_node &i, const map_node &j){
  lru_node &a = (i.second);
  lru_node &b =  (j.second);
  return a.using_count < b.using_count;
}

    mutable size_t iter = 1;
    // Maximum number of bytes could be stored in this cache.
    // i.e all (keys+values) must be less the _max_size
    size_t _max_size;

    size_t _curr_size; //current size

    // Main storage of lru_nodes, elements in this list ordered descending by "freshness": in the head
    // element that wasn't used for longest time.
    //
    // List owns all nodess
    std::unique_ptr<lru_node> _lru_head = nullptr;

    //std::map<std::string, std::pair<std::string, uint>> _lru_cashe;
    // Index of nodes from list above, allows fast random access to elements by lru_node#key
    std::map<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>, std::less<std::string>> _lru_index;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_SIMPLE_LRU_H
