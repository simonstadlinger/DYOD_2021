#include "table.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "value_segment.hpp"

#include "resolve_type.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

Table::Table(const ChunkOffset target_chunk_size) {
  this->max_chunk_size = target_chunk_size;
  this->n_cols = 0;
}

void Table::add_column(const std::string& name, const std::string& type) {
  this->col_names.push_back(name);
  this->col_types.push_back(type);
  // Add a Segment for each Chunk
  int size = this->chunks.size();
  for(int i = 0; i < size; i++) {
    resolve_data_type(type, [&](auto type) {
      using ColumnDataType = typename decltype(type)::type;
      this->chunks.at(i).add_segment(std::make_shared<ValueSegment<ColumnDataType>>());
    });
  }
  this->n_cols++;
}

void Table::append(const std::vector<AllTypeVariant>& values) {
  
}

ColumnCount Table::column_count() const {
  int c = this->n_cols;
  return ColumnCount{c};
}

uint64_t Table::row_count() const {
  if(this->chunks.size() == 0) return 0;
  int c = (this->chunks.size() - 1) * this->max_chunk_size + this->chunks.back().size();
  return c;
}

ChunkID Table::chunk_count() const {
  int c = this->chunks.size();
  return ChunkID{c};
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto id = std::find(this->col_names.begin(), this->col_names.end(), column_name);
  if (id != this->col_names.end())
    {
        int index = id - this->col_names.begin();
        return ColumnID{index};
    }
  else {
    throw std::runtime_error("No column with such a name");
  }
  return ColumnID{0};
}

ChunkOffset Table::target_chunk_size() const {
  return this->max_chunk_size;
}

const std::vector<std::string>& Table::column_names() const {
  throw std::runtime_error("Implement Table::column_names()");
}

const std::string& Table::column_name(const ColumnID column_id) const {
  throw std::runtime_error("Implement Table::column_name");
}

const std::string& Table::column_type(const ColumnID column_id) const {
  throw std::runtime_error("Implement Table::column_type");
}

Chunk& Table::get_chunk(ChunkID chunk_id) { throw std::runtime_error("Implement Table::get_chunk"); }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { throw std::runtime_error("Implement Table::get_chunk"); }

}  // namespace opossum
