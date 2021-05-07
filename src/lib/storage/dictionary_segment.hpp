#pragma once

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "all_type_variant.hpp"
#include "types.hpp"

namespace opossum {

class BaseAttributeVector;
class BaseSegment;

// Even though ValueIDs do not have to use the full width of ValueID (uint32_t), this will also work for smaller ValueID
// types (uint8_t, uint16_t) since after a down-cast INVALID_VALUE_ID will look like their numeric_limit::max()
constexpr ValueID INVALID_VALUE_ID{std::numeric_limits<ValueID::base_type>::max()};

// Dictionary is a specific segment type that stores all its values in a vector
template <typename T>
class DictionarySegment : public BaseSegment {
 public:
  /**
   * Creates a Dictionary segment from a given value segment.
   */
  explicit DictionarySegment(const std::shared_ptr<BaseSegment>& baseSegment) {
    auto valueSegment = std::static_pointer_cast<ValueSegment<T>>(baseSegment);

    _build_compressed_dictionary(valueSegment->values());
  }

  // SEMINAR INFORMATION: Since most of these methods depend on the template parameter, you will have to implement
  // the DictionarySegment in this file. Replace the method signatures with actual implementations.

  // return the value at a certain position. If you want to write efficient operators, back off!
  AllTypeVariant operator[](const ChunkOffset chunk_offset) const {
    return static_cast<AllTypeVariant>(get(chunk_offset));
  };

  // return the value at a certain position.
  T get(const size_t chunk_offset) const {
      return value_by_id(_attribute_vector->at(chunk_offset));
  };

  // dictionary segments are immutable
  void append(const AllTypeVariant& val){
      throw std::runtime_error("Dictionary segments are immutable. You shall not append anything.");
  };

  // returns an underlying dictionary
  std::shared_ptr<const std::vector<T>> dictionary() {
      return _dictionary;
  };

  // returns an underlying data structure
  std::shared_ptr<std::vector<uint32_t>> attribute_vector() const {
      return _attribute_vector;
  };

  // return the value represented by a given ValueID
  const T& value_by_value_id(ValueID value_id) const {
      return _dictionary->at(value_id);
  };

  // returns the first value ID that refers to a value >= the search value
  // returns INVALID_VALUE_ID if all values are smaller than the search value
  ValueID lower_bound(T value) const {
    int size = _dictionary->size();
    for(ValueID dictionary_index = ValueID{0}; dictionary_index < size; ++dictionary_index ) {
      if(value_by_id(dictionary_index) >= value) {
        return dictionary_index;
      }
    }

    return INVALID_VALUE_ID;
  };

  // same as lower_bound(T), but accepts an AllTypeVariant
  ValueID lower_bound(const AllTypeVariant& value) const {
      return lower_bound(static_cast<T>(value));
  };

  // returns the first value ID that refers to a value > the search value
  // returns INVALID_VALUE_ID if all values are smaller than or equal to the search value
  ValueID upper_bound(T value) const {
    for(ValueID dictionary_index = ValueID{0} ; dictionary_index < _dictionary->size(); ++dictionary_index ) {
      if(value_by_id(dictionary_index) > value) {
        return dictionary_index;
      }
    }

    return INVALID_VALUE_ID;
  };

  // same as upper_bound(T), but accepts an AllTypeVariant
  ValueID upper_bound(const AllTypeVariant& value) const {
      return upper_bound(static_cast<T>(value));
  };

  // return the number of _dictionary (dictionary entries)
  size_t unique_values_count() const {
      return _dictionary->size();
  };

  // return the number of entries
  ChunkOffset size() const {
      return _attribute_vector->size();
  };

  // returns the calculated memory usage
  size_t estimate_memory_usage() const final {
      return 0;
  };

 protected:
  std::shared_ptr<std::vector<T>> _dictionary;
  std::shared_ptr<std::vector<uint32_t>> _attribute_vector;

  void _build_compressed_dictionary(const std::vector<T>& values) {
    auto raw_dictionary = std::move(values);
    std::vector<T> raw_values = raw_dictionary;

    std::sort(raw_dictionary.begin(), raw_dictionary.end());
    raw_dictionary.erase(std::unique(raw_dictionary.begin(), raw_dictionary.end()), raw_dictionary.end());

    _dictionary = std::make_shared<std::vector<T>>(raw_dictionary);

    _build_attribute_vector(raw_dictionary, raw_values);
  }

  void _build_attribute_vector(std::vector<T>& raw_dictionary, std::vector<T>& all_values) {
    std::vector<uint32_t> raw_vector{};
    raw_vector.reserve(all_values.size());

    for(auto value: all_values) {
      auto dictionary_iterator = std::lower_bound(raw_dictionary.begin(), raw_dictionary.end(), value);
      uint32_t dictionary_index = dictionary_iterator - raw_dictionary.begin();
      raw_vector.push_back(dictionary_index);
    }

    _attribute_vector = std::make_shared<std::vector<uint32_t>>(raw_vector);
  }
};

}  // namespace opossum
