[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
<a href='https://coveralls.io/github/innostory/avm?branch=master'><img src='https://coveralls.io/repos/github/innostory/avm/badge.svg?branch=master' alt='Coverage Status' /></a>

# AVM - Embeddable concurrent oriented Virtual Machine

Welcome. Since you are here with us, chance you may already using and fall in 
love with scripting language, such as **Lua**. We're on the same boat with you, 
yes we love Lua too. However, there are things that I'm not happy with Lua, 
that wasn't easy to just forking and make improvement. Therefore, `AVM` begin. 
In short, what we get with `AVM`:

- Mostly ANSI-C.
- Simplicity from Lua.
- First-class hot code reloading.
- Scalability of Erlang.
- Concurrent oriented design.
- Hackable runtime.
- Debug services.

What is out of the scope?

- Scripting language.

*Now, `AVM` is still under heavy development.*

[**API Documentation**](https://innostory.github.io/avm/)

## Inspiration

- [Erlang actor model.](http://www.brianstorti.com/the-actor-model/)
- [No frills introduction to Lua.](http://luaforge.net/docman/83/98/ANoFrillsIntroToLua51VMInstructions.pdf)
- [The BEAM book.](https://github.com/happi/theBeamBook)

## Getting Started

Using cmake to compile with Makefile, Visual Studio or XCode etc.

```sh
git clone --recursive https://github.com/innostory/avm.git
cd any-vm && mkdir .build && cd .build
cmake ../ -G "Unix Makefiles" -DTASK_BACKEND=gccasm
make && ctest
```

On windows, just change the `TASK_BACKEND` to `fiber`, it's only one supported.

*Now only lot of unit test to play around :)*

## What works currenty

- Single, cooperative scheduler.
- Lightweight real-time actor.
- Reloadable loader.
- Bytecode assembler.
- Bytecode dispatcher.
- Copying GC.

## Coming soon

- Debug bridge.

## Contributing

- TODO, feel free to contact me if you have any question.

## Contact

- **Giang Nv** - giangnv.is@gmail.com

## Nonsense Badgets
[![Build Status](https://travis-ci.org/innostory/avm.svg?branch=master)](https://travis-ci.org/innostory/avm) 
[![Build status](https://ci.appveyor.com/api/projects/status/0t5f79e4x9akyi0e?svg=true)](https://ci.appveyor.com/project/innostory/avm)
<a href="https://scan.coverity.com/projects/innostory-avm">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/12663/badge.svg"/>
</a>
