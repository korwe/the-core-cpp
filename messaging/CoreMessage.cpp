/* 
 * Copyright 2010 Korwe Software
 *
 * This file is part of TheCore.
 *
 * TheCore is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * TheCore is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TheCore.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Implementation of Core Message classes
 */

#include <string>

#include <sstream>
#include <utility>
#include <algorithm>

#include <boost/lexical_cast.hpp>
//#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "CoreMessage.h"

const std::string CoreMessage::lookupTypeName(CoreMessage::MessageType messageType) {
     switch(messageType) {
         case CoreMessage::INITIATE_SESSION_REQUEST:  return "InitiateSessionRequest";
         case CoreMessage::KILL_SESSION_REQUEST:      return "KillSessionRequest";
         case CoreMessage::SERVICE_REQUEST:           return "ServiceRequest";
         case CoreMessage::INITIATE_SESSION_RESPONSE: return "InitiateSessionResponse";
         case CoreMessage::KILL_SESSION_RESPONSE:     return "KillSessionResponse";
         case CoreMessage::SERVICE_RESPONSE:          return "ServiceResponse";
         case CoreMessage::DATA_RESPONSE:             return "DataResponse";
         default:                                     return "Unknown";
     }
 }

 const CoreMessage::MessageType CoreMessage::lookupType(const std::string& messageTypeName) {
     if ("InitiateSessionRequest" == messageTypeName) return CoreMessage::INITIATE_SESSION_REQUEST;
     else if ("KillSessionRequest" == messageTypeName) return CoreMessage::KILL_SESSION_REQUEST;
     else if ("ServiceRequest" == messageTypeName) return CoreMessage::SERVICE_REQUEST;
     else if ("InitiateSessionResponse" == messageTypeName) return CoreMessage::INITIATE_SESSION_RESPONSE;
     else if ("KillSessionResponse" == messageTypeName) return CoreMessage::KILL_SESSION_RESPONSE;
     else if ("ServiceResponse" == messageTypeName) return CoreMessage::SERVICE_RESPONSE;
     else if ("DataResponse" == messageTypeName) return CoreMessage::DATA_RESPONSE;
     else return CoreMessage::UNKNOWN_MESSAGE_TYPE;
 }

CoreMessage::CoreMessage(const std::string& sessionId, CoreMessage::MessageType messageType) :
    _sessionId(sessionId), _messageType(messageType),
    _timeStamp(boost::posix_time::microsec_clock::local_time()) {
}

CoreMessage::~CoreMessage() {
}

const std::string CoreMessage::choreography() const {
    return _choreography;
}

const CoreMessage::MessageType CoreMessage::messageType() const {
    return _messageType;
}

const std::string CoreMessage::guid() const {
    return _guid;
}

const std::string CoreMessage::sessionId() const {
    return _sessionId;
}

const std::string CoreMessage::description() const {
    if (!_description.empty())
        return _description;
    else
        return "Session Id: " + _sessionId + " Type: " + lookupTypeName(_messageType);
}

const boost::posix_time::ptime& CoreMessage::timeStamp() const {
    return _timeStamp;
}

const std::string CoreMessage::timeStampString() const {
    return boost::posix_time::to_iso_string(_timeStamp);
}

void CoreMessage::setTimeStamp(const boost::posix_time::ptime& timeStamp) {
    _timeStamp = timeStamp;
}

void CoreMessage::updateTimeStamp() {
    _timeStamp = boost::posix_time::microsec_clock::local_time();
}

void CoreMessage::setChoreography(const std::string& choreography) {
    _choreography = choreography;
}

void CoreMessage::setDescription(const std::string& description) {
    _description = description;
}

void CoreMessage::setGuid(const std::string& guid) {
    _guid = guid;
}


CoreRequest::CoreRequest(const std::string& sessionId, CoreMessage::MessageType messageType) :
        CoreMessage(sessionId, messageType) {
    createGuid();
}

CoreRequest::~CoreRequest() {
}

void CoreRequest::createGuid() {
    using namespace boost::uuids;
//    uuid_generator gen;
    random_generator gen;
    uuid basicUuid = gen();
//    setGuid(basicUuid.to_string());
    setGuid(to_string(basicUuid));
}

CoreResponse::CoreResponse(const std::string& sessionId, const std::string& guid,
                           CoreMessage::MessageType messageType) : CoreMessage(sessionId, messageType), 
                           _successful(true) {
    setGuid(guid);
}

CoreResponse::~CoreResponse() {
}

const bool CoreResponse::isSuccessful() const {
    return _successful;
}

const std::string CoreResponse::errorCode() const {
    return _errorCode;
}

