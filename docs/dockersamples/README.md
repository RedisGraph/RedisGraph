# docker samples

This folder containers sample docker files that show how we build redisgraph for a given operating system. These dockers make use of the [readies](https://github.com/RedisLabsModules/readies) submodule. To build, assuming you have Docker installed and running:

```docker build -f <filename> ../..``

The command above instructs docker to build using the associated dockerfile, as if rooted at the tip of this repository.
