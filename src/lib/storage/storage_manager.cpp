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

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) { tables[name] = table; }

void StorageManager::drop_table(const std::string& name) {
  if (tables.at(name) != NULL) {
    tables.erase(name);
  } else {
    throw std::runtime_error("no such table");
  }
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  if (tables.at(name) != NULL) return tables.at(name);
  return nullptr;
}

bool StorageManager::has_table(const std::string& name) const { return tables.contains(name); }

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> names;
  for (auto it = tables.begin(); it != tables.end(); ++it) {
    names.push_back(it->first);
  }
  return names;
}

void StorageManager::print(std::ostream& out) const {
  for (auto it = tables.begin(); it != tables.end(); ++it) {
    it->second->print();
  }
}

void StorageManager::reset() { tables.clear(); }

}  // namespace opossum
