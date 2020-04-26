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

Feature: MatchAcceptance2

  Scenario: Do not return non-existent nodes
    Given an empty graph
    When executing query:
      """
      MATCH (n)
      RETURN n
      """
    Then the result should be:
      | n |
    And no side effects

  Scenario: Do not return non-existent relationships
    Given an empty graph
    When executing query:
      """
      MATCH ()-[r]->()
      RETURN r
      """
    Then the result should be:
      | r |
    And no side effects

  Scenario: Do not fail when evaluating predicates with illegal operations if the AND'ed predicate evaluates to false
    Given an empty graph
    And having executed:
      """
      CREATE (root:Root {name: 'x'}),
             (child1:TextNode {var: 'text'}),
             (child2:IntNode {var: 0})
      CREATE (root)-[:T]->(child1),
             (root)-[:T]->(child2)
      """
    When executing query:
      """
      MATCH (:Root {name: 'x'})-->(i:TextNode)
      WHERE i.var > 'te'
      RETURN i
      """
    Then the result should be:
      | i                         |
      | (:TextNode {var: 'text'}) |
    And no side effects

  Scenario: Do not fail when evaluating predicates with illegal operations if the OR'd predicate evaluates to true
    Given an empty graph
    And having executed:
      """
      CREATE (root:Root {name: 'x'}),
             (child1:TextNode {var: 'text'}),
             (child2:IntNode {var: 0})
      CREATE (root)-[:T]->(child1),
             (root)-[:T]->(child2)
      """
    When executing query:
      """
      MATCH (:Root {name: 'x'})-->(i)
      WHERE exists(i.var) OR i.var > 'te'
      RETURN i
      """
    Then the result should be:
      | i                         |
      | (:TextNode {var: 'text'}) |
      | (:IntNode {var: 0})       |
    And no side effects

  Scenario: Aggregation with named paths
    Given an empty graph
    And having executed:
      """
      CREATE (n1 {num: 1}), (n2 {num: 2}),
             (n3 {num: 3}), (n4 {num: 4})
      CREATE (n1)-[:T]->(n2),
             (n3)-[:T]->(n4)
      """
    When executing query:
      """
      MATCH p = ()-[*]->()
      WITH count(*) AS count, p AS p
      WITH nodes(p) AS nodes
      RETURN *
      """
    Then the result should be:
      | nodes                    |
      | [({num: 1}), ({num: 2})] |
      | [({num: 3}), ({num: 4})] |
    And no side effects

  Scenario: Zero-length variable length pattern in the middle of the pattern
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}),
             (c {name: 'C'}), ({name: 'D'}),
             ({name: 'E'})
      CREATE (a)-[:CONTAINS]->(b),
             (b)-[:FRIEND]->(c)
      """
    When executing query:
      """
      MATCH (a {name: 'A'})-[:CONTAINS*0..1]->(b)-[:FRIEND*0..1]->(c)
      RETURN a, b, c
      """
    Then the result should be:
      | a             | b             | c             |
      | ({name: 'A'}) | ({name: 'A'}) | ({name: 'A'}) |
      | ({name: 'A'}) | ({name: 'B'}) | ({name: 'B'}) |
      | ({name: 'A'}) | ({name: 'B'}) | ({name: 'C'}) |
    And no side effects

  Scenario: Simple variable length pattern
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}),
             (c {name: 'C'}), (d {name: 'D'})
      CREATE (a)-[:CONTAINS]->(b),
             (b)-[:CONTAINS]->(c),
             (c)-[:CONTAINS]->(d)
      """
    When executing query:
      """
      MATCH (a {name: 'A'})-[*]->(x)
      RETURN x
      """
    Then the result should be:
      | x             |
      | ({name: 'B'}) |
      | ({name: 'C'}) |
      | ({name: 'D'}) |
    And no side effects

  Scenario: Variable length relationship without lower bound
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}),
             (c {name: 'C'})
      CREATE (a)-[:KNOWS]->(b),
             (b)-[:KNOWS]->(c)
      """
    When executing query:
      """
      MATCH p = ({name: 'A'})-[:KNOWS*..2]->()
      RETURN p
      """
    Then the result should be:
      | p                                                               |
      | <({name: 'A'})-[:KNOWS]->({name: 'B'})>                         |
      | <({name: 'A'})-[:KNOWS]->({name: 'B'})-[:KNOWS]->({name: 'C'})> |
    And no side effects

  Scenario: Variable length relationship without bounds
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}),
             (c {name: 'C'})
      CREATE (a)-[:KNOWS]->(b),
             (b)-[:KNOWS]->(c)
      """
    When executing query:
      """
      MATCH p = ({name: 'A'})-[:KNOWS*..]->()
      RETURN p
      """
    Then the result should be:
      | p                                                               |
      | <({name: 'A'})-[:KNOWS]->({name: 'B'})>                         |
      | <({name: 'A'})-[:KNOWS]->({name: 'B'})-[:KNOWS]->({name: 'C'})> |
    And no side effects

  Scenario: Returning bound nodes that are not part of the pattern
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}),
             (c {name: 'C'})
      CREATE (a)-[:KNOWS]->(b)
      """
    When executing query:
      """
      MATCH (a {name: 'A'}), (c {name: 'C'})
      MATCH (a)-->(b)
      RETURN a, b, c
      """
    Then the result should be:
      | a             | b             | c             |
      | ({name: 'A'}) | ({name: 'B'}) | ({name: 'C'}) |
    And no side effects

  Scenario: Two bound nodes pointing to the same node
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}),
             (x1 {name: 'x1'}), (x2 {name: 'x2'})
      CREATE (a)-[:KNOWS]->(x1),
             (a)-[:KNOWS]->(x2),
             (b)-[:KNOWS]->(x1),
             (b)-[:KNOWS]->(x2)
      """
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'})
      MATCH (a)-->(x)<-->(b)
      RETURN x
      """
    Then the result should be:
      | x              |
      | ({name: 'x1'}) |
      | ({name: 'x2'}) |
    And no side effects

  Scenario: Three bound nodes pointing to the same node
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}), (c {name: 'C'}),
             (x1 {name: 'x1'}), (x2 {name: 'x2'})
      CREATE (a)-[:KNOWS]->(x1),
             (a)-[:KNOWS]->(x2),
             (b)-[:KNOWS]->(x1),
             (b)-[:KNOWS]->(x2),
             (c)-[:KNOWS]->(x1),
             (c)-[:KNOWS]->(x2)
      """
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'}), (c {name: 'C'})
      MATCH (a)-->(x), (b)-->(x), (c)-->(x)
      RETURN x
      """
    Then the result should be:
      | x              |
      | ({name: 'x1'}) |
      | ({name: 'x2'}) |
    And no side effects

  Scenario: Three bound nodes pointing to the same node with extra connections
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'a'}), (b {name: 'b'}), (c {name: 'c'}),
             (d {name: 'd'}), (e {name: 'e'}), (f {name: 'f'}),
             (g {name: 'g'}), (h {name: 'h'}), (i {name: 'i'}),
             (j {name: 'j'}), (k {name: 'k'})
      CREATE (a)-[:KNOWS]->(d),
             (a)-[:KNOWS]->(e),
             (a)-[:KNOWS]->(f),
             (a)-[:KNOWS]->(g),
             (a)-[:KNOWS]->(i),
             (b)-[:KNOWS]->(d),
             (b)-[:KNOWS]->(e),
             (b)-[:KNOWS]->(f),
             (b)-[:KNOWS]->(h),
             (b)-[:KNOWS]->(k),
             (c)-[:KNOWS]->(d),
             (c)-[:KNOWS]->(e),
             (c)-[:KNOWS]->(h),
             (c)-[:KNOWS]->(g),
             (c)-[:KNOWS]->(j)
      """
    When executing query:
      """
      MATCH (a {name: 'a'}), (b {name: 'b'}), (c {name: 'c'})
      MATCH (a)-->(x), (b)-->(x), (c)-->(x)
      RETURN x
      """
    Then the result should be:
      | x             |
      | ({name: 'd'}) |
      | ({name: 'e'}) |
    And no side effects

  Scenario: MATCH with OPTIONAL MATCH in longer pattern
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}), (c {name: 'C'})
      CREATE (a)-[:KNOWS]->(b),
             (b)-[:KNOWS]->(c)
      """
    When executing query:
      """
      MATCH (a {name: 'A'})
      OPTIONAL MATCH (a)-[:KNOWS]->()-[:KNOWS]->(foo)
      RETURN foo
      """
    Then the result should be:
      | foo           |
      | ({name: 'C'}) |
    And no side effects

  Scenario: Optionally matching named paths
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}), (c {name: 'C'})
      CREATE (a)-[:X]->(b)
      """
    When executing query:
      """
      MATCH (a {name: 'A'}), (x)
      WHERE x.name IN ['B', 'C']
      OPTIONAL MATCH p = (a)-->(x)
      RETURN x, p
      """
    Then the result should be:
      | x             | p                                   |
      | ({name: 'B'}) | <({name: 'A'})-[:X]->({name: 'B'})> |
      | ({name: 'C'}) | null                                |
    And no side effects

  Scenario: Optionally matching named paths with single and variable length patterns
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'})
      CREATE (a)-[:X]->(b)
      """
    When executing query:
      """
      MATCH (a {name: 'A'})
      OPTIONAL MATCH p = (a)-->(b)-[*]->(c)
      RETURN p
      """
    Then the result should be:
      | p    |
      | null |
    And no side effects

@skip
  Scenario: Optionally matching named paths with variable length patterns
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b {name: 'B'}), (c {name: 'C'})
      CREATE (a)-[:X]->(b)
      """
    When executing query:
      """
      MATCH (a {name: 'A'}), (x)
      WHERE x.name IN ['B', 'C']
      OPTIONAL MATCH p = (a)-[r*]->(x)
      RETURN r, x, p
      """
    Then the result should be:
      | r      | x             | p                                   |
      | [[:X]] | ({name: 'B'}) | <({name: 'A'})-[:X]->({name: 'B'})> |
      | null   | ({name: 'C'}) | null                                |
    And no side effects

