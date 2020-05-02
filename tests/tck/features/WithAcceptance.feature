#
# Copyright (c) 2015-2019 "Neo Technology,"
# Network Engine for Objects in Lund AB [http://neotechnology.com]
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
#
# Attribution Notice under the terms of the Apache License 2.0
#
# This work was created by the collective efforts of the openCypher community.
# Without limiting the terms of Section 6, any Derivative Work that is not
# approved by the public consensus process of the openCypher Implementers Group
# should not be described as “Cypher” (and Cypher® is a registered trademark of
# Neo4j Inc.) or as "openCypher". Extensions by implementers or prototypes or
# proposals for change that have been documented or implemented should only be
# described as "implementation extensions to Cypher" or as "proposed changes to
# Cypher that are not yet approved by the openCypher community".
#

#encoding: utf-8

Feature: WithAcceptance

    Scenario: Passing on pattern nodes
        Given an empty graph
        And having executed:
            """
            CREATE (:A)-[:REL]->(:B)
            """
        When executing query:
            """
            MATCH (a:A)
            WITH a
            MATCH (a)-->(b)
            RETURN *
            """
        Then the result should be:
            | a    | b    |
            | (:A) | (:B) |
        And no side effects

    Scenario: ORDER BY and LIMIT can be used
        Given an empty graph
        And having executed:
            """
            CREATE (a:A), (), (), (),
            (a)-[:REL]->()
            """
        When executing query:
            """
            MATCH (a:A)
            WITH a
            ORDER BY a.name
            LIMIT 1
            MATCH (a)-->(b)
            RETURN a
            """
        Then the result should be:
            | a    |
            | (:A) |
        And no side effects

    Scenario: No dependencies between the query parts
        Given an empty graph
        And having executed:
            """
            CREATE (:A), (:B)
            """
        When executing query:
            """
            MATCH (a)
            WITH a
            MATCH (b)
            RETURN a, b
            """
        Then the result should be:
            | a    | b    |
            | (:A) | (:A) |
            | (:A) | (:B) |
            | (:B) | (:A) |
            | (:B) | (:B) |
        And no side effects

    Scenario: Aliasing
        Given an empty graph
        And having executed:
            """
            CREATE (:Begin {num: 42}),
            (:End {num: 42}),
            (:End {num: 3})
            """
        When executing query:
            """
            MATCH (a:Begin)
            WITH a.num AS property
            MATCH (b:End)
            WHERE property = b.num
            RETURN b
            """
        Then the result should be:
            | b                |
            | (:End {num: 42}) |
        And no side effects

@crash
    @skip
    Scenario: Handle dependencies across WITH
        Given an empty graph
        And having executed:
            """
            CREATE (a:End {num: 42, id: 0}),
            (:End {num: 3}),
            (:Begin {num: a.id})
            """
        When executing query:
            """
            MATCH (a:Begin)
            WITH a.num AS property
            LIMIT 1
            MATCH (b)
            WHERE b.id = property
            RETURN b
            """
        Then the result should be:
            | b                       |
            | (:End {num: 42, id: 0}) |
        And no side effects

