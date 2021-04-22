#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "base_segment.hpp"
#include "value_segment.hpp"
#include "chunk.hpp"

#include "utils/assert.hpp"

namespace opossum {

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment) {
  segments.push_back(segment);
}

void Chunk::append(const std::vector<AllTypeVariant>& values) {
   DebugAssert(!values.empty(), "");
   DebugAssert(values.size() == segments.size(), "");

  if(values.size() != segments.size()) throw std::exception();

  int s = segments.size();
  for(int i = 0; i<s; i++) {
    AllTypeVariant val = values[i];
    segments[i]->append(val);
  }
}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const {
  // Implementation goes here
  return segments[column_id];
}

ColumnCount Chunk::column_count() const {
  int count = segments.size();
  return ColumnCount{count};
}

ChunkOffset Chunk::size() const {
  if (segments.empty()) return 0;
  else return segments[0]->size();
}

}  // namespace opossum
