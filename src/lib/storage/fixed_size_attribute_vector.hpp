#include "base_attribute_vector.hpp"

namespace opossum {


//FixedSizeAttributeVector
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
      _attributes.at(i) = (T) value_id;
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
