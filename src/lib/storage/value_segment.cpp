#include "value_segment.hpp"

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "type_cast.hpp"
#include "utils/assert.hpp"

namespace opossum {

template <typename T>
AllTypeVariant ValueSegment<T>::operator[](const ChunkOffset chunk_offset) const {
  throw std::runtime_error("Implement ValueSegment::operator[]");
}

template <typename T>
void ValueSegment<T>::append(const AllTypeVariant& val) {
  // Implementation goes here
}

template <typename T>
ChunkOffset ValueSegment<T>::size() const {
  // Implementation goes here
  return 0;
}

EXPLICITLY_INSTANTIATE_DATA_TYPES(ValueSegment);

}  // namespace opossum
