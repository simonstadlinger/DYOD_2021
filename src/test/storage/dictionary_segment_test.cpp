#include <memory>
#include <string>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "resolve_type.hpp"
#include "storage/base_segment.hpp"
#include "storage/dictionary_segment.hpp"
#include "storage/value_segment.hpp"

namespace opossum {

// class StorageDictionarySegmentTest : public BaseTest {
//  protected:
//   std::shared_ptr<ValueSegment<int>> vc_int = std::make_shared<ValueSegment<int>>();
//   std::shared_ptr<ValueSegment<std::string>> vc_str = std::make_shared<ValueSegment<std::string>>();
// };

// TEST_F(StorageDictionarySegmentTest, CompressSegmentString) {
//   vc_str->append("Bill");
//   vc_str->append("Steve");
//   vc_str->append("Alexander");
//   vc_str->append("Steve");
//   vc_str->append("Hasso");
//   vc_str->append("Bill");

//   std::shared_ptr<BaseSegment> col;
//   resolve_data_type("string", [&](auto type) {
//     using Type = typename decltype(type)::type;
//     col = std::make_shared<DictionarySegment<Type>>(vc_str);
//   });

//   auto dict_col = std::dynamic_pointer_cast<DictionarySegment<std::string>>(col);

//   // Test attribute_vector size
//   EXPECT_EQ(dict_col->size(), 6u);

//   // Test dictionary size (uniqueness)
//   EXPECT_EQ(dict_col->unique_values_count(), 4u);

//   // Test sorting
//   auto dict = dict_col->dictionary();
//   EXPECT_EQ((*dict)[0], "Alexander");
//   EXPECT_EQ((*dict)[1], "Bill");
//   EXPECT_EQ((*dict)[2], "Hasso");
//   EXPECT_EQ((*dict)[3], "Steve");
// }

// TEST_F(StorageDictionarySegmentTest, LowerUpperBound) {
//   for (int i = 0; i <= 10; i += 2) vc_int->append(i);

//   std::shared_ptr<BaseSegment> col;
//   resolve_data_type("int", [&](auto type) {
//     using Type = typename decltype(type)::type;
//     col = std::make_shared<DictionarySegment<Type>>(vc_int);
//   });
//   auto dict_col = std::dynamic_pointer_cast<DictionarySegment<int>>(col);

//   EXPECT_EQ(dict_col->lower_bound(4), (ValueID)2);
//   EXPECT_EQ(dict_col->upper_bound(4), (ValueID)3);

//   EXPECT_EQ(dict_col->lower_bound(5), (ValueID)3);
//   EXPECT_EQ(dict_col->upper_bound(5), (ValueID)3);

//   EXPECT_EQ(dict_col->lower_bound(15), INVALID_VALUE_ID);
//   EXPECT_EQ(dict_col->upper_bound(15), INVALID_VALUE_ID);
// }

// TODO(student): You should add some more tests here (full coverage would be appreciated) and possibly in other files.

}  // namespace opossum
