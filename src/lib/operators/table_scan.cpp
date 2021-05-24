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
         const AllTypeVariant search_value):_in_operator(in), _column_id(column_id), _scan_type(scan_type), _search_value(search_value) {
  _set_comparator_method();
}

ColumnID TableScan::column_id() const {
  return _column_id;
}

ScanType TableScan::scan_type() const {
  return _scan_type;
}

const AllTypeVariant& TableScan::search_value() const {
  return _search_value;
}

void TableScan::_set_comparator_method() {
  switch (_scan_type) {
    case ScanType::OpGreaterThanEquals:
      _comparator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value <= comparison_value;
      };
      break;
    case ScanType::OpGreaterThan:
      _comparator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value < comparison_value;
      };
      break;
    case ScanType::OpLessThanEquals:
      _comparator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value >= comparison_value;
      };
      break;
    case ScanType::OpLessThan:
      _comparator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value > comparison_value;
      };
      break;
    case ScanType::OpNotEquals:
      _comparator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value != comparison_value;
      };
      break;
    case ScanType::OpEquals:
      _comparator = [&] (const AllTypeVariant& search_value, const AllTypeVariant& comparison_value) {
        return search_value == comparison_value;
      };
      break;
    default:
      throw "Operator not implemented";
  }
}

std::shared_ptr<const Table> TableScan::_on_execute() {

  auto input_table = _in_operator->get_output();

  // create empty output table
  auto output_table = std::make_shared<Table>(input_table->target_chunk_size());


  // check if input table contains reference segments
  bool is_ref_table = false;
  auto& probe_chunk = input_table->get_chunk(ChunkID{0});
  const auto& probe_segment = probe_chunk.get_segment(ColumnID{0});

  auto probe_ref_segment = std::dynamic_pointer_cast<ReferenceSegment>(probe_segment);
  std::shared_ptr<const Table> orig_input_table;
  if(probe_ref_segment) {
    orig_input_table = input_table;
    is_ref_table = true;
    input_table = probe_ref_segment->referenced_table();

  }


  auto col_type = input_table->column_type(_column_id);
  auto chunk_count = input_table->chunk_count();
  auto column_count = input_table->column_count();
  for(auto chunk_id = ChunkID{0}; chunk_id < chunk_count; ++chunk_id) {
    auto pos_list = std::make_shared<PosList>();
    auto& chunk = input_table->get_chunk(chunk_id);
    std::shared_ptr<BaseSegment> segment = chunk.get_segment(_column_id);
    std::shared_ptr<ReferenceSegment> ref_segment;
    if(is_ref_table) {
      ref_segment = std::dynamic_pointer_cast<ReferenceSegment>(orig_input_table->get_chunk(ChunkID{0}).get_segment(_column_id));
    }

    // Fill position list for different segment types
    resolve_data_type(col_type, [&](const auto data_type_t) {
      using Type = typename decltype(data_type_t)::type;
      segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
      if(segment) {

        // Value Segment
        auto value_segment = std::dynamic_pointer_cast<ValueSegment<Type>>(segment);
        if(!is_ref_table) {
          auto chunk_size = chunk.size();
          for(auto chunk_offset = ChunkOffset{0}; chunk_offset < chunk_size; ++chunk_offset) {
            // TODO: extract method
            auto comparison_value = value_segment->values()[chunk_offset];
            if(_comparator(_search_value, comparison_value)) {
              pos_list->push_back(RowID{chunk_id, chunk_offset});
            }
          }
        } else {
          for(auto position: *(ref_segment->pos_list())) {
            auto chunk_offset = position.chunk_offset;
            auto comparison_value = value_segment->values()[chunk_offset];
            if(_comparator(_search_value, comparison_value)) {
              pos_list->push_back(RowID{chunk_id, chunk_offset});
            }
          }
        }

      } else {

        // Dictionary Segment
        auto dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<Type>>(segment);
        if(!is_ref_table) {
          auto chunk_size = chunk.size();
          for(auto chunk_offset = ChunkOffset{0}; chunk_offset < chunk_size; ++chunk_offset) {
            // TODO: extract method
            auto comparison_value = dictionary_segment->get(chunk_offset);
            if(_comparator(_search_value, comparison_value)) {
              pos_list->push_back(RowID{chunk_id, chunk_offset});
            }
          }
        } else {
          for(auto position: *(ref_segment->pos_list())) {
            auto chunk_offset = position.chunk_offset;
            auto comparison_value = dictionary_segment->get(chunk_offset);
            if(_comparator(_search_value, comparison_value)) {
              pos_list->push_back(RowID{chunk_id, chunk_offset});
            }
          }
        }
      }
    });

    // create output chunk for input chunk;
    std::shared_ptr<Chunk> final_chunk = std::make_shared<Chunk>();
    for(auto column_id = ColumnID{0}; column_id < column_count; column_id++) {
      const auto& reference_segment = std::make_shared<ReferenceSegment>(input_table, column_id,  pos_list);
      final_chunk->add_segment(reference_segment);
      output_table->add_column_definition(input_table->column_name(column_id), input_table->column_type(column_id));
    }

    output_table->emplace_chunk(final_chunk);
  }

  // for each col, create Reference Segment and append to table



  return output_table;
}



}  // namespace opossum