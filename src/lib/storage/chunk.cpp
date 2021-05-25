#include <iomanip>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "base_segment.hpp"
#include "chunk.hpp"

#include "utils/assert.hpp"

namespace opossum {

Chunk::Chunk(size_t column_count) {
  _segments = std::vector<std::shared_ptr<BaseSegment>>(column_count);
}

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment) {
  std::lock_guard<std::mutex> lock(_add_segment_lock);
  _segments.push_back(segment);
}

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment, ColumnID col_id) {
    std::lock_guard<std::mutex> lock(_add_segment_lock);
    _segments[col_id] = segment;
}

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  Assert(values.size() == _segments.size(), "Invalid number of columns to be inserted");

  int size = _segments.size();
  for (int column_index = 0; column_index < size; column_index++) {
    _segments[column_index]->append(values[column_index]);
  }
}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const { return _segments.at(column_id); }

ColumnCount Chunk::column_count() const {
  uint16_t count = _segments.size();
  return ColumnCount{count};
}

ChunkOffset Chunk::size() const { return _segments.empty() ? 0 : _segments[0]->size(); }

void Chunk::print(int col_size, std::ostream& out) const {
  for (ChunkOffset i = 0; i < size(); i++) {
    for (auto segment : _segments) {
      std::string line;
      AllTypeVariant value = (*segment)[i];
      std::stringstream ss;
      ss << value;
      ss >> line;  // convert value to string regardless of type
      if ((static_cast<int>(line.length())) > col_size) line.resize(col_size);
      out << line << std::string(col_size - line.length(), ' ');
    }
    out << "\n";
  }
}

}  // namespace opossum