const std::string CoreResponse::errorMessage() const {
    return _errorMessage;
}

void CoreResponse::setSuccessful(const bool successful) {
    _successful = successful;
}
void CoreResponse::setErrorCode(const std::string& errorCode) {
    _errorCode = errorCode;
}

void CoreResponse::setErrorMessage(const std::string& errorMessage) {
    _errorMessage = errorMessage;
}


/** @brief InitiateSessionRequest
  *
  * @todo: document this function
  */
InitiateSessionRequest::InitiateSessionRequest(const std::string& sessionId) :
        CoreRequest(sessionId, CoreMessage::INITIATE_SESSION_REQUEST) {
}

/** @brief ~InitiateSessionRequest
  *
  * @todo: document this function
  */
InitiateSessionRequest::~InitiateSessionRequest() {
}

/** @brief InitiateSessionResponse
  *
  * @todo: document this function
  */
InitiateSessionResponse::InitiateSessionResponse(const std::string& sessionId, const std::string& guid) :
        CoreResponse(sessionId, guid, CoreMessage::INITIATE_SESSION_RESPONSE) {
}

/** @brief ~InitiateSessionResponse
  *
  * @todo: document this function
  */
InitiateSessionResponse::~InitiateSessionResponse() {
}

/** @brief KillSessionRequest
  *
  * @todo: document this function
  */
KillSessionRequest::KillSessionRequest(const std::string& sessionId) : 
      CoreRequest(sessionId, CoreMessage::KILL_SESSION_REQUEST) {
}

/** @brief ~KillSessionRequest
  *
  * @todo: document this function
  */
KillSessionRequest::~KillSessionRequest() {
}

/** @brief KillSessionResponse
  *
  * @todo: document this function
  */
KillSessionResponse::KillSessionResponse(const std::string& sessionId, const std::string& guid) : 
      CoreResponse(sessionId, guid, CoreMessage::KILL_SESSION_RESPONSE) {
}

/** @brief ~KillSessionResponse
  *
  * @todo: document this function
  */
KillSessionResponse::~KillSessionResponse() {
}

/** @brief ServiceRequest
  *
  * @todo: document this function
  */
ServiceRequest::ServiceRequest(const std::string& sessionId, const std::string& functionName) : 
      CoreRequest(sessionId, CoreMessage::SERVICE_REQUEST), _functionName(functionName) {
}

/** @brief ~ServiceRequest
  *
  * @todo: document this function
  */
ServiceRequest::~ServiceRequest() {
}

const std::string ServiceRequest::functionName() const {
    return _functionName;
}

const std::vector<std::string>& ServiceRequest::parameterNames() const {
    return _paramNames;
}

const std::string ServiceRequest::location() const {
    return _location;
}

/** @brief parameterValue
  *
  * @todo: document this function
  */
const std::string ServiceRequest::parameterValue(const std::string& name) const {
    std::vector<std::string>::const_iterator found = std::find(_paramNames.begin(), _paramNames.end(), name);
    if (found == _paramNames.end()) {
        return "";
    }
    else {
        return _params.find(*found)->second;
    }
}

/** @brief setParameter
  *
  * @todo: document this function
  */
void ServiceRequest::setParameter(const std::string& name, const std::string& value) {
    const std::vector<std::string>::iterator found = std::find(_paramNames.begin(), _paramNames.end(), name);
    if (found == _paramNames.end()) {
        _paramNames.push_back(name);
    }
    _params[name] = value;
}

void ServiceRequest::setLocation(const std::string& location) {
    _location = location;
}

/** @brief ServiceResponse
  *
  * @todo: document this function
  */
ServiceResponse::ServiceResponse(const std::string& sessionId, const std::string& guid, bool hasData) : 
      CoreResponse(sessionId, guid, CoreMessage::SERVICE_RESPONSE), _hasData(hasData) {
}

/** @brief ~ServiceResponse
  *
  * @todo: document this function
  */
ServiceResponse::~ServiceResponse() {
}

/** @brief hasData
  *
  * @todo: document this function
  */
const bool ServiceResponse::hasData() const {
    return _hasData;
}

/** @brief DataResponse
  *
  * @todo: document this function
  */
DataResponse::DataResponse(const std::string& sessionId, 
                           const std::string& guid, 
                           const std::string& data) : CoreResponse(sessionId, guid, CoreMessage::DATA_RESPONSE), 
                                                      _data(data) {
}

/** @brief ~DataResponse
  *
  * @todo: document this function
  */
DataResponse::~DataResponse() {
}

/** @brief data
  *
  * @todo: document this function
  */
const std::string DataResponse::data() const {
    return _data;
}
