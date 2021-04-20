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
  this->segments.push_back(segment);
}

void Chunk::append(const std::vector<AllTypeVariant>& values) {
   DebugAssert(!values.empty(), "");
   DebugAssert(values.size() == this->segments.size(), "");

  if(values.size() != this->segments.size()) throw std::exception();

  int s = this->segments.size();
  for(int i = 0; i<s; i++) {
    AllTypeVariant val = values.at(i);
    this->segments.at(i).get()->append(val);
  }
}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const {
  // Implementation goes here
  return this->segments.at(column_id);
}

ColumnCount Chunk::column_count() const {
  int count = this->segments.size();
  return ColumnCount{count};
}

ChunkOffset Chunk::size() const {
  if (this->segments.empty()) return 0;
  else return this->segments.at(0)->size();
}

}  // namespace opossum
