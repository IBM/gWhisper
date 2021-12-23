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

#include <libCli/MessageParsing.hpp>
#include <fstream>
#include <sstream>
#include <google/protobuf/util/json_util.h>

using namespace ArgParse;

namespace cli
{

std::vector<std::unique_ptr<google::protobuf::Message>> MessageParserJson::parseMessages(
        ArgParse::ParsedElement & f_parseTree,
        google::protobuf::DynamicMessageFactory & f_factory,
        const google::protobuf::Descriptor* f_messageDescriptor,
        bool f_isClientStreamingRpc
        )
{
        std::vector<ArgParse::ParsedElement *> jsonInputs;
        // search all passed messages: (true flag prevents searching sub-messages)
        f_parseTree.findAllSubTrees("JsonInput", jsonInputs, true);

        std::vector<std::unique_ptr<google::protobuf::Message>> result;
        // Write all request messages (multiple in case of request stream)
        for (ArgParse::ParsedElement *jsonInputParseTree : jsonInputs)
        {
            std::unique_ptr<google::protobuf::Message> message(f_factory.GetPrototype(f_messageDescriptor)->New());
            std::string jsonFileName = jsonInputParseTree->findFirstChild("JsonInputFile");

            std::ifstream file;
            std::istream * source = &std::cin;
            if(jsonFileName != "-")
            {
                source = &file;
                file.open(jsonFileName);
                if(source->fail())
                {
                    std::cerr << "Warning: Failed to read JSON file '" << jsonFileName << "': " << strerror(errno) << std::endl;
                    continue;
                }
            }
            std::stringstream buffer;
            buffer << source->rdbuf();
            std::string fileContent = buffer.str();
            auto status = google::protobuf::util::JsonStringToMessage(
                    google::protobuf::StringPiece(fileContent),
                    message.get(),
                    google::protobuf::util::JsonParseOptions());
            if(not status.ok())
            {
                std::cerr << "Warning: Failed to parse JSON file '" << jsonFileName << "': " << status.ToString() << std::endl;
                continue;
            }
            result.push_back(std::move(message));
        }
        return result;
}

}
