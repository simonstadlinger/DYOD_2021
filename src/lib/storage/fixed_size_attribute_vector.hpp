#pragma once

#include <vector>

#include "base_attribute_vector.hpp"
#include "type_cast.hpp"

namespace opossum {

template <typename T>
class FixedSizeAttributeVector : public BaseAttributeVector {
 public:
  explicit FixedSizeAttributeVector(size_t attribute_vector_size) {
    _attributes = std::vector<T>(attribute_vector_size);
  }

  ~FixedSizeAttributeVector() = default;

  // we need to explicitly set the move constructor to default when
  // we overwrite the copy constructor
  FixedSizeAttributeVector(FixedSizeAttributeVector&&) = default;
  FixedSizeAttributeVector& operator=(FixedSizeAttributeVector&&) = default;

  // returns the value id at a given position
  ValueID get(const size_t i) const { return static_cast<ValueID>(_attributes.at(i)); }

  // sets the value id at a given position
  void set(const size_t index, const ValueID value_id) { _attributes.at(index) = static_cast<T>(value_id); }

  // returns the number of values
  size_t size() const { return _attributes.size(); }

  // returns the width of biggest value id in bytes
  AttributeVectorWidth width() const { return static_cast<AttributeVectorWidth>(sizeof(T)); }

 private:
  std::vector<T> _attributes = {};
};
}  // namespace opossum
