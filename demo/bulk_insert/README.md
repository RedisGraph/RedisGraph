# redisgraph-bulk-loader
A Python utility for building RedisGraph databases from CSV inputs

## Installation
The bulk loader script's dependencies can be resolved using pip:
```
pip install --user -r requirements.txt
```

A Redis server with the [RedisGraph](https://github.com/RedisLabsModules/RedisGraph) module must be running. Installation instructions may be found at:
https://oss.redislabs.com/redisgraph/

## Usage
bulk_insert.py GRAPHNAME [OPTIONS]

| Flags   | Extended flags        |    Parameter                                                    |
|---------|-----------------------|-----------------------------------------------------------------|
|  -h     | --host TEXT           |    Redis server host (default: 127.0.0.1)                       |
|  -p     | --port INTEGER        |    Redis server port   (default: 6379)                          |
|  -a     | --password TEXT       |    Redis server password                                        |
|  -n     | --nodes TEXT          |    path to node csv file [required]                             |
|  -r     | --relations TEXT      |    path to relationship csv file                                |
|  -t     | --max-token-count INT |    max number of tokens sent in each Redis query (default 1024) |
|  -b     | --max-buffer-size INT |    max batch size (MBs) of each Redis query (default 4096)      |
|  -c     | --max-token-size INT  |    max size (MBs) of each token sent to Redis (default 500)     |


The only required arguments are the name to give the newly-created graph (which can appear anywhere) and at least one node CSV file.
The nodes and relationship flags should be specified once per input file.

The flags for `max-token-count`, `max-buffer-size`, and `max-token-size` should only be specified if the memory overhead of graph creation is too high. The bulk loader builds large graphs by sending binary tokens (each of which holds multiple nodes or relations) to Redis in batches. By lowering these limits from their defaults, the size of each transmission to Redis is lowered and fewer entities are held in memory, at the expense of a longer overall runtime.

```
python bulk_insert.py GRAPH_DEMO -n example/Person.csv -n example/Country.csv -r example/KNOWS.csv -r example/VISITED.csv
```
The label (for nodes) or relationship type (for relationships) is derived from the base name of the input CSV file. In this example, we'll construct two sets of nodes, labeled `Person` and `Country`, and two types of relationships - `KNOWS` and `VISITED`.

## Input constraints
### Node identifiers
- If both nodes and relations are being created, each node must be associated with a unique identifier.
- The identifier is the first column of each label CSV file. If this column's name starts with an underscore (`_`), the identifier is internal to the bulk loader operation and does not appear in the resulting graph. Otherwise, it is treated as a node property.
- Each identifier must be entirely unique across all label files.
- Source and destination nodes in relation CSV files should be referred to by their identifiers.
- The uniqueness restriction is lifted if only nodes are being created.

### Entity properties
- Property types do not need to be explicitly provided.
- Properties are not required to be exclusively composed of any type.
- The types currently supported by the bulk loader are:
    - `boolean`: either `true` or `false` (case-insensitive, not quote-interpolated).
    - `numeric`: an unquoted value that can be read as a floating-point or integer type.
    - `string`: any field that is either quote-interpolated or cannot be casted to a numeric or boolean type.
    - `NULL`: an empty field.

### Label file format:
- Each row must have the same number of fields.
- Leading and trailing whitespace is ignored.
- The first field of a label file will be the node identifier, as described in [Node Identifiers](#node-identifiers).
- All fields are property keys that will be associated with each node.

### Relationship files
- Each row must have the same number of fields.
- Leading and trailing whitespace is ignored.
- The first two fields of each row are the source and destination node identifiers. The names of these fields in the header do not matter.
- If the file has more than 2 fields, all subsequent fields are relationship properties that adhere to the same rules as node properties.
- Described relationships are always considered to be directed (source->destination).
