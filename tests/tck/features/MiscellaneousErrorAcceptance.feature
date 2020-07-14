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

Feature: MiscellaneousErrorAcceptance

  Background:
    Given any graph

@skip
  Scenario: Failing on incorrect unicode literal
    When executing query:
      """
      RETURN '\uH'
      """
    Then a SyntaxError should be raised at compile time: InvalidUnicodeLiteral

@skip
  Scenario: Failing on merging relationship with null property
    When executing query:
      """
      CREATE (a), (b)
      MERGE (a)-[r:X {num: null}]->(b)
      """
    Then a SemanticError should be raised at compile time: MergeReadOwnWrites

@skip
  Scenario: Failing on merging node with null property
    When executing query:
      """
      MERGE ({num: null})
      """
    Then a SemanticError should be raised at compile time: MergeReadOwnWrites

  Scenario: Failing on aggregation in WHERE
    When executing query:
      """
      MATCH (a)
      WHERE count(a) > 10
      RETURN a
      """
    Then a SyntaxError should be raised at compile time: InvalidAggregation

@skip
  Scenario: Failing on aggregation in ORDER BY after RETURN
    When executing query:
      """
      MATCH (n)
      RETURN n.num1
        ORDER BY max(n.num2)
      """
    Then a SyntaxError should be raised at compile time: InvalidAggregation

@skip
  Scenario: Failing on aggregation in ORDER BY after WITH
    When executing query:
      """
      MATCH (n)
      WITH n.num1 AS foo
        ORDER BY max(n.num2)
      RETURN foo AS foo
      """
    Then a SyntaxError should be raised at compile time: InvalidAggregation

  Scenario: Failing when not aliasing expressions in WITH
    When executing query:
      """
      MATCH (a)
      WITH a, count(*)
      RETURN a
      """
    Then a SyntaxError should be raised at compile time: NoExpressionAlias

  Scenario: Failing when using undefined variable in pattern
    When executing query:
      """
      MATCH (a)
      CREATE (a)-[:KNOWS]->(b {name: missing})
      RETURN b
      """
    Then a SyntaxError should be raised at compile time: UndefinedVariable

  Scenario: Failing when using undefined variable in SET
    When executing query:
      """
      MATCH (a)
      SET a.name = missing
      RETURN a
      """
    Then a SyntaxError should be raised at compile time: UndefinedVariable

  Scenario: Failing when using undefined variable in DELETE
    When executing query:
      """
      MATCH (a)
      DELETE x
      """
    Then a SyntaxError should be raised at compile time: UndefinedVariable

@skip
  Scenario: Failing when using a variable that is already bound in CREATE
    When executing query:
      """
      MATCH (a)
      CREATE (a {name: 'foo'})
      RETURN a
      """
    Then a SyntaxError should be raised at compile time: VariableAlreadyBound

@skip
  Scenario: Failing when using a path variable that is already bound
    When executing query:
      """
      MATCH p = (a)
      WITH p, a
      MATCH p = (a)-->(b)
      RETURN a
      """
    Then a SyntaxError should be raised at compile time: VariableAlreadyBound

@skip
  Scenario: Failing when using a list as a node
    When executing query:
      """
      MATCH (n)
      WITH [n] AS users
      MATCH (users)-->(messages)
      RETURN messages
      """
    Then a SyntaxError should be raised at compile time: VariableTypeConflict

@skip
  Scenario: Failing when using a variable length relationship as a single relationship
    When executing query:
      """
      MATCH (n)
      MATCH (n)-[r*]->()
      WHERE r.name = 'apa'
      RETURN r
      """
    Then a SyntaxError should be raised at compile time: InvalidArgumentType

  Scenario: Failing when UNION has different columns
    When executing query:
      """
      RETURN 1 AS a
      UNION
      RETURN 2 AS b
      """
    Then a SyntaxError should be raised at compile time: DifferentColumnsInUnion

  Scenario: Failing when mixing UNION and UNION ALL
    When executing query:
      """
      RETURN 1 AS a
      UNION
      RETURN 2 AS a
      UNION ALL
      RETURN 3 AS a
      """
    Then a SyntaxError should be raised at compile time: InvalidClauseComposition

  Scenario: Failing when creating without direction
    When executing query:
      """
      CREATE (a)-[:FOO]-(b)
      """
    Then a SyntaxError should be raised at compile time: RequiresDirectedRelationship

  Scenario: Failing when creating with two directions
    When executing query:
      """
      CREATE (a)<-[:FOO]->(b)
      """
    Then a SyntaxError should be raised at compile time: RequiresDirectedRelationship

@skip
  Scenario: Failing when deleting a label
    When executing query:
      """
      MATCH (n)
      DELETE n:Person
      """
    Then a SyntaxError should be raised at compile time: InvalidDelete

@skip
  Scenario: Failing when setting a list of maps as a property
    When executing query:
      """
      CREATE (a)
      SET a.maplist = [{num: 1}]
      """
    Then a TypeError should be raised at compile time: InvalidPropertyType

@crash
@skip
  Scenario: Failing when multiple columns have the same name
    When executing query:
      """
      RETURN 1 AS a, 2 AS a
      """
    Then a SyntaxError should be raised at compile time: ColumnNameConflict

@skip
  Scenario: Failing when using RETURN * without variables in scope
    When executing query:
      """
      MATCH ()
      RETURN *
      """
    Then a SyntaxError should be raised at compile time: NoVariablesInScope
