---
title: "Commands"
linkTitle: "Commands"
weight: 1
description: >
    Commands Overview
---

## Overview

### RedisGraph Features

RedisGraph exposes graph database functionality within Redis using the [openCypher](https://opencypher.org/) query language. Its basic commands accept openCypher queries, while additional commands are exposed for configuration or metadata retrieval.

### RedisGraph API

Command details can be retrieved by filtering for the [module](/commands/?group=redisgraph) or for a specific command, e.g. [`QUERY`](/commands/?group=redisgraph&name=graph.query).
The details include the syntax for the commands, where:

*   Command and subcommand names are in uppercase, for example `GRAPH.CONFIG` or `SET`
*   Optional arguments are enclosed in square brackets, for example `[timeout]`
*   Additional optional arguments are indicated by an ellipsis: `...`

Most commands require a graph key's name as their first argument.
