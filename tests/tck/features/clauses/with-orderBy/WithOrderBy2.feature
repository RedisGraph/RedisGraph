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

Feature: WithOrderBy2 - Order by a single expression

  Scenario: [1] ORDER BY on an aggregating key
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
    Then the result should be, in any order:
      | bars | relCount |
      | 'A'  | 2        |
      | 'B'  | 1        |
    And no side effects

  Scenario: [2] ORDER BY a DISTINCT column
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
    Then the result should be, in any order:
      | bars |
      | 'A'  |
      | 'B'  |
    And no side effects

  @NegativeTest
  Scenario: [3] Fail on aggregation in ORDER BY after WITH
    Given any graph
    When executing query:
      """
      MATCH (n)
      WITH n.num1 AS foo
        ORDER BY max(n.num2)
      RETURN foo AS foo
      """
    Then a SyntaxError should be raised at compile time: InvalidAggregation
