/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "ignition/transport/parameters/Client.hh"

#include <memory>
#include <sstream>
#include <string>

#include "ignition/msgs/boolean.pb.h"
#include "ignition/msgs/parameter_name.pb.h"
#include "ignition/msgs/parameter_value.pb.h"

#include "ignition/transport/parameters/exceptions.hh"

using namespace ignition;
using namespace transport;
using namespace parameters;

struct ignition::transport::parameters::ParametersClientPrivate
{
  ParametersClientPrivate(
    const std::string & _serverNamespace,
    unsigned int _timeoutMs)
  : serverNamespace{_serverNamespace},
    timeoutMs{_timeoutMs}
  {}
  std::string serverNamespace;
  mutable ignition::transport::Node node;
  unsigned int timeoutMs;
};

ParametersClient::~ParametersClient() = default;

ParametersClient::ParametersClient(
  const std::string & _serverNamespace,
  unsigned int _timeoutMs)
: dataPtr{std::make_unique<ParametersClientPrivate>(
    _serverNamespace,
    _timeoutMs)}
{}

std::unique_ptr<google::protobuf::Message>
ParametersClient::Parameter(const std::string & _parameterName) const
{
  bool result{false};
  const std::string service{dataPtr->serverNamespace + "/get_parameter"};

  msgs::ParameterName req;
  msgs::ParameterValue res;

  req.set_name(_parameterName);

  if (!dataPtr->node.Request(service, req, dataPtr->timeoutMs, res, result))
  {
    throw std::runtime_error{
      "ParametersClient::Parameter(): request timed out"};
  }
  if (!result)
  {
    throw ParameterNotDeclaredException {
      "ParametersClient::Parameter()", _parameterName.c_str()};
  }
  std::unique_ptr<google::protobuf::Message> ret =
    ignition::msgs::Factory::New(res.type());
  std::istringstream iss{res.value()};
  ret->ParseFromIstream(&iss);
  return ret;
}

void
ParametersClient::SetParameter(
  const std::string & _parameterName,
  const google::protobuf::Message & _msg) const
{
  bool result{false};
  const std::string service{dataPtr->serverNamespace + "/set_parameter"};

  std::string protoType{"ign_msgs."};
  protoType += _msg.GetDescriptor()->name();

  msgs::Parameter req;
  msgs::Boolean res;

  req.set_name(_parameterName);
  req.set_type(std::move(protoType));

  std::ostringstream oss;
  _msg.SerializeToOstream(&oss);
  req.set_value(oss.str());

  if (!dataPtr->node.Request(service, req, dataPtr->timeoutMs, res, result))
  {
    throw std::runtime_error{
      "ParametersClient::SetParameter(): request timed out"};
  }
  if (!result)
  {
    throw ParameterNotDeclaredException {
      "ParametersClient::SetParameter()", _parameterName.c_str()};
  }
}

void
ParametersClient::DeclareParameter(
  const std::string & _parameterName,
  const google::protobuf::Message & _msg) const
{
  bool result{false};
  const std::string service{dataPtr->serverNamespace + "/declare_parameter"};

  std::string protoType{"ign_msgs."};
  protoType += _msg.GetDescriptor()->name();

  msgs::Parameter req;
  msgs::Boolean res;

  req.set_name(_parameterName);
  req.set_type(std::move(protoType));

  std::ostringstream oss;
  _msg.SerializeToOstream(&oss);
  req.set_value(oss.str());

  if (!dataPtr->node.Request(service, req, dataPtr->timeoutMs, res, result))
  {
    throw std::runtime_error{
      "ParametersClient::DeclareParameter(): request timed out"};
  }
  if (!result)
  {
    throw ParameterAlreadyDeclaredException {
      "ParametersClient::DeclareParameter()", _parameterName.c_str()};
  }
}

msgs::ParameterDeclarations
ParametersClient::ListParameters() const
{
  bool result{false};
  const std::string service{dataPtr->serverNamespace + "/list_parameters"};

  msgs::Empty req;
  msgs::ParameterDeclarations res;

  if (!dataPtr->node.Request(service, req, dataPtr->timeoutMs, res, result))
  {
    throw std::runtime_error{
      "ParametersClient::ListParameters(): request timed out"};
  }
  if (!result)
  {
    throw std::runtime_error {
      "ParametersClient::ListParameters(): unexpected error"};
  }
  return res;
}