@skip
  Scenario: Matching variable length patterns from a bound node
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b), (c)
      CREATE (a)-[:X]->(b),
             (b)-[:Y]->(c)
      """
    When executing query:
      """
      MATCH (a:A)
      MATCH (a)-[r*2]->()
      RETURN r
      """
    Then the result should be (ignoring element order for lists):
      | r            |
      | [[:X], [:Y]] |
    And no side effects

@skip
  Scenario: Excluding connected nodes
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B {id: 1}), (:B {id: 2})
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH (a:A), (other:B)
      OPTIONAL MATCH (a)-[r]->(other)
      WITH other WHERE r IS NULL
      RETURN other
      """
    Then the result should be:
      | other        |
      | (:B {id: 2}) |
    And no side effects

  Scenario: Do not fail when predicates on optionally matched and missed nodes are invalid
    Given an empty graph
    And having executed:
      """
      CREATE (a), (b {name: 'Mark'})
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH (n)-->(x0)
      OPTIONAL MATCH (x0)-->(x1)
      WHERE x1.name = 'bar'
      RETURN x0.name
      """
    Then the result should be:
      | x0.name |
      | 'Mark'  |
    And no side effects

@skip
  Scenario: MATCH and OPTIONAL MATCH on same pattern
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'A'}), (b:B {name: 'B'}), (c:C {name: 'C'})
      CREATE (a)-[:T]->(b),
             (a)-[:T]->(c)
      """
    When executing query:
      """
      MATCH (a)-->(b)
      WHERE b:B
      OPTIONAL MATCH (a)-->(c)
      WHERE c:C
      RETURN a.name
      """
    Then the result should be:
      | a.name |
      | 'A'    |
    And no side effects

@skip
  Scenario: Matching using an undirected pattern
    Given an empty graph
    And having executed:
      """
      CREATE (:A {id: 0})-[:ADMIN]->(:B {id: 1})
      """
    When executing query:
      """
      MATCH (a)-[:ADMIN]-(b)
      WHERE a:A
      RETURN a.id, b.id
      """
    Then the result should be:
      | a.id | b.id |
      | 0    | 1    |
    And no side effects

  Scenario: Matching all nodes
    Given an empty graph
    And having executed:
      """
      CREATE (:A), (:B)
      """
    When executing query:
      """
      MATCH (n)
      RETURN n
      """
    Then the result should be:
      | n    |
      | (:A) |
      | (:B) |
    And no side effects

  Scenario: Comparing nodes for equality
    Given an empty graph
    And having executed:
      """
      CREATE (:A), (:B)
      """
    When executing query:
      """
      MATCH (a), (b)
      WHERE a <> b
      RETURN a, b
      """
    Then the result should be:
      | a    | b    |
      | (:A) | (:B) |
      | (:B) | (:A) |
    And no side effects

  Scenario: Matching using self-referencing pattern returns no result
    Given an empty graph
    And having executed:
      """
      CREATE (a), (b), (c)
      CREATE (a)-[:T]->(b),
             (b)-[:T]->(c)
      """
    When executing query:
      """
      MATCH (a)-->(b), (b)-->(b)
      RETURN b
      """
    Then the result should be:
      | b |
    And no side effects

  Scenario: Variable length relationship in OPTIONAL MATCH
    Given an empty graph
    And having executed:
      """
      CREATE (:A), (:B)
      """
    When executing query:
      """
      MATCH (a:A), (b:B)
      OPTIONAL MATCH (a)-[r*]-(b)
      WHERE r IS NULL
        AND a <> b
      RETURN b
      """
    Then the result should be:
      | b    |
      | (:B) |
    And no side effects

  Scenario: Matching using relationship predicate with multiples of the same type
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH (a)-[:T|:T]->(b)
      RETURN b
      """
    Then the result should be:
      | b    |
      | (:B) |
    And no side effects

  Scenario: ORDER BY with LIMIT
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (n1 {num: 1}), (n2 {num: 2}),
             (m1), (m2)
      CREATE (a)-[:T]->(n1),
             (n1)-[:T]->(m1),
             (a)-[:T]->(n2),
             (n2)-[:T]->(m2)
      """
    When executing query:
      """
      MATCH (a:A)-->(n)-->(m)
      RETURN n.num, count(*)
        ORDER BY n.num
        LIMIT 1000
      """
    Then the result should be, in order:
      | n.num | count(*) |
      | 1     | 1        |
      | 2     | 1        |
    And no side effects

  Scenario: Simple node property predicate
    Given an empty graph
    And having executed:
      """
      CREATE ({name: 'bar'})
      """
    When executing query:
      """
      MATCH (n)
      WHERE n.name = 'bar'
      RETURN n
      """
    Then the result should be:
      | n               |
      | ({name: 'bar'}) |
    And no side effects

  Scenario: Handling direction of named paths
    Given an empty graph
    And having executed:
      """
      CREATE (a:A)-[:T]->(b:B)
      """
    When executing query:
      """
      MATCH p = (b)<--(a)
      RETURN p
      """
    Then the result should be:
      | p                 |
      | <(:B)<-[:T]-(:A)> |
    And no side effects

  Scenario: Simple OPTIONAL MATCH on empty graph
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (n)
      RETURN n
      """
    Then the result should be:
      | n    |
      | null |
    And no side effects

  Scenario: OPTIONAL MATCH with previously bound nodes
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      OPTIONAL MATCH (n)-[:NOT_EXIST]->(x)
      RETURN n, x
      """
    Then the result should be:
      | n  | x    |
      | () | null |
    And no side effects

  Scenario: `collect()` filtering nulls
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      OPTIONAL MATCH (n)-[:NOT_EXIST]->(x)
      RETURN n, collect(x)
      """
    Then the result should be:
      | n  | collect(x) |
      | () | []         |
    And no side effects

