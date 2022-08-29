# Copyright 2022 IBM Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Script for creating config file for gwhisper.
# Initially setting all settings to null

import json
from pathlib import Path # python 3.4 should i rather use os.path for compatibility with older versions?

home = Path.home()
gWhisperFolder = Path("/.cache/gwhisper") # put in .config/gWhsiperConfig.json
configPath= home / ".cache/gwhisper"
configFile = "testfile.json"
config={
    "configParameter":
      {
         "Ssl":None, 
         "SslSettings":
         {
            "ServerCertFile":None,
            "ClientCertFile":None,
            "ClientKeyFile":None
         },
   
         "DisableCache":None,

         "RpcTimeoutInMs":None,
         "connectTimeout":None
      }
}


if((configPath).exists()):
    if(not (configPath / configFile).exists()):
        filePath = configPath / configFile
        with open (filePath, 'w') as f:
            json.dump(config, f, ensure_ascii=False, indent=4)
    else:
        print(f'Config File already exists at ' + str(configPath / configFile))
else:
    print(f'The directory ' + str(configPath) + " can not be accessed")