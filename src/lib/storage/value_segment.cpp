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
  if (chunk_offset >= this->size()) {
    throw std::runtime_error("Vector out of bounds");
  }
    return valuesVector[chunk_offset];
}

template <typename T>
void ValueSegment<T>::append(const AllTypeVariant& val) {
  return valuesVector.push_back(type_cast<T> (val));
}

template <typename T>
ChunkOffset ValueSegment<T>::size() const {
  return valuesVector.size();
}

EXPLICITLY_INSTANTIATE_DATA_TYPES(ValueSegment);

}  // namespace opossum
