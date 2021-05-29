#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base_segment.hpp"
#include "dictionary_segment.hpp"
#include "resolve_type.hpp"
#include "table.hpp"
#include "types.hpp"
#include "utils/assert.hpp"
#include "value_segment.hpp"

namespace opossum {

// ReferenceSegment is a specific segment type that stores all its values as position list of a referenced segment
class ReferenceSegment : public BaseSegment {
 public:
  // creates a reference segment
  // the parameters specify the positions and the referenced segment
  ReferenceSegment(const std::shared_ptr<const Table>& referenced_table, const ColumnID referenced_column_id,
                   const std::shared_ptr<const PosList>& pos)
      : _ref_table(referenced_table), _ref_col_id(referenced_column_id), _pos_list(pos) {}

  AllTypeVariant operator[](const ChunkOffset chunk_offset) const override {
    auto position = _pos_list->at(chunk_offset);
    auto& chunk = _ref_table->get_chunk(position.chunk_id);
    auto segment = chunk.get_segment(_ref_col_id);
    return (*segment)[position.chunk_offset];
  };

  void append(const AllTypeVariant&) override { throw std::logic_error("ReferenceSegment is immutable"); };

  ChunkOffset size() const override { return _pos_list->size(); }

  const std::shared_ptr<const PosList>& pos_list() const { return _pos_list; }
  const std::shared_ptr<const Table>& referenced_table() const { return _ref_table; }

  ColumnID referenced_column_id() const { return _ref_col_id; }

  size_t estimate_memory_usage() const final { return (sizeof(RowID) * _pos_list->size()); }

 protected:
  const std::shared_ptr<const Table> _ref_table;
  const ColumnID _ref_col_id;
  const std::shared_ptr<const PosList> _pos_list;
};

}  // namespace opossum
