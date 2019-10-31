# Contributing
## Contributing In General
Our project welcomes external contributions. If you have an itch, please feel
free to scratch it.

To contribute code or documentation, please submit a [pull request](https://github.com/ibm/gWhisper/pulls).

A good way to familiarize yourself with the codebase and contribution process is
to look for and tackle low-hanging fruit in the [issue tracker](https://github.com/ibm/gWhisper/issues).
Before embarking on a more ambitious contribution, please quickly [get in touch](#communication) with us and have a look at the [PROJECT_SCOPE.md](PROJECT_SCOPE.md).

**Note: We appreciate your effort, and want to avoid a situation where a contribution
requires extensive rework (by you or by us), sits in backlog for a long time, or
cannot be accepted at all!**

### Proposing new features

If you would like to implement a new feature, please [raise an issue](https://github.com/ibm/gWhisper/issues)
before sending a pull request so the feature can be discussed. This is to avoid
you wasting your valuable time working on a feature that the project developers
are not interested in accepting into the code base.

### Fixing bugs

If you would like to fix a bug, please [raise an issue](https://github.com/ibm/gWhisper/issues) before sending a
pull request so it can be tracked.

### Merge approval

The project maintainers use LGTM (Looks Good To Me) in comments on the code
review to indicate acceptance. A change requires LGTMs from one of the
maintainers of each component affected.

For a list of the maintainers, see the [MAINTAINERS.md](MAINTAINERS.md) page.

## Communication
For feature requests, bug reports or technical discussions, please use the [issue tracker](https://github.com/ibm/gWhisper/issues).

Depending on the need, we might create a channel on matrix.org (see [riot.im](https://about.riot.im/)) or on slack for general questions. In the meantime please contact one of the [maintainers](MAINTAINERS.md) directly for general questions or feedback.

## Testing
As we currently do not have a CI and test framework set up, but plan to do so in the future,
we encourage contributors to ensure that existing unit-tests pass by running `make test` before
submitting a pull request. Also it is highly appreciated if you implement unit-tests
for new code.

## Coding style guidelines
Existing code might not always follow the guidelines listed below. However please
write new code to correspond to these guidelines and feel free to re-factor old
code to be compliant.

### Directory structure
We are trying to stay close to _Canonical Project Structure_ as described [in this proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1204r0.html).
All new code should follow the directory and naming conventions described here.

### Indentation
This project is indented using spaces. Tabs are not allowed. One level of indentation corresponds to 4 spaces.
```
void myMethod(bool f_condition)
{
    ...
    if(f_condition)
    {
        if(otherCondition)
        {
            doSomething();
        }
    }
}
```

### Naming conventions
- Classes:
    UpperCamelCase
- Methods:
    lowerCamelCase
- Variables:
    lowerCamelCase

    Scope for variables is indicated by prefixes:
```
    g_      -> global scope
    m_      -> object member variable
    f_      -> variable given as method/function argument
    f_out   -> variable given as method/function argument, out parameter.
             I.e. caller expects function to write/modify data referenced
             to by this variable.
```

### Scoping brackets
The curly bracket at scope start and scope end should have the same indentation level and immediately be followed by a new-line:
```
if(condition)
{
    something();
}
```

### Source code documentation
For documenting code, please use the [doxygen style](http://www.doxygen.nl/manual/docblocks.html) method 3.
For example to document a method:
```
/// Method for doing my stuff.
/// This method is doing my stuff by doing ....
myMethod()
...
```

## Legal

Each source file must include a license header for the Apache
Software License 2.0:

```
Copyright [yyyy] [name of copyright owner]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

We have tried to make it as easy as possible to make contributions. This
applies to how we handle the legal aspects of contribution. We use the
same approach - the [Developer's Certificate of Origin 1.1 (DCO)](DCO1.1.txt) - that the Linux® Kernel [community](https://elinux.org/Developer_Certificate_Of_Origin)
uses to manage code contributions.

We simply ask that when submitting a patch for review, the developer
must include a sign-off statement in the commit message.

Here is an example Signed-off-by line, which indicates that the
submitter accepts the DCO:

```
Signed-off-by: John Doe <john.doe@example.com>
```

You can include this automatically when you commit a change to your
local git repository using the following command:

```
git commit -s
```

