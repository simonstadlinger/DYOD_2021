#pragma once

#include <vector>

#include "base_attribute_vector.hpp"
#include "type_cast.hpp"

namespace opossum {

template <typename T>
class FixedSizeAttributeVector : public BaseAttributeVector {
 public:
  explicit FixedSizeAttributeVector(size_t attribute_vector_size) { _attributes.reserve(attribute_vector_size); }

  ~FixedSizeAttributeVector() = default;

  // we need to explicitly set the move constructor to default when
  // we overwrite the copy constructor
  FixedSizeAttributeVector(FixedSizeAttributeVector&&) = default;
  FixedSizeAttributeVector& operator=(FixedSizeAttributeVector&&) = default;

  // returns the value id at a given position
  ValueID get(const size_t i) const { return static_cast<ValueID>(_attributes.at(i)); }

  // sets the value id at a given position
  void set(const size_t index, const ValueID value_id) {
    // [] operator does somehow not change the size of the vector
    // according to this stackoverflow answer it's considered bad practice to just use the [] operator
    // https://stackoverflow.com/questions/31372809/c-vector-size-is-returning-zero
    // nevertheless, this does not feel right.
     if (_attributes.begin() + index < _attributes.end()) {
      _attributes.at(index) = (T)value_id;
    } else if (_attributes.begin() + index == _attributes.end()){
      _attributes.push_back((T) value_id);
    } else {
      throw std::runtime_error("Out of Bounds"); 
    }
  }

  // returns the number of values
  size_t size() const { return _attributes.size(); }

  // returns the width of biggest value id in bytes
  AttributeVectorWidth width() const { return static_cast<AttributeVectorWidth>(sizeof(T)); }

 private:
  std::vector<T> _attributes = {};
};
}  // namespace opossum
