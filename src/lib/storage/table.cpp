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

Table::Table(const ChunkOffset target_chunk_size) : max_chunk_size{target_chunk_size} {
  chunks.push_back(std::make_shared<Chunk>());
}

void Table::add_column(const std::string& name, const std::string& type) {
  DebugAssert(row_count() == 0, "The chunk is not empty: no modification of the colyumn layout possible");
    throw std::exception();
  } else {
    col_names.push_back(name);
    col_types.push_back(type);

    for (auto chunk : chunks) {
      _add_segment_to_chunk(chunk, type);
    }
  }
}

void Table::_add_segment_to_chunk(std::shared_ptr<Chunk> chunk, std::string type) {
  resolve_data_type(type, [&](const auto data_type_t) {
    using ColumnDataType = typename decltype(data_type_t)::type;
    const auto value_segment = std::make_shared<ValueSegment<ColumnDataType>>();
    chunk->add_segment(value_segment);
  });
}

void Table::append(const std::vector<AllTypeVariant>& values) {
  if (chunks.back()->size() == max_chunk_size) {
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
    chunks.push_back(chunk);
    for (auto type : col_types) {
      _add_segment_to_chunk(chunk, type);
    }
  }
  chunks.back()->append(values);
}

ColumnCount Table::column_count() const {
  u_int16_t count = col_names.size();
  return ColumnCount{count};
}

uint64_t Table::row_count() const {
  if (chunks.size() == 0) return 0;
  int full_chunks_count = chunks.size() - 1;
  return full_chunks_count * max_chunk_size + chunks.back()->size();
}

ChunkID Table::chunk_count() const {
  uint16_t count = chunks.size();
  return ChunkID{count};
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto id = std::find(col_names.begin(), col_names.end(), column_name);
  if (id != col_names.end()) {
    int index = id - col_names.begin();
    return ColumnID(index);
  } else {
    throw std::runtime_error("no column with this name exists");
  }
}

ChunkOffset Table::target_chunk_size() const { return max_chunk_size; }

const std::vector<std::string>& Table::column_names() const { return col_names; }

const std::string& Table::column_name(const ColumnID column_id) const { return col_names[column_id]; }

const std::string& Table::column_type(const ColumnID column_id) const { return col_types.at(column_id); }

Chunk& Table::get_chunk(ChunkID chunk_id) { return *chunks[chunk_id]; }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { return *chunks[chunk_id]; }

void Table::print(std::ostream& out) const {
  int col_size = 20;
  for (auto name : col_names) {
    std::string value = name;
    if (static_cast<int>(value.length()) > col_size) value.resize(col_size);
    out << value << std::string(col_size - value.length(), ' ');
  }
  out << "\n" << std::string(col_size * column_count(), '-') << "\n";
  for (auto chunk : chunks) {
    chunk->print(col_size);
  }
}

}  // namespace opossum
