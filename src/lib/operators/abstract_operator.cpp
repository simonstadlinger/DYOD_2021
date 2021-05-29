#include "abstract_operator.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "storage/table.hpp"
#include "utils/assert.hpp"

namespace opossum {

AbstractOperator::AbstractOperator(const std::shared_ptr<const AbstractOperator> left,
                                   const std::shared_ptr<const AbstractOperator> right)
    : _has_been_executed(false), _left_input(left), _right_input(right) {}

void AbstractOperator::execute() {
  if (!_has_been_executed) {
    _has_been_executed = true;
    _output = _on_execute();

  } else {
    throw std::runtime_error("Operators can only be executed once");
  }
}

std::shared_ptr<const Table> AbstractOperator::get_output() const {
  if (_has_been_executed) {
    return _output;
  } else {
    return std::nullptr_t();
  }
}

std::shared_ptr<const Table> AbstractOperator::_left_input_table() const { return _left_input->get_output(); }

std::shared_ptr<const Table> AbstractOperator::_right_input_table() const { return _right_input->get_output(); }

}  // namespace opossum
