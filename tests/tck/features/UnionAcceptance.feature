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

Feature: UnionAcceptance

  Scenario: Should be able to create text output from union queries
    Given an empty graph
    And having executed:
      """
      CREATE (:A), (:B)
      """
    When executing query:
      """
      MATCH (a:A)
      RETURN a AS a
      UNION
      MATCH (b:B)
      RETURN b AS a
      """
    Then the result should be:
      | a    |
      | (:A) |
      | (:B) |
    And no side effects

  Scenario: Two elements, both unique, not distinct
    Given an empty graph
    When executing query:
      """
      RETURN 1 AS x
      UNION ALL
      RETURN 2 AS x
      """
    Then the result should be:
      | x |
      | 1 |
      | 2 |
    And no side effects

  Scenario: Two elements, both unique, distinct
    Given an empty graph
    When executing query:
      """
      RETURN 1 AS x
      UNION
      RETURN 2 AS x
      """
    Then the result should be:
      | x |
      | 1 |
      | 2 |
    And no side effects

  Scenario: Three elements, two unique, distinct
    Given an empty graph
    When executing query:
      """
      RETURN 2 AS x
      UNION
      RETURN 1 AS x
      UNION
      RETURN 2 AS x
      """
    Then the result should be:
      | x |
      | 2 |
      | 1 |
    And no side effects

  Scenario: Three elements, two unique, not distinct
    Given an empty graph
    When executing query:
      """
      RETURN 2 AS x
      UNION ALL
      RETURN 1 AS x
      UNION ALL
      RETURN 2 AS x
      """
    Then the result should be:
      | x |
      | 2 |
      | 1 |
      | 2 |
    And no side effects
