#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "../../lib/resolve_type.hpp"
#include "../../lib/storage/base_segment.hpp"
#include "../../lib/storage/dictionary_segment.hpp"
#include "../../lib/storage/value_segment.hpp"

namespace opossum {

class StorageDictionarySegmentTest : public ::testing::Test {
 protected:
  std::shared_ptr<opossum::ValueSegment<int>> vc_int = std::make_shared<opossum::ValueSegment<int>>();
  std::shared_ptr<opossum::ValueSegment<std::string>> vc_str = std::make_shared<opossum::ValueSegment<std::string>>();
};

std::shared_ptr<DictionarySegment<std::string>> compressStringValueSegment(std::shared_ptr<opossum::ValueSegment<std::string>> value_segment) {
  std::shared_ptr<BaseSegment> col;
  resolve_data_type("string", [&](auto type) {
    using Type = typename decltype(type)::type;
    col = std::make_shared<DictionarySegment<Type>>(value_segment);
  });

  return std::dynamic_pointer_cast<DictionarySegment<std::string>>(col);
}

std::shared_ptr<DictionarySegment<int>> compressIntValueSegment(std::shared_ptr<opossum::ValueSegment<int>> value_segment) {
  std::shared_ptr<BaseSegment> col;
  resolve_data_type("int", [&](auto type) {
    using Type = typename decltype(type)::type;
    col = std::make_shared<DictionarySegment<Type>>(value_segment);
  });

  return std::dynamic_pointer_cast<DictionarySegment<int>>(col);
}


TEST_F(StorageDictionarySegmentTest, CompressSegmentString) {
  vc_str->append("Bill");
  vc_str->append("Steve");
  vc_str->append("Alexander");
  vc_str->append("Steve");
  vc_str->append("Hasso");
  vc_str->append("Bill");

  auto dict_col = compressStringValueSegment(vc_str);

  // Test attribute_vector size
  EXPECT_EQ(dict_col->size(), 6u);

  // Test dictionary size (uniqueness)
  EXPECT_EQ(dict_col->unique_values_count(), 4u);

  // Test sorting
  auto dict = dict_col->dictionary();
  EXPECT_EQ((*dict)[0], "Alexander");
  EXPECT_EQ((*dict)[1], "Bill");
  EXPECT_EQ((*dict)[2], "Hasso");
  EXPECT_EQ((*dict)[3], "Steve");
}

TEST_F(StorageDictionarySegmentTest, LowerUpperBound) {
  for (int i = 0; i <= 10; i += 2) vc_int->append(i);

  auto dict_col = compressIntValueSegment(vc_int);

  EXPECT_EQ(dict_col->lower_bound(4), (opossum::ValueID)2);
  EXPECT_EQ(dict_col->upper_bound(4), (opossum::ValueID)3);

  EXPECT_EQ(dict_col->lower_bound(5), (opossum::ValueID)3);
  EXPECT_EQ(dict_col->upper_bound(5), (opossum::ValueID)3);

  EXPECT_EQ(dict_col->lower_bound(15), opossum::INVALID_VALUE_ID);
  EXPECT_EQ(dict_col->upper_bound(15), opossum::INVALID_VALUE_ID);
}

TEST_F(StorageDictionarySegmentTest, ImpossibleAppend) {
  for (int i = 0; i <= 10; i += 2) vc_int->append(i);

  auto dict_col = compressIntValueSegment(vc_int);

  EXPECT_THROW(dict_col->append(11), std::runtime_error);
}

TEST_F(StorageDictionarySegmentTest, GetOperator) {
  vc_str->append("Bill");
  vc_str->append("Steve");
  vc_str->append("Alexander");
  vc_str->append("Steve");

  auto dict_col = compressStringValueSegment(vc_str);

  EXPECT_EQ((*dict_col).get(0), "Bill");
  EXPECT_EQ((*dict_col).get(1), "Steve");
  EXPECT_EQ((*dict_col).get(2), "Alexander");
  EXPECT_EQ((*dict_col).get(3), "Steve");

}

TEST_F(StorageDictionarySegmentTest, ValueByValueID) {
  // vc_str->append("Bill");
  // vc_str->append("Steve");
  // vc_str->append("Alexander");
  // vc_str->append("Steve");

  // auto dict_col = compressStringValueSegment(vc_str);

  for (int i = 0; i <= 10; i += 2) vc_int->append(i);

  auto dict_col = compressIntValueSegment(vc_int);


  auto actualValue = dict_col->value_by_value_id(dict_col->lower_bound(2));
  EXPECT_EQ(2, actualValue );
}

// TODO(student): You should add some more tests here (full coverage would be appreciated) and possibly in other files.

}  // namespace opossum
