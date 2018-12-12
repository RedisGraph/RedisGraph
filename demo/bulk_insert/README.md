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

| Flags   | Extended flags        |    Parameter                                 |
|---------|-----------------------|----------------------------------------------|
|  -h     | --host TEXT           |    Redis server host (default: 127.0.0.1)    |
|  -p     | --port INTEGER        |    Redis server port   (default: 6379)       |
|  -a     | --password TEXT       |    Redis server password                     |
|  -c     | --ssl_certfile TEXT   |    path to certfile for SSL connection       |
|  -k     | --ssl_keyfile TEXT    |    path to keyfile for SSL connection        |
|  -n     | --nodes TEXT          |    path to node csv file  [required]         |
|  -r     | --relationships TEXT  |    path to relationship csv file             |

The only required arguments are the name to give the newly-created graph (which can appear anywhere) and at least one node CSV file.
The nodes and relationship flags should be specified once per input file.

```
python bulk_insert.py GRAPH_DEMO -n example/Person.csv -n example/Country.csv -r example/KNOWS.csv -r example/VISITED.csv
```
The label (for nodes) or relationship type (for relationships) is derived from the base name of the input CSV file. In this query, we'll construct two sets of nodes, labeled `Person` and `Country`, and two types of relationships - `KNOWS` and `VISITED`.

## Input constraints
### Node identifiers
- If both nodes and relations are being created, each node must be associated with a unique identifier.
- The identifier is the first column of each label CSV file. If this column's name starts with an underscore (`_`), the identifier is internal to the bulk loader operation and does not appear in the resulting graph. Otherwise, it is treated as a node property.
- Source and destination nodes in relation CSV files should be referred to by their identifiers.
- The name of the identifier columns otherwise do not matter, so long as no node has a duplicated value.
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
- With the possible exception of the first, each field in the header is a property key. The value in that position for each node is the property associated with that key.

### Relationship files
- Each row must have the same number of fields.
- Leading and trailing whitespace is ignored.
- The first two fields of each row are the source and destination node identifiers. The names of these fields in the header do not matter.
- If the file has more than 2 fields, all subsequent fields are relationship properties that adhere to the same rules as node properties.
- Described relationships are always considered to be directed (source->destination).