@skip
  Scenario: Multiple anonymous nodes in a pattern
    Given an empty graph
    And having executed:
      """
      CREATE (:A)
      """
    When executing query:
      """
      MATCH (a)<--()<--(b)-->()-->(c)
      WHERE a:A
      RETURN c
      """
    Then the result should be:
      | c |
    And no side effects

  Scenario: Matching a relationship pattern using a label predicate
    Given an empty graph
    And having executed:
      """
      CREATE (a), (b1:Foo), (b2)
      CREATE (a)-[:T]->(b1),
             (a)-[:T]->(b2)
      """
    When executing query:
      """
      MATCH (a)-->(b:Foo)
      RETURN b
      """
    Then the result should be:
      | b      |
      | (:Foo) |
    And no side effects

  Scenario: Matching a relationship pattern using a label predicate on both sides
    Given an empty graph
    And having executed:
      """
      CREATE (:A)-[:T1]->(:B),
             (:B)-[:T2]->(:A),
             (:B)-[:T3]->(:B),
             (:A)-[:T4]->(:A)
      """
    When executing query:
      """
      MATCH (:A)-[r]->(:B)
      RETURN r
      """
    Then the result should be:
      | r     |
      | [:T1] |
    And no side effects

@skip
  Scenario: Matching nodes using multiple labels
    Given an empty graph
    And having executed:
      """
      CREATE (:A:B:C), (:A:B), (:A:C), (:B:C),
             (:A), (:B), (:C)
      """
    When executing query:
      """
      MATCH (a:A:B:C)
      RETURN a
      """
    Then the result should be:
      | a        |
      | (:A:B:C) |
    And no side effects

