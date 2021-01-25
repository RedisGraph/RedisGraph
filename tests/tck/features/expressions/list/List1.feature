#
# Copyright (c) 2015-2021 "Neo Technology,"
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

Feature: List1 - Dynamic Element Access
# Dynamic element access refers to the bracket-operator – <expression resulting in a list>[<expression resulting in an integer>] – irrespectively of whether the list index – i.e. <expression resulting in an integer> – could be evaluated statically in a given scenario.

  Scenario: [1] Indexing into literal list
    Given any graph
    When executing query:
      """
      RETURN [1, 2, 3][0] AS value
      """
    Then the result should be, in any order:
      | value |
      | 1     |
    And no side effects

  Scenario: [2] Indexing into nested literal lists
    Given any graph
    When executing query:
      """
      RETURN [[1]][0][0]
      """
    Then the result should be, in any order:
      | [[1]][0][0] |
      | 1           |
    And no side effects

  Scenario: [3] Use list lookup based on parameters when there is no type information
    Given any graph
    And parameters are:
      | expr | ['Apa'] |
      | idx  | 0       |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[idx] AS value
      """
    Then the result should be, in any order:
      | value |
      | 'Apa' |
    And no side effects

  Scenario: [4] Use list lookup based on parameters when there is lhs type information
    Given any graph
    And parameters are:
      | idx | 0 |
    When executing query:
      """
      WITH ['Apa'] AS expr
      RETURN expr[$idx] AS value
      """
    Then the result should be, in any order:
      | value |
      | 'Apa' |
    And no side effects

  Scenario: [5] Use list lookup based on parameters when there is rhs type information
    Given any graph
    And parameters are:
      | expr | ['Apa'] |
      | idx  | 0       |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[toInteger(idx)] AS value
      """
    Then the result should be, in any order:
      | value |
      | 'Apa' |
    And no side effects

  @NegativeTest
  Scenario: [6] Fail at runtime when attempting to index with a String into a List
    Given any graph
    And parameters are:
      | expr | ['Apa'] |
      | idx  | 'name'  |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[idx]
      """
    Then a TypeError should be raised at runtime: ListElementAccessByNonInteger

  @NegativeTest
  Scenario: [7] Fail at runtime when trying to index into a list with a list
    Given any graph
    And parameters are:
      | expr | ['Apa'] |
      | idx  | ['Apa'] |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[idx]
      """
    Then a TypeError should be raised at compile time: ListElementAccessByNonInteger

  @NegativeTest
  Scenario: [8] Fail at compile time when attempting to index with a non-integer into a list
    Given any graph
    When executing query:
      """
      WITH [1, 2, 3, 4, 5] AS list, 3.14 AS idx
      RETURN list[idx]
      """
    Then a SyntaxError should be raised at compile time: ListElementAccessByNonInteger

  @NegativeTest
  Scenario: [9] Fail at runtime when trying to index something which is not a list
    Given any graph
    And parameters are:
      | expr | 100 |
      | idx  | 0   |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[idx]
      """
    Then a TypeError should be raised at runtime: InvalidArgumentType
