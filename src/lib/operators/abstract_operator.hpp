#pragma once

#include <memory>
#include <string>
#include <vector>
#include <boost/thread/once.hpp>
#include "types.hpp"

namespace opossum {

class Table;

// AbstractOperator is the abstract super class for all operators.
// All operators have up to two input tables and one output table.
// Their lifecycle has three phases:
// 1. The operator is constructed. Previous operators are not guaranteed to have already executed, so operators must not
// call get_output in their execute method
// 2. The execute method is called from the outside (usually by the scheduler). This is where the heavy lifting is done.
// By now, the input operators have already executed.
// 3. The consumer (usually another operator) calls get_output. This should be very cheap. It is only guaranteed to
// succeed if execute was called before. Otherwise, a nullptr or an empty table could be returned.
//
// Operators shall not be executed twice.

class AbstractOperator : private Noncopyable {
 public:

  AbstractOperator(const std::shared_ptr<const AbstractOperator> left = nullptr,
                   const std::shared_ptr<const AbstractOperator> right = nullptr);

  virtual ~AbstractOperator() = default;

  // we need to explicitly set the move constructor to default when
  // we overwrite the copy constructor
  AbstractOperator(AbstractOperator&&) = default;
  AbstractOperator& operator=(AbstractOperator&&) = default;

  void execute();

  // returns the result of the operator
  std::shared_ptr<const Table> get_output() const;

  // Get the input operators.
  std::shared_ptr<const AbstractOperator> left_input() const;
  std::shared_ptr<const AbstractOperator> right_input() const;

 protected:
  // abstract method to actually execute the operator
  // execute and get_output are split into two methods to allow for easier
  // asynchronous execution


  virtual std::shared_ptr<const Table> _on_execute() = 0;

  std::shared_ptr<const Table> _left_input_table() const;
  std::shared_ptr<const Table> _right_input_table() const;

  //wrapper function to allow tracking of method calls in public execute interface.
  bool _has_been_executed;

  // Shared pointers to input operators, can be nullptr.
  std::shared_ptr<const AbstractOperator> _left_input;
  std::shared_ptr<const AbstractOperator> _right_input;

  // Is nullptr until the operator is executed
  std::shared_ptr<const Table> _output;
};

}  // namespace opossum
