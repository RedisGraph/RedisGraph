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

Feature: List6 - List size

  Scenario: [1] Return list size
    Given any graph
    When executing query:
      """
      RETURN size([1, 2, 3]) AS n
      """
    Then the result should be, in any order:
      | n |
      | 3 |
    And no side effects

  Scenario: [2] Setting and returning the size of a list property
    Given an empty graph
    And having executed:
      """
      CREATE (:TheLabel)
      """
    When executing query:
      """
      MATCH (n:TheLabel)
      SET n.numbers = [1, 2, 3]
      RETURN size(n.numbers)
      """
    Then the result should be, in any order:
      | size(n.numbers) |
      | 3               |
    And the side effects should be:
      | +properties | 1 |

  Scenario: [3] Concatenating and returning the size of literal lists
    Given any graph
    When executing query:
      """
      RETURN size([[], []] + [[]]) AS l
      """
    Then the result should be, in any order:
      | l |
      | 3 |
    And no side effects

  Scenario: [4] `size()` on null list
    Given any graph
    When executing query:
      """
      WITH null AS l
      RETURN size(l), size(null)
      """
    Then the result should be, in any order:
      | size(l) | size(null) |
      | null    | null       |
    And no side effects

  @NegativeTest
  @skip
  Scenario: [5] Fail for `size()` on paths
    Given any graph
    When executing query:
      """
      MATCH p = (a)-[*]->(b)
      RETURN size(p)
      """
    Then a SyntaxError should be raised at compile time: InvalidArgumentType
