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

Feature: EqualsAcceptance

@skip
  Scenario: Number-typed integer comparison
    Given an empty graph
    And having executed:
      """
      CREATE ({id: 0})
      """
    When executing query:
      """
      WITH collect([0, 0.0]) AS numbers
      UNWIND numbers AS arr
      WITH arr[0] AS expected
      MATCH (n) WHERE toInteger(n.id) = expected
      RETURN n
      """
    Then the result should be:
      | n         |
      | ({id: 0}) |
    And no side effects

@skip
  Scenario: Number-typed float comparison
    Given an empty graph
    And having executed:
      """
      CREATE ({id: 0})
      """
    When executing query:
      """
      WITH collect([0.5, 0]) AS numbers
      UNWIND numbers AS arr
      WITH arr[0] AS expected
      MATCH (n) WHERE toInteger(n.id) = expected
      RETURN n
      """
    Then the result should be:
      | n |
    And no side effects

@skip
  Scenario: Any-typed string comparison
    Given an empty graph
    And having executed:
      """
      CREATE ({id: 0})
      """
    When executing query:
      """
      WITH collect(['0', 0]) AS things
      UNWIND things AS arr
      WITH arr[0] AS expected
      MATCH (n) WHERE toInteger(n.id) = expected
      RETURN n
      """
    Then the result should be:
      | n |
    And no side effects

  Scenario: Comparing nodes to nodes
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (a)
      WITH a
      MATCH (b)
      WHERE a = b
      RETURN count(b)
      """
    Then the result should be:
      | count(b) |
      | 1        |
    And no side effects

@skip
  Scenario: Comparing relationships to relationships
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:T]->()
      """
    When executing query:
      """
      MATCH ()-[a]->()
      WITH a
      MATCH ()-[b]->()
      WHERE a = b
      RETURN count(b)
      """
    Then the result should be:
      | count(b) |
      | 1        |
    And no side effects

@skip
  Scenario Outline: Comparing lists to lists
    Given an empty graph
    When executing query:
      """
      RETURN <lhs> = <rhs> AS result
      """
    Then the result should be:
      | result   |
      | <result> |
    And no side effects

    Examples:
      | lhs           | rhs           | result |
      | [1, 2]        | [1]           | false  |
      | [null]        | [1]           | null   |
      | ['a']         | [1]           | null   |
      | [[1]]         | [[1], [null]] | false  |
      | [[1], [2]]    | [[1], [null]] | null   |
      | [[1], [2, 3]] | [[1], [null]] | false  |
