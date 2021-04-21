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
  this->chunks.push_back(std::make_shared<Chunk>());
}

void Table::add_column(const std::string& name, const std::string& type) {
  this->col_names.push_back(name);
  this->col_types.push_back(type);
  // Add Segments to each chunk
  int size = this->chunks.size();
  for(int i = 0; i < size; i++) {
    this->_add_segment_to_chunk(this->chunks.at(i), this->col_names.size() - 1);
  }
  this->n_cols++;
}

void Table::_add_segment_to_chunk(std::shared_ptr<Chunk> chunk, int seg_index) {
  resolve_data_type(this->col_types.at(seg_index), [&](const auto data_type_t) {
           using ColumnDataType = typename
               decltype(data_type_t)::type;
           const auto value_segment =
               std::make_shared<ValueSegment<ColumnDataType>>();
            chunk->add_segment(value_segment);
  });
}

void Table::_add_segments_to_chunk(std::shared_ptr<Chunk> chunk) {
  int s = this->col_names.size();
  for(int i = 0; i < s; i++) {
    this->_add_segment_to_chunk(chunk, i);
  }
}

void Table::append(const std::vector<AllTypeVariant>& values) {
  if(this->chunks.back()->size() == this->max_chunk_size) {
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
    this->chunks.push_back(chunk);
    this->_add_segments_to_chunk(chunk);
  }
  this->chunks.back()->append(values);
}

ColumnCount Table::column_count() const {
  int c = this->n_cols;
  return ColumnCount{c};
}

uint64_t Table::row_count() const {
  if(this->chunks.size() == 0) return 0;
  int c = (this->chunks.size() - 1) * this->max_chunk_size + this->chunks.back()->size();
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
  return this->col_names;
}

const std::string& Table::column_name(const ColumnID column_id) const {
  return this->col_names.at(column_id);
}

const std::string& Table::column_type(const ColumnID column_id) const {
  return this->col_types.at(column_id);
}

Chunk& Table::get_chunk(ChunkID chunk_id) {
  return *(this->chunks.at(chunk_id));
}

const Chunk& Table::get_chunk(ChunkID chunk_id) const {
  return *(this->chunks.at(chunk_id));
}

}  // namespace opossum
