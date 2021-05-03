#include "storage_manager.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "utils/assert.hpp"

namespace opossum {

StorageManager& StorageManager::get() {
  static StorageManager sm;
  return sm;
}

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) {
  DebugAssert(!_tables.contains(name), "A table with provided name already exists");
  _tables[name] = table;  
}

void StorageManager::drop_table(const std::string& name) {
  const auto dropped_table_count = _tables.erase(name);
  Assert(dropped_table_count == 1, "table could not be removed");
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  if (_tables.contains(name)){
    return _tables.at(name);
  }
  throw std::runtime_error("No table with the provided name");
  return nullptr;
}

bool StorageManager::has_table(const std::string& name) const { return _tables.contains(name); }

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> names;
  for (auto it = _tables.begin(); it != _tables.end(); ++it) {
    names.push_back(it->first);
  }
  return names;
}

void StorageManager::print(std::ostream& out) const {
  for (auto it = _tables.begin(); it != _tables.end(); ++it) {
    it->second->print();
  }
}

void StorageManager::reset() { _tables.clear(); }

}  // namespace opossum
