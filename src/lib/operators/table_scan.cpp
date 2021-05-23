#include "table_scan.hpp"

#include <memory>
#include <functional>
#include <storage/reference_segment.hpp>
#include <string>
#include <vector>
#include "../storage/dictionary_segment.hpp"
#include "../storage/table.hpp"
#include "../storage/value_segment.hpp"
#include "resolve_type.hpp"


namespace opossum {

TableScan::TableScan(const std::shared_ptr<const AbstractOperator>& in, const ColumnID column_id, const ScanType scan_type,
         const AllTypeVariant search_value):_in_operator(in), _column_id(column_id), _scan_type(scan_type), _search_value(search_value) { }

ColumnID TableScan::column_id() const {
  return _column_id;
}

ScanType TableScan::scan_type() const {
  return _scan_type;
}

const AllTypeVariant& TableScan::search_value() const {
  return _search_value;
}

std::shared_ptr<const Table> TableScan::_on_execute() {


  auto input_table = _in_operator->get_output();

  // create empty output table
  auto output_table = std::make_shared<Table>(input_table->target_chunk_size());

  auto pos_list = std::make_shared<PosList>();
  // set operator function based on scan type
  std::function<bool(AllTypeVariant, AllTypeVariant)> _operator;
  switch (_scan_type) {
    case ScanType::OpGreaterThanEquals:
      _operator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value <= comparison_value;
      };
      break;
    case ScanType::OpGreaterThan:
      _operator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value < comparison_value;
      };
      break;
    case ScanType::OpLessThanEquals:
      _operator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value >= comparison_value;
      };
      break;
    case ScanType::OpLessThan:
      _operator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value > comparison_value;
      };
      break;
    case ScanType::OpNotEquals:
      _operator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value != comparison_value;
      };
      break;
    case ScanType::OpEquals:
      _operator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value == comparison_value;
      };
      break;
    default:
      throw "Operator not implemented";
      break;
  }

  // check if input table contains reference segments
  bool is_ref_table = false;
  auto& probe_chunk = input_table->get_chunk(ChunkID{0});
  const auto& probe_segment = probe_chunk.get_segment(ColumnID{0});
  if(std::dynamic_pointer_cast<ReferenceSegment>(probe_segment)) {
    is_ref_table = true;
  }

  auto chunk_count = input_table->chunk_count();
  auto col_type = input_table->column_type(_column_id);
  for(auto chunk_id = ChunkID{0}; chunk_id < chunk_count; ++chunk_id) {
    auto& chunk = input_table->get_chunk(chunk_id);
    std::shared_ptr<BaseSegment> segment = chunk.get_segment(_column_id);
    std::shared_ptr<ReferenceSegment> ref_segment;
    if(is_ref_table) {
      ref_segment = std::dynamic_pointer_cast<ReferenceSegment>(segment);
      segment = ref_segment->referenced_table()->get_chunk(chunk_id).get_segment(_column_id);
    }

    // set accessor for different segment types
    std::function<AllTypeVariant (ChunkOffset, const std::shared_ptr<BaseSegment>&)> _accessor;
    resolve_data_type(col_type, [&](const auto data_type_t) {
      using Type = typename decltype(data_type_t)::type;
      segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
      if(segment) {
        segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
        _accessor = [&](ChunkOffset row_id, const std::shared_ptr<BaseSegment>& segment) {
          return segment[row_id];
        };
      } else {
        segment = std::dynamic_pointer_cast<DictionarySegment<Type>>(segment);
        _accessor = [&](ChunkOffset row_id, const std::shared_ptr<BaseSegment>& segment) {
          return segment[row_id];
        };
      }

      // do the actual comparison and fill position list
      if(!is_ref_table) {
        auto chunk_size = chunk.size();
        for(auto chunk_offset = ChunkOffset{0}; chunk_offset < chunk_size; ++chunk_offset) {
          // TODO: extract method
          auto comparison_value = _accessor(chunk_offset, segment);
          if(_operator(_search_value, comparison_value)) {
            pos_list->push_back(RowID{chunk_id, chunk_offset});
          }
        }
      } else {
        for(auto position: *(ref_segment->pos_list())) {
          auto chunk_offset = position.chunk_offset;
          auto comparison_value = _accessor(chunk_offset, segment);
          if(_operator(_search_value, comparison_value)) {
            pos_list->push_back(RowID{chunk_id, chunk_offset});
          }
        }
      }


    });

  }

  return output_table;
}



}  // namespace opossum