@skip
  Scenario: Returning label predicate expression
    Given an empty graph
    And having executed:
      """
      CREATE (), (:Foo)
      """
    When executing query:
      """
      MATCH (n)
      RETURN (n:Foo)
      """
    Then the result should be:
      | (n:Foo) |
      | true    |
      | false   |
    And no side effects

  Scenario: Matching with many predicates and larger pattern
    Given an empty graph
    And having executed:
      """
      CREATE (advertiser {name: 'advertiser1', id: 0}),
             (thing {name: 'Color', id: 1}),
             (red {name: 'red'}),
             (p1 {name: 'product1'}),
             (p2 {name: 'product4'})
      CREATE (advertiser)-[:ADV_HAS_PRODUCT]->(p1),
             (advertiser)-[:ADV_HAS_PRODUCT]->(p2),
             (thing)-[:AA_HAS_VALUE]->(red),
             (p1)-[:AP_HAS_VALUE]->(red),
             (p2)-[:AP_HAS_VALUE]->(red)
      """
    And parameters are:
      | name | value |
      | 1    | 0     |
      | 2    | 1     |
    When executing query:
      """
      MATCH (advertiser)-[:ADV_HAS_PRODUCT]->(out)-[:AP_HAS_VALUE]->(red)<-[:AA_HAS_VALUE]-(a)
      WHERE advertiser.id = $1
        AND a.id = $2
        AND red.name = 'red'
        AND out.name = 'product1'
      RETURN out.name
      """
    Then the result should be:
      | out.name   |
      | 'product1' |
    And no side effects

  Scenario: Matching using a simple pattern with label predicate
    Given an empty graph
    And having executed:
      """
      CREATE (a:Person {name: 'Alice'}), (b:Person {name: 'Bob'}),
             (c), (d)
      CREATE (a)-[:T]->(c),
             (b)-[:T]->(d)
      """
    When executing query:
      """
      MATCH (n:Person)-->()
      WHERE n.name = 'Bob'
      RETURN n
      """
    Then the result should be:
      | n                       |
      | (:Person {name: 'Bob'}) |
    And no side effects

  Scenario: Matching disconnected patterns
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B), (c:C)
      CREATE (a)-[:T]->(b),
             (a)-[:T]->(c)
      """
    When executing query:
      """
      MATCH (a)-->(b)
      MATCH (c)-->(d)
      RETURN a, b, c, d
      """
    Then the result should be:
      | a    | b    | c    | d    |
      | (:A) | (:B) | (:A) | (:B) |
      | (:A) | (:B) | (:A) | (:C) |
      | (:A) | (:C) | (:A) | (:B) |
      | (:A) | (:C) | (:A) | (:C) |
    And no side effects

  Scenario: Non-optional matches should not return nulls
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B {id: 1}), (c:C {id: 2}), (d:D)
      CREATE (a)-[:T]->(b),
             (a)-[:T]->(c),
             (a)-[:T]->(d),
             (b)-[:T]->(c),
             (b)-[:T]->(d),
             (c)-[:T]->(d)
      """
    When executing query:
      """
      MATCH (a)--(b)--(c)--(d)--(a), (b)--(d)
      WHERE a.id = 1
        AND c.id = 2
      RETURN d
      """
    Then the result should be:
      | d    |
      | (:A) |
      | (:D) |
    And no side effects

  Scenario: Handling cyclic patterns
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'a'}), (b {name: 'b'}), (c {name: 'c'})
      CREATE (a)-[:A]->(b),
             (b)-[:B]->(a),
             (b)-[:B]->(c)
      """
    When executing query:
      """
      MATCH (a)-[:A]->()-[:B]->(a)
      RETURN a.name
      """
    Then the result should be:
      | a.name |
      | 'a'    |
    And no side effects

  Scenario: Handling cyclic patterns when separated into two parts
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'a'}), (b {name: 'b'}), (c {name: 'c'})
      CREATE (a)-[:A]->(b),
             (b)-[:B]->(a),
             (b)-[:B]->(c)
      """
    When executing query:
      """
      MATCH (a)-[:A]->(b), (b)-[:B]->(a)
      RETURN a.name
      """
    Then the result should be:
      | a.name |
      | 'a'    |
    And no side effects

