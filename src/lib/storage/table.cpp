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
  max_chunk_size = target_chunk_size;
  n_cols = 0;
  chunks.push_back(std::make_shared<Chunk>());
}

void Table::add_column(const std::string& name, const std::string& type) {
  col_names.push_back(name);
  col_types.push_back(type);
  // Add Segments to each chunk
  for(auto chunk : chunks) {
    _add_segment_to_chunk(chunk, col_names.size() - 1);
  }
  n_cols++;
}

void Table::_add_segment_to_chunk(std::shared_ptr<Chunk> chunk, int seg_index) {
  resolve_data_type(col_types[seg_index], [&](const auto data_type_t) {
           using ColumnDataType = typename
               decltype(data_type_t)::type;
           const auto value_segment =
               std::make_shared<ValueSegment<ColumnDataType>>();
            chunk->add_segment(value_segment);
  });
}

void Table::_add_segments_to_chunk(std::shared_ptr<Chunk> chunk) {
  int s = col_names.size();
  for(int i = 0; i < s; i++) {
    _add_segment_to_chunk(chunk, i);
  }
}

void Table::append(const std::vector<AllTypeVariant>& values) {
  if(chunks.back()->size() == max_chunk_size) {
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
    chunks.push_back(chunk);
    _add_segments_to_chunk(chunk);
  }
  chunks.back()->append(values);
}

ColumnCount Table::column_count() const {
  return ColumnCount{n_cols};
}

uint64_t Table::row_count() const {
  if(chunks.size() == 0) return 0;
  int full_chunks_count = chunks.size() - 1;
  return full_chunks_count * max_chunk_size + chunks.back()->size();
}

ChunkID Table::chunk_count() const {
  return ChunkID{chunks.size()};
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto id = std::find(col_names.begin(), col_names.end(), column_name);
  if (id != col_names.end())
    {
        int index = id - col_names.begin();
        return ColumnID{index};
    }
  else {
    throw std::runtime_error("No column with such a name");
  }
  return ColumnID{0};
}

ChunkOffset Table::target_chunk_size() const {
  return max_chunk_size;
}

const std::vector<std::string>& Table::column_names() const {
  return col_names;
}

const std::string& Table::column_name(const ColumnID column_id) const {
  return col_names[column_id];
}

const std::string& Table::column_type(const ColumnID column_id) const {
  return col_types[column_id];
}

Chunk& Table::get_chunk(ChunkID chunk_id) {
  return *chunks[chunk_id];
}

const Chunk& Table::get_chunk(ChunkID chunk_id) const {
  return *(chunks[chunk_id]);
}

}  // namespace opossum
