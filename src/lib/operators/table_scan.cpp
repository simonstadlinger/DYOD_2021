#include "table_scan.hpp"

#include <memory>
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
  auto input = _in_operator->get_output();
  // create position list
  //    iterate over chunks in table
  //      get column with column ID.
  //      resolve segment type.
  //      resolve column type.
  //      do comparison, append RowId if match.

  auto pos_list = std::make_shared<PosList>();
  auto& reference_table = input;
  auto chunk_count = input->chunk_count();
  for(auto chunk_index = ChunkID{0}; chunk_index < chunk_count; ++chunk_index) {
    auto& chunk = input->get_chunk(chunk_index);
    auto base_column_segment = chunk.get_segment(_column_id);
    auto data_type = input->column_type(_column_id);
    resolve_data_type(data_type, [&] (auto type) {
      using Type = typename decltype(type)::type;
      auto value_segment = std::dynamic_pointer_cast<ValueSegment<Type>>(base_column_segment);
      if(value_segment) {
        // trivial for value segment

        switch(_scan_type) {
          case ScanType::OpEquals:
            break;
          case ScanType::OpNotEquals:
            break;
          case ScanType::OpLessThan: {
            auto segment_size = value_segment->size();
            for (auto segment_position = ChunkOffset{0}; segment_position < segment_size; ++segment_position) {
              if ((*value_segment)[segment_position] < _search_value) {
                pos_list->push_back(RowID{chunk_index, segment_position});
              }
            }
            break;
          }
          case ScanType::OpLessThanEquals:
            break;
          case ScanType::OpGreaterThan:
            break;
          case ScanType::OpGreaterThanEquals: {
            auto segment_size = value_segment->size();
            for (auto segment_position = ChunkOffset{0}; segment_position < segment_size; ++segment_position) {
              if ((*value_segment)[segment_position] >= _search_value) {
                pos_list->push_back(RowID{chunk_index, segment_position});
              }
            }
            break;
          }
            break;
          default:
            throw "Scan Type not implemented";
            break;
        }
      } else {
        auto dictionary_segment = std::dynamic_pointer_cast<DictionarySegment<Type>>(base_column_segment);
        if(dictionary_segment) {
          // dictionary segment
        } else {
          auto referenced_segment = std::dynamic_pointer_cast<ReferenceSegment>(base_column_segment);
          reference_table = referenced_segment->referenced_table();
          for(auto position: *(referenced_segment->pos_list())) {
            auto& chunk = reference_table->get_chunk(position.chunk_id);
            auto segment = chunk.get_segment(_column_id);
            auto data_type = reference_table->column_type(_column_id);
            resolve_data_type(data_type, [&] (auto type) {
              using ColumnType = typename decltype(type)::type;
              auto typed_segment = std::dynamic_pointer_cast<ValueSegment<ColumnType>>(base_column_segment);
              if (typed_segment) {
                switch(_scan_type) {
                  case ScanType::OpEquals:
                    break;
                  case ScanType::OpNotEquals:
                    break;
                  case ScanType::OpLessThan: {
                    auto segment_size = typed_segment->size();
                    for (auto segment_position = ChunkOffset{0}; segment_position < segment_size; ++segment_position) {
                      if ((*typed_segment)[segment_position] < _search_value) {
                        pos_list->push_back(RowID{chunk_index, segment_position});
                      }
                    }
                    break;
                  }
                  case ScanType::OpLessThanEquals:
                    break;
                  case ScanType::OpGreaterThan:
                    break;
                  case ScanType::OpGreaterThanEquals: {
                    auto segment_size = typed_segment->size();
                    for (auto segment_position = ChunkOffset{0}; segment_position < segment_size; ++segment_position) {
                      if ((*typed_segment)[segment_position] >= _search_value) {
                        pos_list->push_back(RowID{chunk_index, segment_position});
                      }
                    }
                    break;
                  }
                    break;
                  default:
                    throw "Scan Type not implemented";
                    break;
                }
              }
            });
          }
          //reference segment
        }
      }
    });
  }

  // create output table
  //    create one chunk.
  //    check for empty result, and return if necessary.
  //    create ref_segments for each column in input table.
  //    put pos list in each ref_segment.

  auto output_chunk = std::make_shared<Chunk>();
  auto col_count = input->column_count();
  for(auto column_index = ColumnID{0}; column_index < col_count; ++column_index) {
    auto ref_seg = std::make_shared<ReferenceSegment>(reference_table, column_index, pos_list);
    output_chunk->add_segment(std::dynamic_pointer_cast<BaseSegment>(ref_seg));
  }

  return std::make_shared<const Table>(output_chunk);
}
}  // namespace opossum