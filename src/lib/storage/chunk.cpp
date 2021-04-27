#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "base_segment.hpp"
#include "chunk.hpp"

#include "utils/assert.hpp"

namespace opossum {

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment) { segments.push_back(segment); }

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  DebugAssert(!values.empty(), "");
  DebugAssert(values.size() == segments.size(), "");

  int size = segments.size();
  for (int i = 0; i < size; i++) {
    segments[i]->append(values[i]);
  }
}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const { return segments[column_id]; }

ColumnCount Chunk::column_count() const {
  uint16_t count = segments.size();
  return ColumnCount{count};
}

ChunkOffset Chunk::size() const {
  if (segments.empty())
    return 0;
  else
    return segments[0]->size();
}

void Chunk::print(int col_size, std::ostream& out) const {
  for (ChunkOffset i = 0; i < size(); i++) {
    for (auto segment : segments) {
      std::string line;
      AllTypeVariant value = (*segment)[i];
      std::stringstream ss;
      ss << value;
      ss >> line;
      out << line << std::string(col_size - line.length(), ' ');
    }
    out << "\n";
  }
}

}  // namespace opossum
