#include "definitions.hpp"
#include <algorithm>
#include <string>
#include <json/json.h>
#include "parameter.hpp"


namespace sblocks {

class parameter_initializer : public parameter_visitor {
public:
  parameter_initializer(const Json::Value &obj): _obj(obj) {}

  void visit(switch_parameter &p) override {
    p.set(_obj.asUInt());
  }

  void visit(number_parameter &p) override {
    p.set(_obj.asDouble());
  }

private:
  const Json::Value &_obj;
};

static std::string get_string(const Json::Value &obj) {
  if (obj.isString()) {
    std::string str(std::move(obj.asString()));
    if (str.size())
      return std::move(str);
  }
  throw -999;
}

static std::unique_ptr<parameter_descriptor>
read_parameter_descriptor(const Json::Value &obj) {
  std::string name(std::move(get_string(obj["name"])));
  std::string description(std::move(get_string(obj["description"])));
  const Json::Value &options = obj["options"];
  if (options.isArray()) {
    if (options.size() < 2)
      throw -3;
    std::vector<std::string> options_vec;
    for (const auto &opt : options) {
      options_vec.emplace_back(std::move(get_string(opt)));
    }
    return std::unique_ptr<parameter_descriptor>(
        new switch_parameter_descriptor(std::move(name),
            std::move(description), std::move(options_vec))
    );
  }
  const Json::Value &min = obj["min"];
  const Json::Value &max = obj["max"];
  if (!(min.isNumeric() && max.isNumeric()))
    throw -4;
  return std::unique_ptr<parameter_descriptor>(new number_parameter_descriptor(
      std::move(name), std::move(description), min.asDouble(), max.asDouble())
  );
}

static std::vector<std::unique_ptr<parameter_descriptor>>
read_parameters(const Json::Value &json_params) {
  std::vector<std::unique_ptr<parameter_descriptor>> vec;
  for (const auto &obj : json_params) {
    vec.emplace_back(std::move(read_parameter_descriptor(obj)));
  }
  return std::move(vec);
}

static std::unique_ptr<parameter>
block_read_init_parameter(const Json::Value &obj,
    const std::vector<std::unique_ptr<parameter_descriptor>> &params) {
  if (!(obj.isArray() && obj.size() == 2))
    throw -23;
  const Json::Value &json_name = obj[0];
  if (!json_name.isString())
    throw -25;
  std::string name(std::move(json_name.asString()));
  auto iter = std::find_if(params.cbegin(), params.cend(),
      [&name](const auto &descr) {
        return descr->name() == name;
      }
  );
  if (iter == params.end())
    throw -78;
  std::unique_ptr<parameter> p(std::move((*iter)->instantiate()));
  parameter_initializer visitor(obj[1]);
  p->accept(visitor);
  return std::move(p);
}

static block_descriptor read_block_descriptor(const Json::Value &obj,
    const std::vector<std::unique_ptr<parameter_descriptor>> &params) {
  std::string name(std::move(get_string(obj["name"])));
  std::string description(std::move(get_string(obj["description"])));
  unsigned input_n, output_n;
  input_n = obj["input_n"].asUInt();
  output_n = obj["output_n"].asUInt();
  const Json::Value &param_array = obj["parameters"];
  if (!param_array.isArray())
    throw -20;
  std::vector<std::unique_ptr<parameter>> vec;
  for (const auto &param_obj : param_array) {
    vec.emplace_back(std::move(block_read_init_parameter(param_obj, params)));
  }
  return block_descriptor(std::move(name), std::move(description), input_n,
      output_n, std::move(vec));
}

static std::vector<block_descriptor>read_blocks(const Json::Value &json_blocks,
    const std::vector<std::unique_ptr<parameter_descriptor>> &params) {
  std::vector<block_descriptor> vec;
  for (const auto &obj : json_blocks) {
    vec.emplace_back(std::move(read_block_descriptor(obj, params)));
  }
  return std::move(vec);
}

definition_handler::definition_handler(std::istream &in) {
  Json::Value root;
  in >> root;
  const Json::Value &json_params = root["parameters"];
  if (!(json_params.isArray() && json_params.size()))
    throw -1;
  _parameter_descriptors = std::move(read_parameters(json_params));
  const Json::Value &json_blocks = root["blocks"];
  if (!(json_blocks.isArray() && json_blocks.size()))
    throw -1;
  _block_descriptors = std::move(read_blocks(json_blocks,
      _parameter_descriptors));
}

definition_handler::const_iterator definition_handler::cbegin() const {
  return _block_descriptors.cbegin();
}

definition_handler::const_iterator definition_handler::cend() const {
  return _block_descriptors.cend();
}

}
