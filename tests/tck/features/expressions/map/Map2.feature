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

Feature: Map2 - Dynamic Value Access
# Dynamic value access refers to the bracket-operator – <expression resulting in a map>'['<expression resulting in a string>']' – irrespectively of whether the map key – i.e. <expression resulting in a string> – could be evaluated statically in a given scenario.

  Scenario: [1] Use dynamic property lookup based on parameters when there is no type information
    Given any graph
    And parameters are:
      | expr | {name: 'Apa'} |
      | idx  | 'name'        |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[idx] AS value
      """
    Then the result should be, in any order:
      | value |
      | 'Apa' |
    And no side effects

  Scenario: [2] Use dynamic property lookup based on parameters when there is rhs type information
    Given any graph
    And parameters are:
      | expr | {name: 'Apa'} |
      | idx  | 'name'        |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[toString(idx)] AS value
      """
    Then the result should be, in any order:
      | value |
      | 'Apa' |
    And no side effects

  @NegativeTest
  Scenario: [3] Fail at runtime when attempting to index with an Int into a Map
    Given any graph
    And parameters are:
      | expr | {name: 'Apa'} |
      | idx  | 0             |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[idx]
      """
    Then a TypeError should be raised at runtime: MapElementAccessByNonString

  @NegativeTest
  Scenario: [4] Fail at runtime when trying to index into a map with a non-string
    Given any graph
    And parameters are:
      | expr | {name: 'Apa'} |
      | idx  | 12.3          |
    When executing query:
      """
      WITH $expr AS expr, $idx AS idx
      RETURN expr[idx]
      """
    Then a TypeError should be raised at runtime: MapElementAccessByNonString

  @NegativeTest
  Scenario: [5] Fail at runtime when trying to index something which is not a map
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