@crash
    @skip
    Scenario: Handle dependencies across WITH with SKIP
        Given an empty graph
        And having executed:
            """
            CREATE (a {name: 'A', num: 0, id: 0}),
            ({name: 'B', num: a.id, id: 1}),
            ({name: 'C', num: 0, id: 2})
            """
        When executing query:
            """
            MATCH (a)
            WITH a.name AS property, a.num AS idToUse
            ORDER BY property
            SKIP 1
            MATCH (b)
            WHERE b.id = idToUse
            RETURN DISTINCT b
            """
        Then the result should be:
            | b                            |
            | ({name: 'A', num: 0, id: 0}) |
        And no side effects

    Scenario: WHERE after WITH should filter results
        Given an empty graph
        And having executed:
            """
            CREATE ({name: 'A'}),
            ({name: 'B'}),
            ({name: 'C'})
            """
        When executing query:
            """
            MATCH (a)
            WITH a
            WHERE a.name = 'B'
            RETURN a
            """
        Then the result should be:
            | a             |
            | ({name: 'B'}) |
        And no side effects

    Scenario: WHERE after WITH can filter on top of an aggregation
        Given an empty graph
        And having executed:
            """
            CREATE (a {name: 'A'}),
            (b {name: 'B'})
            CREATE (a)-[:REL]->(),
            (a)-[:REL]->(),
            (a)-[:REL]->(),
            (b)-[:REL]->()
            """
        When executing query:
            """
            MATCH (a)-->()
            WITH a, count(*) AS relCount
            WHERE relCount > 1
            RETURN a
            """
        Then the result should be:
            | a             |
            | ({name: 'A'}) |
        And no side effects

    Scenario: ORDER BY on an aggregating key
        Given an empty graph
        And having executed:
            """
            CREATE ({name: 'A'}),
            ({name: 'A'}),
            ({name: 'B'})
            """
        When executing query:
            """
            MATCH (a)
            WITH a.name AS bars, count(*) AS relCount
            ORDER BY a.name
            RETURN *
            """
        Then the result should be:
            | bars | relCount |
            | 'A'  | 2        |
            | 'B'  | 1        |
        And no side effects

    Scenario: ORDER BY a DISTINCT column
        Given an empty graph
        And having executed:
            """
            CREATE ({name: 'A'}),
            ({name: 'A'}),
            ({name: 'B'})
            """
        When executing query:
            """
            MATCH (a)
            WITH DISTINCT a.name AS bars
            ORDER BY a.name
            RETURN *
            """
        Then the result should be:
            | bars |
            | 'A'  |
            | 'B'  |
        And no side effects

    Scenario: WHERE on a DISTINCT column
        Given an empty graph
        And having executed:
            """
            CREATE ({name2: 'A'}),
            ({name2: 'A'}),
            ({name2: 'B'})
            """
        When executing query:
            """
            MATCH (a)
            WITH DISTINCT a.name2 AS bars
            WHERE a.name2 = 'B'
            RETURN *
            """
        Then the result should be:
            | bars |
            | 'B'  |
        And no side effects

    Scenario: A simple pattern with one bound endpoint
        Given an empty graph
        And having executed:
            """
            CREATE (:A)-[:REL]->(:B)
            """
        When executing query:
            """
            MATCH (a:A)-[r:REL]->(b:B)
            WITH a AS b, b AS tmp, r AS r
            WITH b AS a, r
            LIMIT 1
            MATCH (a)-[r]->(b)
            RETURN a, r, b
            """
        Then the result should be:
            | a    | r      | b    |
            | (:A) | [:REL] | (:B) |
        And no side effects

    Scenario: Null handling
        Given an empty graph
        When executing query:
            """
            OPTIONAL MATCH (a:Start)
            WITH a
            MATCH (a)-->(b)
            RETURN *
            """
        Then the result should be:
            | a | b |
        And no side effects

    @skip
    Scenario: Nested maps
        Given an empty graph
        When executing query:
            """
            WITH {name: {name2: 'baz'}} AS nestedMap
            RETURN nestedMap.name.name2
            """
        Then the result should be:
            | nestedMap.name.name2 |
            | 'baz'                |
        And no side effects

    Scenario: Connected components succeeding WITH
        Given an empty graph
        And having executed:
            """
            CREATE (:A)-[:REL]->(:X)
            CREATE (:B)
            """
        When executing query:
            """
            MATCH (n:A)
            WITH n
            LIMIT 1
            MATCH (m:B), (n)-->(x:X)
            RETURN *
            """
        Then the result should be:
            | m    | n    | x    |
            | (:B) | (:A) | (:X) |
        And no side effects

    Scenario: Single WITH using a predicate and aggregation
        Given an empty graph
        And having executed:
            """
            CREATE ({num: 43}), ({num: 42})
            """
        When executing query:
            """
            MATCH (n)
            WITH n
            WHERE n.num = 42
            RETURN count(*)
            """
        Then the result should be:
            | count(*) |
            | 1        |
        And no side effects

    Scenario: Multiple WITHs using a predicate and aggregation
        Given an empty graph
        And having executed:
            """
            CREATE (a {name: 'David'}),
            (b {name: 'Other'}),
            (c {name: 'NotOther'}),
            (d {name: 'NotOther2'}),
            (a)-[:REL]->(b),
            (a)-[:REL]->(c),
            (a)-[:REL]->(d),
            (b)-[:REL]->(),
            (b)-[:REL]->(),
            (c)-[:REL]->(),
            (c)-[:REL]->(),
            (d)-[:REL]->()
            """
        When executing query:
            """
            MATCH (david {name: 'David'})--(otherPerson)-->()
            WITH otherPerson, count(*) AS foaf
            WHERE foaf > 1
            WITH otherPerson
            WHERE otherPerson.name <> 'NotOther'
            RETURN count(*)
            """
        Then the result should be:
            | count(*) |
            | 1        |
        And no side effects
