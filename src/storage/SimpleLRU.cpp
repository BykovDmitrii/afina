#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
  size_t _new_node_len = key.size() + value.size();
  if (_new_node_len > _max_size)
    return false;
   auto el = _lru_index.find(key);
   if (el != _lru_index.end())
     Delete(key);
   return this->PutIfAbsent(key, value);
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
  auto el = _lru_index.find(key);
  if (el != _lru_index.end())
    return false;
  size_t _new_node_len = key.size() + value.size();
  if (_new_node_len > _max_size)
    return false;
  while (_new_node_len + _curr_size > _max_size)
  {
    Delete(_lru_head->key);
  }
  _curr_size += _new_node_len;
  lru_node* _new_node(new lru_node());
  _new_node->key = key;
  _new_node->value = value;
  _new_node->next = nullptr;
  if(_lru_end != nullptr)
    _lru_end->next = _new_node;
  _new_node->prev = nullptr;
  _new_node->prev.swap(_lru_end);
  _lru_end.reset(_new_node);
  _lru_index.insert(map_node (_lru_end->key, *_lru_end));
  if(_lru_head == nullptr)
    _lru_head = _new_node;
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
  auto el = _lru_index.find(key);
  if (el == _lru_index.end())
    return false;
  Put(key, value);
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
  auto el = _lru_index.find(key);
  if (el == _lru_index.end())
    return false;
  lru_node &node = el->second;
  size_t _node_len = node.key.size() + node.value.size();
  _curr_size -= _node_len;
  if (node.prev != nullptr){
      node.prev->next = node.next;
    }
  if (node.next != nullptr)
    node.next->prev.swap(node.prev);
  else{
      _lru_end.swap(node.prev);
    }
  if(_lru_head->key == node.key)
    _lru_head = node.next;
  _lru_index.erase(el);
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value){
  auto el = _lru_index.find(key);
  if (el == _lru_index.end())
    return false;
  lru_node &node = el->second;
  value = node.value;
  Put(key, value);
  return true;
}

} // namespace Backend
} // namespace Afina
