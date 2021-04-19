#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base_segment.hpp"

namespace opossum {

// ValueSegment is a segment type that stores all its values in a vector
template <typename T>
class ValueSegment : public BaseSegment {
 public:
  // return the value at a certain position. If you want to write efficient operators, back off!
  AllTypeVariant operator[](const ChunkOffset chunk_offset) const final;

  // add a value to the end
  void append(const AllTypeVariant& val) final;

  // return the number of entries
  ChunkOffset size() const final;

  // Return all values. This is the preferred method to check a value at a certain index. Usually you need to
  // access more than a single value anyway.
  // e.g. const auto& values = value_segment.values(); and then: values[i]; in your loop.
  const std::vector<T>& values() const;

 protected:
  // Implementation goes here
  std::vector<T> raw_values;
};

}  // namespace opossum
