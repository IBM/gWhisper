// Copyright 2019 IBM Corporation
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <libCli/MessageFormatter.hpp>
#include <google/protobuf/util/json_util.h>

namespace cli
{
    std::string MessageFormatterJson::messageToString(
            const grpc::protobuf::Message & f_message,
            const grpc::protobuf::Descriptor* f_messageDescriptor)
    {
        std::string resultString;
        google::protobuf::util::JsonPrintOptions printOptions;
        printOptions.preserve_proto_field_names = true;

        // Will print primitive fields regardless of their values. So e.g. an int32 field set to 0 will not be omitted. 
        printOptions.always_print_fields_with_no_presence = true;

        const auto status = google::protobuf::util::MessageToJsonString(f_message, &resultString, printOptions);
        if(not status.ok())
        {
            resultString = std::string("Json encoding of Message failed: ") + status.ToString();
        }
        return resultString;
    }
}
