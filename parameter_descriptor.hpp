#ifndef PARAMETER_DESCRIPTOR_HPP_STOMPBLOCKS_INCLUDED
#define PARAMETER_DESCRIPTOR_HPP_STOMPBLOCKS_INCLUDED
#include <string>
#include <vector>
#include <memory>
#include "describable.hpp"

namespace sblocks {

class parameter;

/// \brief An abstract base class for all parameter descriptors. A parameter
///        descriptor acts like a class for a parameter (it's instance).
///
/// Being a subclass of descriptor (and describable), every parameter
/// descriptor has a name and a description. Also, more importantly, it must be
/// able to instantiate itself returning a pointer to a parameter.
struct parameter_descriptor : descriptor {
  parameter_descriptor(std::string name, std::string descr);
  virtual std::unique_ptr<parameter> instantiate() const = 0;
};

/// \brief A descriptor for a switch_parameter.
///
/// Nonbrief part.
struct switch_parameter_descriptor : parameter_descriptor {
  switch_parameter_descriptor(std::string name, std::string descr,
      std::vector<std::string> options_);

  std::unique_ptr<parameter> instantiate() const override;

  const std::vector<std::string> options;
};

/// \brief A descriptor for a number_parameter.
///
/// Nonbrief part.
struct number_parameter_descriptor : parameter_descriptor {
  number_parameter_descriptor(std::string name, std::string descr, double min_,
      double max_);

  std::unique_ptr<parameter> instantiate() const override;

  const double min, max;
};

}

#endif
