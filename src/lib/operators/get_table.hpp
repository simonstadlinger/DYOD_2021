#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../storage/storage_manager.hpp"
#include "abstract_operator.hpp"

namespace opossum {

// operator to retrieve a table from the StorageManager by specifying its name
class GetTable : public AbstractOperator {
 public:
  explicit GetTable(std::string table_name) : _name(std::move(table_name)) {}

  [[nodiscard]] const std::string& table_name() const { return _name; }

 protected:
  std::shared_ptr<const Table> _on_execute() override { return StorageManager::get().get_table(_name); }

  const std::string _name;
};
}  // namespace opossum