@skip
  Scenario: Handling fixed-length variable length pattern
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T]->()
      """
    When executing query:
      """
      MATCH (a)-[r*1..1]->(b)
      RETURN r
      """
    Then the result should be:
      | r      |
      | [[:T]] |
    And no side effects

  Scenario: Matching from null nodes should return no results owing to finding no matches
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a)
      WITH a
      MATCH (a)-->(b)
      RETURN b
      """
    Then the result should be:
      | b |
    And no side effects

  Scenario: Matching from null nodes should return no results owing to matches being filtered out
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T]->()
      """
    When executing query:
      """
      OPTIONAL MATCH (a:TheLabel)
      WITH a
      MATCH (a)-->(b)
      RETURN b
      """
    Then the result should be:
      | b |
    And no side effects

  Scenario: Optionally matching from null nodes should return null
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a)
      WITH a
      OPTIONAL MATCH (a)-->(b)
      RETURN b
      """
    Then the result should be:
      | b    |
      | null |
    And no side effects

  Scenario: OPTIONAL MATCH returns null
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a)
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

  Scenario: Zero-length named path
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH p = (a)
      RETURN p
      """
    Then the result should be:
      | p    |
      | <()> |
    And no side effects

  Scenario: Variable-length named path
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH p = ()-[*0..]->()
      RETURN p
      """
    Then the result should be:
      | p    |
      | <()> |
    And no side effects

  Scenario: Matching with aggregation
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 42})
      """
    When executing query:
      """
      MATCH (n)
      RETURN n.num AS n, count(n) AS count
      """
    Then the result should be:
      | n  | count |
      | 42 | 1     |
    And no side effects

@skip
  Scenario: Matching using a relationship that is already bound
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T1]->(),
             ()-[:T2]->()
      """
    When executing query:
      """
      MATCH ()-[r1]->()
      WITH r1 AS r2
      MATCH ()-[r2]->()
      RETURN r2 AS rel
      """
    Then the result should be:
      | rel   |
      | [:T1] |
      | [:T2] |
    And no side effects

@skip
  Scenario: Matching using a relationship that is already bound, in conjunction with aggregation
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T1]->(),
             ()-[:T2]->()
      """
    When executing query:
      """
      MATCH ()-[r1]->()
      WITH r1 AS r2, count(*) AS c
        ORDER BY c
      MATCH ()-[r2]->()
      RETURN r2 AS rel
      """
    Then the result should be:
      | rel   |
      | [:T1] |
      | [:T2] |
    And no side effects

@crash
@skip
  Scenario: Matching using a relationship that is already bound, in conjunction with aggregation and ORDER BY
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T1 {id: 0}]->(),
             ()-[:T2 {id: 1}]->()
      """
    When executing query:
      """
      MATCH (a)-[r]->(b)
      WITH a, r, b, count(*) AS c
        ORDER BY c
      MATCH (a)-[r]->(b)
      RETURN r AS rel
        ORDER BY rel.id
      """
    Then the result should be, in order:
      | rel           |
      | [:T1 {id: 0}] |
      | [:T2 {id: 1}] |
    And no side effects

  Scenario: Matching with LIMIT and optionally matching using a relationship that is already bound
    Given an empty graph
    And having executed:
      """
      CREATE (:A)-[:T]->(:B)
      """
    When executing query:
      """
      MATCH ()-[r]->()
      WITH r
        LIMIT 1
      OPTIONAL MATCH (a2)-[r]->(b2)
      RETURN a2, r, b2
      """
    Then the result should be:
      | a2   | r    | b2   |
      | (:A) | [:T] | (:B) |
    And no side effects

  Scenario: Matching with LIMIT and optionally matching using a relationship and node that are both already bound
    Given an empty graph
    And having executed:
      """
      CREATE (:A)-[:T]->(:B)
      """
    When executing query:
      """
      MATCH (a1)-[r]->()
      WITH r, a1
        LIMIT 1
      OPTIONAL MATCH (a1)-[r]->(b2)
      RETURN a1, r, b2
      """
    Then the result should be:
      | a1   | r    | b2   |
      | (:A) | [:T] | (:B) |
    And no side effects

  Scenario: Matching with LIMIT, then matching again using a relationship and node that are both already bound along with an additional predicate
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T]->()
      """
    When executing query:
      """
      MATCH (a1)-[r]->()
      WITH r, a1
        LIMIT 1
      MATCH (a1:X)-[r]->(b2)
      RETURN a1, r, b2
      """
    Then the result should be:
      | a1 | r | b2 |
    And no side effects

@skip
  Scenario: Matching with LIMIT and predicates, then matching again using a relationship and node that are both already bound along with a duplicate predicate
    Given an empty graph
    And having executed:
      """
      CREATE (:X:Y)-[:T]->()
      """
    When executing query:
      """
      MATCH (a1:X:Y)-[r]->()
      WITH r, a1
        LIMIT 1
      MATCH (a1:Y)-[r]->(b2)
      RETURN a1, r, b2
      """
    Then the result should be:
      | a1     | r    | b2 |
      | (:X:Y) | [:T] | () |
    And no side effects

  Scenario: Matching twice with conflicting relationship types on same relationship
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T]->()
      """
    When executing query:
      """
      MATCH (a1)-[r:T]->()
      WITH r, a1
        LIMIT 1
      MATCH (a1)-[r:Y]->(b2)
      RETURN a1, r, b2
      """
    Then the result should be:
      | a1 | r | b2 |
    And no side effects

  Scenario: Matching twice with duplicate relationship types on same relationship
    Given an empty graph
    And having executed:
      """
      CREATE (:A)-[:T]->(:B)
      """
    When executing query:
      """
      MATCH (a1)-[r:T]->() WITH r, a1
      LIMIT 1
      MATCH (a1)-[r:T]->(b2)
      RETURN a1, r, b2
      """
    Then the result should be:
      | a1   | r    | b2   |
      | (:A) | [:T] | (:B) |
    And no side effects

