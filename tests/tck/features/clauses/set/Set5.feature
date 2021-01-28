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

Feature: Set5 - Set multiple properties with a map

  @crash
  Scenario: [1] Ignore null when setting properties using an appending map
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      SET a += {num: 42}
      RETURN a
      """
    Then the result should be, in any order:
      | a    |
      | null |
    And no side effects

  @crash
  Scenario: [2] Overwrite values when using +=
    Given an empty graph
    And having executed:
      """
      CREATE (:X {name: 'A', name2: 'B'})
      """
    When executing query:
      """
      MATCH (n:X {name: 'A'})
      SET n += {name2: 'C'}
      RETURN n
      """
    Then the result should be, in any order:
      | n                            |
      | (:X {name: 'A', name2: 'C'}) |
    And the side effects should be:
      | +properties | 1 |
      | -properties | 1 |

  @crash
  Scenario: [3] Retain old values when using +=
    Given an empty graph
    And having executed:
      """
      CREATE (:X {name: 'A'})
      """
    When executing query:
      """
      MATCH (n:X {name: 'A'})
      SET n += {name2: 'B'}
      RETURN n
      """
    Then the result should be, in any order:
      | n                            |
      | (:X {name: 'A', name2: 'B'}) |
    And the side effects should be:
      | +properties | 1 |

  @crash
  Scenario: [4] Explicit null values in a map remove old values
    Given an empty graph
    And having executed:
      """
      CREATE (:X {name: 'A', name2: 'B'})
      """
    When executing query:
      """
      MATCH (n:X {name: 'A'})
      SET n += {name: null}
      RETURN n
      """
    Then the result should be, in any order:
      | n                 |
      | (:X {name2: 'B'}) |
    And the side effects should be:
      | -properties | 1 |

  @crash
  Scenario: [5] Set an empty map when using += has no effect
    Given an empty graph
    And having executed:
      """
      CREATE (:X {name: 'A', name2: 'B'})
      """
    When executing query:
      """
      MATCH (n:X {name: 'A'})
      SET n += { }
      RETURN n
      """
    Then the result should be, in any order:
      | n                            |
      | (:X {name: 'A', name2: 'B'}) |
    And no side effects