#Copyright 2019 IBM Corporation
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

function _gwhisper_getCompletions;
  set -l commandLineUntilCursor (commandline -c)
  set -l commandLineTokenized (commandline -o)
  set -l commandName (commandline -o)[1]

  set -l args (string sub --start (math (string length $commandName) + 1) $commandLineUntilCursor)

  set -l cmd "$commandName --complete=fish \"$args\""
  #echo $cmd > file
  set -l completions (eval $cmd)
  #echo "NEW" >> file
  for completion in $completions
    echo $completion
    #echo $completion >> file
  end
end

complete -c gwhisper -e
complete -f -c gwhisper -a '(_gwhisper_getCompletions)'