@skip
  Scenario: Matching relationships into a list and matching variable length using the list
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B), (c:C)
      CREATE (a)-[:Y]->(b),
             (b)-[:Y]->(c)
      """
    When executing query:
      """
      MATCH ()-[r1]->()-[r2]->()
      WITH [r1, r2] AS rs
        LIMIT 1
      MATCH (first)-[rs*]->(second)
      RETURN first, second
      """
    Then the result should be:
      | first | second |
      | (:A)  | (:C)   |
    And no side effects

@skip
  Scenario: Matching relationships into a list and matching variable length using the list, with bound nodes
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B), (c:C)
      CREATE (a)-[:Y]->(b),
             (b)-[:Y]->(c)
      """
    When executing query:
      """
      MATCH (a)-[r1]->()-[r2]->(b)
      WITH [r1, r2] AS rs, a AS first, b AS second
        LIMIT 1
      MATCH (first)-[rs*]->(second)
      RETURN first, second
      """
    Then the result should be:
      | first | second |
      | (:A)  | (:C)   |
    And no side effects

@skip
  Scenario: Matching relationships into a list and matching variable length using the list, with bound nodes, wrong direction
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B), (c:C)
      CREATE (a)-[:Y]->(b),
             (b)-[:Y]->(c)
      """
    When executing query:
      """
      MATCH (a)-[r1]->()-[r2]->(b)
      WITH [r1, r2] AS rs, a AS second, b AS first
        LIMIT 1
      MATCH (first)-[rs*]->(second)
      RETURN first, second
      """
    Then the result should be:
      | first | second |
    And no side effects

  Scenario: Matching and optionally matching with bound nodes in reverse direction
    Given an empty graph
    And having executed:
      """
      CREATE (:A)-[:T]->(:B)
      """
    When executing query:
      """
      MATCH (a1)-[r]->()
      WITH r, a1
        LIMIT 1
      OPTIONAL MATCH (a1)<-[r]-(b2)
      RETURN a1, r, b2
      """
    Then the result should be:
      | a1   | r    | b2   |
      | (:A) | [:T] | null |
    And no side effects

  Scenario: Matching and optionally matching with unbound nodes and equality predicate in reverse direction
    Given an empty graph
    And having executed:
      """
      CREATE (:A)-[:T]->(:B)
      """
    When executing query:
      """
      MATCH (a1)-[r]->()
      WITH r, a1
        LIMIT 1
      OPTIONAL MATCH (a2)<-[r]-(b2)
      WHERE a1 = a2
      RETURN a1, r, b2, a2
      """
    Then the result should be:
      | a1   | r    | b2   | a2   |
      | (:A) | [:T] | null | null |
    And no side effects

  Scenario: Fail when using property access on primitive type
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 42})
      """
    When executing query:
      """
      MATCH (n)
      WITH n.num AS n2
      RETURN n2.num
      """
    Then a TypeError should be raised at runtime: PropertyAccessOnNonMap

  Scenario: Matching and returning ordered results, with LIMIT
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 1}), ({num: 3}), ({num: 2})
      """
    When executing query:
      """
      MATCH (foo)
      RETURN foo.num AS x
        ORDER BY x DESC
        LIMIT 4
      """
    Then the result should be, in order:
      | x |
      | 3 |
      | 2 |
      | 1 |
    And no side effects

@skip
  Scenario: Counting an empty graph
    Given an empty graph
    When executing query:
      """
      MATCH (a)
      RETURN count(a) > 0
      """
    Then the result should be:
      | count(a) > 0 |
      | false        |
    And no side effects

@skip
  Scenario: Matching variable length pattern with property predicate
    Given an empty graph
    And having executed:
      """
      CREATE (a:Artist:A), (b:Artist:B), (c:Artist:C)
      CREATE (a)-[:WORKED_WITH {year: 1987}]->(b),
             (b)-[:WORKED_WITH {year: 1988}]->(c)
      """
    When executing query:
      """
      MATCH (a:Artist)-[:WORKED_WITH* {year: 1988}]->(b:Artist)
      RETURN *
      """
    Then the result should be:
      | a           | b           |
      | (:Artist:B) | (:Artist:C) |
    And no side effects

@skip
  Scenario: Variable length pattern checking labels on endnodes
    Given an empty graph
    And having executed:
      """
      CREATE (a:TheLabel {id: 0}), (b:TheLabel {id: 1}), (c:TheLabel {id: 2})
      CREATE (a)-[:T]->(b),
             (b)-[:T]->(c)
      """
    When executing query:
      """
      MATCH (a), (b)
      WHERE a.id = 0
        AND (a)-[:T]->(b:TheLabel)
        OR (a)-[:T*]->(b:MissingLabel)
      RETURN DISTINCT b
      """
    Then the result should be:
      | b                   |
      | (:TheLabel {id: 1}) |
    And no side effects

@skip
  Scenario: Variable length pattern with label predicate on both sides
    Given an empty graph
    And having executed:
      """
      CREATE (a:Blue), (b:Red), (c:Green), (d:Yellow)
      CREATE (a)-[:T]->(b),
             (b)-[:T]->(c),
             (b)-[:T]->(d)
      """
    When executing query:
      """
      MATCH (a:Blue)-[r*]->(b:Green)
      RETURN count(r)
      """
    Then the result should be:
      | count(r) |
      | 1        |
    And no side effects

  Scenario: Undirected named path
    Given an empty graph
    And having executed:
      """
      CREATE (a:Movie), (b)
      CREATE (b)-[:T]->(a)
      """
    When executing query:
      """
      MATCH p = (n:Movie)--(m)
      RETURN p
        LIMIT 1
      """
    Then the result should be:
      | p                   |
      | <(:Movie)<-[:T]-()> |
    And no side effects

  Scenario: Named path with WITH
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH p = (a)
      WITH p
      RETURN p
      """
    Then the result should be:
      | p    |
      | <()> |
    And no side effects

