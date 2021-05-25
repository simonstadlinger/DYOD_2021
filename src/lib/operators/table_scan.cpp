#include "table_scan.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <storage/dictionary_segment.hpp>
#include <storage/reference_segment.hpp>
#include <storage/table.hpp>
#include <storage/value_segment.hpp>
#include "resolve_type.hpp"

namespace opossum {

TableScan::TableScan(const std::shared_ptr<const AbstractOperator>& in, const ColumnID column_id,
                     const ScanType scan_type, const AllTypeVariant search_value)
    : _in_operator(in), _column_id(column_id), _scan_type(scan_type), _search_value(search_value) {}

ColumnID TableScan::column_id() const { return _column_id; }

ScanType TableScan::scan_type() const { return _scan_type; }

const AllTypeVariant& TableScan::search_value() const { return _search_value; }

template <typename T>
std::function<bool(T, T)> TableScan::_get_comparator_method() {
  std::function<bool(T, T)> _comp;

  switch (_scan_type) {
    case ScanType::OpGreaterThanEquals:
      _comp = [&](const T search_value, const T comparison_value) { return search_value <= comparison_value; };
      break;
    case ScanType::OpGreaterThan:
      _comp = [&](const T search_value, const T comparison_value) { return search_value < comparison_value; };
      break;
    case ScanType::OpLessThanEquals:
      _comp = [&](const T search_value, const T comparison_value) { return search_value >= comparison_value; };
      break;
    case ScanType::OpLessThan:
      _comp = [&](const T search_value, const T comparison_value) { return search_value > comparison_value; };
      break;
    case ScanType::OpNotEquals:
      _comp = [&](const T search_value, const T comparison_value) { return search_value != comparison_value; };
      break;
    case ScanType::OpEquals:
      _comp = [&](const T search_value, const T comparison_value) { return search_value == comparison_value; };
      break;
    default:
      throw "Operator not implemented";
  }

  return _comp;
}

std::shared_ptr<const Table> TableScan::_on_execute() {
  auto input_table = _in_operator->get_output();

  // get characteristics for output table
  auto col_type = input_table->column_type(_column_id);
  auto column_count = input_table->column_count();

  // create empty output table
  auto output_table = std::make_shared<Table>(input_table->target_chunk_size());
  auto pos_list = std::make_shared<PosList>();

  // check if input table contains reference segments
  auto& probe_chunk = input_table->get_chunk(ChunkID{0});
  const auto& probe_segment = probe_chunk.get_segment(ColumnID{0});
  auto probe_ref_segment = std::dynamic_pointer_cast<ReferenceSegment>(probe_segment);
  if (probe_ref_segment) {
    // ref segment
    input_table = probe_ref_segment->referenced_table();
    for (auto position : *(probe_ref_segment->pos_list())) {
      auto chunk_id = position.chunk_id;
      auto chunk_offset = position.chunk_offset;

      auto segment = input_table->get_chunk(chunk_id).get_segment(_column_id);
      resolve_data_type(col_type, [&](const auto data_type_t) {
        using Type = typename decltype(data_type_t)::type;

        Assert((_search_value.type() == typeid(Type)), "Type of search value and column should be the same");

        auto value_segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
        auto _comparator = _get_comparator_method<Type>();
        if (value_segment) {
          // Value Segment
          auto comparison_value = type_cast<Type>(value_segment->values()[chunk_offset]);
          if (_comparator(type_cast<Type>(_search_value), comparison_value)) {
            pos_list->push_back(RowID{chunk_id, chunk_offset});
          }
        } else {
          // Dictionary Segment
          auto dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<Type>>(segment);
          auto comparison_value = type_cast<Type>(dictionary_segment->get(chunk_offset));
          if (_comparator(type_cast<Type>(_search_value), comparison_value)) {
            pos_list->push_back(RowID{chunk_id, chunk_offset});
          }
        }
      });
    }
  } else {
    // no ref segment
    auto chunk_count = input_table->chunk_count();
    for (auto chunk_id = ChunkID{0}; chunk_id < chunk_count; ++chunk_id) {
      auto& chunk = input_table->get_chunk(chunk_id);
      std::shared_ptr<BaseSegment> segment = chunk.get_segment(_column_id);

      // Fill position list for different segment types
      resolve_data_type(col_type, [&](const auto data_type_t) {
        using Type = typename decltype(data_type_t)::type;

        Assert((_search_value.type() == typeid(Type)), "Type of search value and column should be the same");

        auto value_segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
        auto _comparator = _get_comparator_method<Type>();
        if (value_segment) {
          // Value Segment
          auto chunk_size = chunk.size();
          for (auto chunk_offset = ChunkOffset{0}; chunk_offset < chunk_size; ++chunk_offset) {
            // TODO(anyone): extract method
            auto comparison_value = type_cast<Type>(value_segment->values()[chunk_offset]);
            if (_comparator(type_cast<Type>(_search_value), comparison_value)) {
              pos_list->push_back(RowID{chunk_id, chunk_offset});
            }
          }
        } else {
          // Dictionary Segment
          auto dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<Type>>(segment);
          auto chunk_size = chunk.size();
          for (auto chunk_offset = ChunkOffset{0}; chunk_offset < chunk_size; ++chunk_offset) {
            // TODO(anyone): extract method (#48)
            auto comparison_value = type_cast<Type>(dictionary_segment->get(chunk_offset));
            if (_comparator(type_cast<Type>(_search_value), comparison_value)) {
              pos_list->push_back(RowID{chunk_id, chunk_offset});
            }
          }
        }
      });
    }
  }

  std::shared_ptr<Chunk> final_chunk = std::make_shared<Chunk>(column_count);
  for (auto column_id = ColumnID{0}; ColumnCount{column_id} < column_count; column_id++) {
    // for each col, create Reference Segment and append to table
    const auto& reference_segment = std::make_shared<ReferenceSegment>(input_table, column_id, pos_list);
    final_chunk->add_segment(reference_segment);
    output_table->add_column_definition(input_table->column_name(column_id), input_table->column_type(column_id));
  }

  output_table->emplace_chunk(final_chunk);

  return output_table;
}

}  // namespace opossum
