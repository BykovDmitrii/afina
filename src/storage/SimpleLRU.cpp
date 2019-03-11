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
     return Set(el, key, value);
   return PutNewElem(key, value);
}


// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
  auto el = _lru_index.find(key);
   if (el != _lru_index.end())
     return false;
  return PutNewElem(key, value);
}


bool SimpleLRU::PutNewElem(const std::string &key, const std::string &value)
{
  size_t _new_node_len = key.size() + value.size();
  if (_new_node_len > _max_size)
    return false;
  while (_new_node_len + _curr_size > _max_size)
  {
    Delete(_lru_head->key);
  }
  _curr_size += _new_node_len;
  lru_node*_new_node = new lru_node(key, value);
  if(_lru_end != nullptr)
    _lru_end->next = _new_node;
  _new_node->prev.swap(_lru_end);
  _lru_end.reset(_new_node);
  _lru_index.insert(std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<lru_node>>(_lru_end->key, *_lru_end));
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
  return this->Set(el, key, value);
}

bool SimpleLRU::Set(map_iterator el, const std::string &key, const std::string &value)
{
  if(key.size() + value.size() > _max_size)
    return false;
  MoveToEnd(el);
  lru_node &node = el->second;
  size_t _size_delta = value.size() - node.value.size();
  while((_max_size < _curr_size + _size_delta))
    {
      Delete(_lru_head->key);
    }
  _max_size = _curr_size + _size_delta;
  node.value = value;
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
  auto el = _lru_index.find(key);
  if (el == _lru_index.end())
    return false;
  DeleteElem(el);
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value){
  auto el = _lru_index.find(key);
  if (el == _lru_index.end())
    return false;
  MoveToEnd(el);
  value = _lru_end->value;
  return true;
}

void SimpleLRU::DeleteElem(map_iterator el){
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
}

void SimpleLRU::MoveToEnd(map_iterator el){
  lru_node &node = el->second;
  if(node.next == nullptr)
    return;
  if(_lru_head == &node){
      _lru_head = node.next;
   }
  else{
      node.prev->next = node.next;
    }
  node.next->prev.swap(node.prev);
  node.next = nullptr;
  _lru_end->next = &node;
  node.prev.swap(_lru_end);
}
} // namespace Backend
} // namespace Afina