@skip
  Scenario: Named path with alternating directed/undirected relationships
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B), (c:C)
      CREATE (b)-[:T]->(a),
             (c)-[:T]->(b)
      """
    When executing query:
      """
      MATCH p = (n)-->(m)--(o)
      RETURN p
      """
    Then the result should be:
      | p                            |
      | <(:C)-[:T]->(:B)-[:T]->(:A)> |
    And no side effects

@skip
  Scenario: Named path with multiple alternating directed/undirected relationships
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B), (c:C), (d:D)
      CREATE (b)-[:T]->(a),
             (c)-[:T]->(b),
             (d)-[:T]->(c)
      """
    When executing query:
      """
      MATCH path = (n)-->(m)--(o)--(p)
      RETURN path
      """
    Then the result should be:
      | path                                    |
      | <(:D)-[:T]->(:C)-[:T]->(:B)-[:T]->(:A)> |
    And no side effects

@skip
  Scenario: Named path with undirected fixed variable length pattern
    Given an empty graph
    And having executed:
      """
      CREATE (db1:Start), (db2:End), (mid), (other)
      CREATE (mid)-[:CONNECTED_TO]->(db1),
             (mid)-[:CONNECTED_TO]->(db2),
             (mid)-[:CONNECTED_TO]->(db2),
             (mid)-[:CONNECTED_TO]->(other),
             (mid)-[:CONNECTED_TO]->(other)
      """
    When executing query:
      """
      MATCH topRoute = (:Start)<-[:CONNECTED_TO]-()-[:CONNECTED_TO*3..3]-(:End)
      RETURN topRoute
      """
    Then the result should be:
      | topRoute                                                                                       |
      | <(:Start)<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->()<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->(:End)> |
      | <(:Start)<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->()<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->(:End)> |
      | <(:Start)<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->()<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->(:End)> |
      | <(:Start)<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->()<-[:CONNECTED_TO]-()-[:CONNECTED_TO]->(:End)> |
    And no side effects

  Scenario: Returning a node property value
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 1})
      """
    When executing query:
      """
      MATCH (a)
      RETURN a.num
      """
    Then the result should be:
      | a.num |
      | 1     |
    And no side effects

  Scenario: Returning a relationship property value
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T {num: 1}]->()
      """
    When executing query:
      """
      MATCH ()-[r]->()
      RETURN r.num
      """
    Then the result should be:
      | r.num |
      | 1     |
    And no side effects

  Scenario: Projecting nodes and relationships
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH (a)-[r]->()
      RETURN a AS foo, r AS bar
      """
    Then the result should be:
      | foo  | bar  |
      | (:A) | [:T] |
    And no side effects

  Scenario: Missing node property should become null
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 1})
      """
    When executing query:
      """
      MATCH (a)
      RETURN a.name
      """
    Then the result should be:
      | a.name |
      | null    |
    And no side effects

  Scenario: Missing relationship property should become null
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T {name: 1}]->()
      """
    When executing query:
      """
      MATCH ()-[r]->()
      RETURN r.name2
      """
    Then the result should be:
      | r.name2 |
      | null    |
    And no side effects

  Scenario: Returning multiple node property values
    Given an empty graph
    And having executed:
      """
      CREATE ({name: 'Philip J. Fry', age: 2046, seasons: [1, 2, 3, 4, 5, 6, 7]})
      """
    When executing query:
      """
      MATCH (a)
      RETURN a.name, a.age, a.seasons
      """
    Then the result should be:
      | a.name          | a.age | a.seasons             |
      | 'Philip J. Fry' | 2046  | [1, 2, 3, 4, 5, 6, 7] |
    And no side effects

  Scenario: Adding a property and a literal in projection
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 1})
      """
    When executing query:
      """
      MATCH (a)
      RETURN a.num + 1 AS foo
      """
    Then the result should be:
      | foo |
      | 2   |
    And no side effects

  Scenario: Adding list properties in projection
    Given an empty graph
    And having executed:
      """
      CREATE ({list1: [1, 2, 3], list2: [4, 5]})
      """
    When executing query:
      """
      MATCH (a)
      RETURN a.list2 + a.list1 AS foo
      """
    Then the result should be:
      | foo             |
      | [4, 5, 1, 2, 3] |
    And no side effects

