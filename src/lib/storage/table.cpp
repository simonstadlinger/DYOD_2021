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

Table::Table(const ChunkOffset target_chunk_size) : _max_chunk_size{target_chunk_size} {
  _chunks.push_back(std::make_shared<Chunk>());
}

void Table::add_column(const std::string& name, const std::string& type) {
  Assert(row_count() == 0, "The chunk is not empty: no modification of the column layout possible");
  _col_names.push_back(name);
  _col_types.push_back(type);

  for (auto chunk : _chunks) {
    _add_segment_to_chunk(chunk, type);
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
  if (_chunks.back()->size() == _max_chunk_size) {
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
    _chunks.push_back(chunk);
    for (auto type : _col_types) {
      _add_segment_to_chunk(chunk, type);
    }
  }
  _chunks.back()->append(values);
}

ColumnCount Table::column_count() const {
  u_int16_t count = _col_names.size();
  return ColumnCount{count};
}

uint64_t Table::row_count() const {
  if (_chunks.size() == 0) return 0;
  int full_chunks_count = _chunks.size() - 1;
  return full_chunks_count * _max_chunk_size + _chunks.back()->size();
}

ChunkID Table::chunk_count() const {
  uint16_t count = _chunks.size();
  return ChunkID{count};
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto id = std::find(_col_names.begin(), _col_names.end(), column_name);
  Assert(id != _col_names.end(), "No column with this name exists");
  int index = std::distance(_col_names.begin(), id);
  return ColumnID(index);
}

ChunkOffset Table::target_chunk_size() const { return _max_chunk_size; }

const std::vector<std::string>& Table::column_names() const { return _col_names; }

const std::string& Table::column_name(const ColumnID column_id) const { return _col_names.at(column_id); }

const std::string& Table::column_type(const ColumnID column_id) const { return _col_types.at(column_id); }

Chunk& Table::get_chunk(ChunkID chunk_id) { return *_chunks.at(chunk_id); }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { return *_chunks.at(chunk_id); }

void Table::print(std::ostream& out) const {
  int col_width = 20;
  for (auto name : _col_names) {
    std::string value = name;
    if (static_cast<int>(value.length()) > col_width) value.resize(col_width);
    out << value << std::string(col_width - value.length(), ' ');
  }
  out << "\n" << std::string(col_width * column_count(), '-') << "\n";
  for (auto chunk : _chunks) {
    chunk->print(col_width);
  }
}

}  // namespace opossum
