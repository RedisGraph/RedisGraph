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

Feature: Return3 - Return multiple expressions (if column order correct)

  Scenario: [1] Returning multiple expressions
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (a)
      RETURN exists(a.id), a IS NOT NULL
      """
    Then the result should be, in any order:
      | exists(a.id) | a IS NOT NULL |
      | false        | true          |
    And no side effects

  Scenario: [2] Returning multiple node property values
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
    Then the result should be, in any order:
      | a.name          | a.age | a.seasons             |
      | 'Philip J. Fry' | 2046  | [1, 2, 3, 4, 5, 6, 7] |
    And no side effects

  Scenario: [3] Projecting nodes and relationships
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
    Then the result should be, in any order:
      | foo  | bar  |
      | (:A) | [:T] |
    And no side effects
