#include "base_attribute_vector.hpp"

namespace opossum {

template <typename T>
class FixedSizeAttributeVector : public BaseAttributeVector {
  public:
    FixedSizeAttributeVector(size_t attribute_vector_size) {
      _attributes.reserve(attribute_vector_size);
    }

    ~FixedSizeAttributeVector() = default;

    // we need to explicitly set the move constructor to default when
    // we overwrite the copy constructor
    FixedSizeAttributeVector(FixedSizeAttributeVector&&) = default;
    FixedSizeAttributeVector& operator=(FixedSizeAttributeVector&&) = default;

    // returns the value id at a given position
    ValueID get(const size_t i) const {
      return static_cast<ValueID>(_attributes.at(i));
    }

    // sets the value id at a given position
    void set(const size_t i, const ValueID value_id) {
      // [] operator does somehow not change the size of the vector
      // according to this stackoverflow answer it's considered bad stil to just use the [] operator
      // https://stackoverflow.com/questions/31372809/c-vector-size-is-returning-zero
      // nevertheless, this does not feel right.
      _attributes.push_back((T) value_id);
      _attributes[i] = _attributes.back();
    }

    // returns the number of values
    size_t size() const {
      return _attributes.size();
    }

    // returns the width of biggest value id in bytes
    AttributeVectorWidth width() const {
      return static_cast<AttributeVectorWidth>(sizeof(T));
    };

  private:
    std::vector<T> _attributes = {};
};
}  // namespace opossum