@skip
  Scenario: Variable length relationship variables are lists of relationships
    Given an empty graph
    And having executed:
      """
      CREATE (a), (b), (c)
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH ()-[r*0..1]-()
      RETURN last(r) AS l
      """
    Then the result should be:
      | l    |
      | [:T] |
      | [:T] |
      | null |
      | null |
      | null |
    And no side effects

  Scenario: Variable length patterns and nulls
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      """
    When executing query:
      """
      MATCH (a:A)
      OPTIONAL MATCH (a)-[:FOO]->(b:B)
      OPTIONAL MATCH (b)<-[:BAR*]-(c:B)
      RETURN a, b, c
      """
    Then the result should be:
      | a    | b    | c    |
      | (:A) | null | null |
    And no side effects

  Scenario: Projecting a list of nodes and relationships
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH (n)-[r]->(m)
      RETURN [n, r, m] AS r
      """
    Then the result should be:
      | r                  |
      | [(:A), [:T], (:B)] |
    And no side effects

@skip
  Scenario: Projecting a map of nodes and relationships
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH (n)-[r]->(m)
      RETURN {node1: n, rel: r, node2: m} AS m
      """
    Then the result should be:
      | m                                     |
      | {node1: (:A), rel: [:T], node2: (:B)} |
    And no side effects

  Scenario: Respecting direction when matching existing path
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'a'}), (b {name: 'b'})
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH p = ({name: 'a'})-->({name: 'b'})
      RETURN p
      """
    Then the result should be:
      | p                                   |
      | <({name: 'a'})-[:T]->({name: 'b'})> |
    And no side effects

  Scenario: Respecting direction when matching non-existent path
    Given an empty graph
    And having executed:
      """
      CREATE (a {name: 'a'}), (b {name: 'b'})
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH p = ({name: 'a'})<--({name: 'b'})
      RETURN p
      """
    Then the result should be:
      | p |
    And no side effects

@skip
  Scenario: Respecting direction when matching non-existent path with multiple directions
    Given an empty graph
    And having executed:
      """
      CREATE (a), (b)
      CREATE (a)-[:T]->(b),
             (b)-[:T]->(a)
      """
    When executing query:
      """
      MATCH p = (n)-->(k)<--(n)
      RETURN p
      """
    Then the result should be:
      | p |
    And no side effects

@skip
  Scenario: Matching path with both directions should respect other directions
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      CREATE (a)-[:T1]->(b),
             (b)-[:T2]->(a)
      """
    When executing query:
      """
      MATCH p = (n)<-->(k)<--(n)
      RETURN p
      """
    Then the result should be:
      | p                              |
      | <(:A)<-[:T2]-(:B)<-[:T1]-(:A)> |
      | <(:B)<-[:T1]-(:A)<-[:T2]-(:B)> |
    And no side effects

@skip
  Scenario: Matching path with multiple bidirectional relationships
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      CREATE (a)-[:T1]->(b),
             (b)-[:T2]->(a)
      """
    When executing query:
      """
      MATCH p=(n)<-->(k)<-->(n)
      RETURN p
      """
    Then the result should be:
      | p                              |
      | <(:A)<-[:T2]-(:B)<-[:T1]-(:A)> |
      | <(:A)-[:T1]->(:B)-[:T2]->(:A)> |
      | <(:B)<-[:T1]-(:A)<-[:T2]-(:B)> |
      | <(:B)-[:T2]->(:A)-[:T1]->(:B)> |
    And no side effects

@skip
  Scenario: Matching nodes with many labels
    Given an empty graph
    And having executed:
      """
      CREATE (a:A:B:C:D:E:F:G:H:I:J:K:L:M),
             (b:U:V:W:X:Y:Z)
      CREATE (a)-[:T]->(b)
      """
    When executing query:
      """
      MATCH (n:A:B:C:D:E:F:G:H:I:J:K:L:M)-[:T]->(m:Z:Y:X:W:V:U)
      RETURN n, m
      """
    Then the result should be:
      | n                            | m              |
      | (:A:B:C:D:E:F:G:H:I:J:K:L:M) | (:Z:Y:X:W:V:U) |
    And no side effects

@skip
  Scenario: Matching longer variable length paths
    Given an empty graph
    And having executed:
      """
      CREATE (a {var: 'start'}), (b {var: 'end'})
      WITH *
      UNWIND range(1, 20) AS i
      CREATE (n {var: i})
      WITH [a] + collect(n) + [b] AS nodeList
      UNWIND range(0, size(nodeList) - 2, 1) AS i
      WITH nodeList[i] AS n1, nodeList[i+1] AS n2
      CREATE (n1)-[:T]->(n2)
      """
    When executing query:
      """
      MATCH (n {var: 'start'})-[:T*]->(m {var: 'end'})
      RETURN m
      """
    Then the result should be:
      | m              |
      | ({var: 'end'}) |
    And no side effects

@skip
  Scenario: Counting rows after MATCH, MERGE, OPTIONAL MATCH
    Given an empty graph
    And having executed:
      """
      CREATE (a:A), (b:B)
      CREATE (a)-[:T1]->(b),
             (b)-[:T2]->(a)
      """
    When executing query:
      """
      MATCH (a)
      MERGE (b)
      WITH *
      OPTIONAL MATCH (a)--(b)
      RETURN count(*)
      """
    Then the result should be:
      | count(*) |
      | 6        |
    And no side effects

@skip
  Scenario: Matching a self-loop
    Given an empty graph
    And having executed:
      """
      CREATE (a)
      CREATE (a)-[:T]->(a)
      """
    When executing query:
      """
      MATCH ()-[r]-()
      RETURN type(r) AS r
      """
    Then the result should be:
      | r   |
      | 'T' |
    And no side effects
