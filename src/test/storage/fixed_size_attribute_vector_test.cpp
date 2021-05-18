#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "../../lib/storage/fixed_size_attribute_vector.hpp"
#include "../../lib/storage/value_segment.hpp"
#include "../../lib/type_cast.hpp"

namespace opossum {

class FixedSizeAttributeVectorTest : public ::testing::Test {
 protected:
  size_t size = 5;
  FixedSizeAttributeVector<uint16_t> attribute_vector = FixedSizeAttributeVector<uint16_t>(size);
};

TEST_F(FixedSizeAttributeVectorTest, set) {
  for (size_t i = 0; i < size; i++) {
    attribute_vector.set(i, static_cast<ValueID>(i));
  }

  // test size
  EXPECT_EQ(attribute_vector.size(), size);

  // test single elements
  EXPECT_EQ(attribute_vector.get(0), 0u);
  EXPECT_EQ(attribute_vector.get(1), 1u);
  EXPECT_EQ(attribute_vector.get(2), 2u);
  EXPECT_EQ(attribute_vector.get(3), 3u);
  EXPECT_EQ(attribute_vector.get(4), 4u);
}

}  // namespace opossum
