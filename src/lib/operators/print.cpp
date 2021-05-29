#include "print.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "operators/table_wrapper.hpp"
#include "storage/base_segment.hpp"
#include "storage/table.hpp"
#include "type_cast.hpp"

namespace opossum {

Print::Print(const std::shared_ptr<const AbstractOperator> in, std::ostream& out) : AbstractOperator(in), _out(out) {}

void Print::print(std::shared_ptr<const Table>& table, std::ostream& out) {
  auto table_wrapper = std::make_shared<TableWrapper>(table);
  table_wrapper->execute();
  Print(table_wrapper, out).execute();
}

std::shared_ptr<const Table> Print::_on_execute() {
  auto widths = _column_string_widths(8, 20, _left_input_table());

  // print column headers
  _out << "=== Columns" << std::endl;
  for (auto column_id = ColumnID{0}; column_id < _left_input_table()->column_count(); ++column_id) {
    _out << "|" << std::setw(widths[column_id]) << _left_input_table()->column_name(column_id) << std::setw(0);
  }
  _out << "|" << std::endl;
  for (auto column_id = ColumnID{0}; column_id < _left_input_table()->column_count(); ++column_id) {
    _out << "|" << std::setw(widths[column_id]) << _left_input_table()->column_type(column_id) << std::setw(0);
  }
  _out << "|" << std::endl;

  // print each chunk
  for (auto chunk_id = ChunkID{0}; chunk_id < _left_input_table()->chunk_count(); ++chunk_id) {
    const auto& chunk = _left_input_table()->get_chunk(chunk_id);

    _out << "=== Chunk " << chunk_id << " === " << std::endl;

    if (chunk.size() == 0) {
      _out << "Empty chunk." << std::endl;
      continue;
    }

    // print the rows in the chunk
    for (size_t row = 0; row < chunk.size(); ++row) {
      _out << "|";
      for (auto column_id = ColumnID{0}; column_id < chunk.column_count(); ++column_id) {
        // well yes, we use BaseSegment::operator[] here, but since Print is not an operation that should
        // be part of a regular query plan, let's keep things simple here
        _out << std::setw(widths[column_id]) << (*chunk.get_segment(column_id))[row] << "|" << std::setw(0);
      }

      _out << std::endl;
    }
  }

  return _left_input_table();
}

// In order to print the table as an actual table, with columns being aligned, we need to calculate the
// number of characters in the printed representation of each column
// `min` and `max` can be used to limit the width of the columns - however, every column fits at least the column's name
std::vector<uint16_t> Print::_column_string_widths(uint16_t min, uint16_t max,
                                                   const std::shared_ptr<const Table>& table) const {
  auto widths = std::vector<uint16_t>(table->column_count());
  // calculate the length of the column name
  for (auto column_id = ColumnID{0}; column_id < table->column_count(); ++column_id) {
    widths[column_id] = std::max(min, static_cast<uint16_t>(table->column_name(column_id).size()));
  }

  // go over all rows and find the maximum length of the printed representation of a value, up to max
  for (auto chunk_id = ChunkID{0}; chunk_id < _left_input_table()->chunk_count(); ++chunk_id) {
    auto& chunk = _left_input_table()->get_chunk(chunk_id);

    for (auto column_id = ColumnID{0}; column_id < chunk.column_count(); ++column_id) {
      for (size_t row = 0; row < chunk.size(); ++row) {
        auto cell_length =
            static_cast<uint16_t>(boost::lexical_cast<std::string>((*chunk.get_segment(column_id))[row]).size());
        widths[column_id] = std::max({min, widths[column_id], std::min(max, cell_length)});
      }
    }
  }
  return widths;
}

}  // namespace opossum
