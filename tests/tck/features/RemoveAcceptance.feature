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

Feature: RemoveAcceptance

@skip
  Scenario: Should ignore nulls
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 42})
      """
    When executing query:
      """
      MATCH (n)
      OPTIONAL MATCH (n)-[r]->()
      REMOVE r.num
      RETURN n
      """
    Then the result should be:
      | n           |
      | ({num: 42}) |
    And no side effects

@skip
  Scenario: Remove a single label
    Given an empty graph
    And having executed:
      """
      CREATE (:L {num: 42})
      """
    When executing query:
      """
      MATCH (n)
      REMOVE n:L
      RETURN n.num
      """
    Then the result should be:
      | n.num |
      | 42    |
    And the side effects should be:
      | -labels | 1 |

@skip
  Scenario: Remove multiple labels
    Given an empty graph
    And having executed:
      """
      CREATE (:L1:L2:L3 {num: 42})
      """
    When executing query:
      """
      MATCH (n)
      REMOVE n:L1:L3
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n) |
      | ['L2']    |
    And the side effects should be:
      | -labels | 2 |

@skip
  Scenario: Remove a single node property
    Given an empty graph
    And having executed:
      """
      CREATE (:L {num: 42})
      """
    When executing query:
      """
      MATCH (n)
      REMOVE n.num
      RETURN exists(n.num) AS still_there
      """
    Then the result should be:
      | still_there |
      | false       |
    And the side effects should be:
      | -properties | 1 |

@skip
  Scenario: Remove multiple node properties
    Given an empty graph
    And having executed:
      """
      CREATE (:L {num: 42, name: 'a', name2: 'B'})
      """
    When executing query:
      """
      MATCH (n)
      REMOVE n.num, n.name
      RETURN size(keys(n)) AS props
      """
    Then the result should be:
      | props |
      | 1     |
    And the side effects should be:
      | -properties | 2 |

@skip
  Scenario: Remove a single relationship property
    Given an empty graph
    And having executed:
      """
      CREATE (a), (b), (a)-[:X {num: 42}]->(b)
      """
    When executing query:
      """
      MATCH ()-[r]->()
      REMOVE r.num
      RETURN exists(r.num) AS still_there
      """
    Then the result should be:
      | still_there |
      | false       |
    And the side effects should be:
      | -properties | 1 |

@skip
  Scenario: Remove multiple relationship properties
    Given an empty graph
    And having executed:
      """
      CREATE (a), (b), (a)-[:X {num: 42, a: 'a', b: 'B'}]->(b)
      """
    When executing query:
      """
      MATCH ()-[r]->()
      REMOVE r.num, r.a
      RETURN size(keys(r)) AS props
      """
    Then the result should be:
      | props |
      | 1     |
    And the side effects should be:
      | -properties | 2 |

@skip
  Scenario: Remove a missing property should be a valid operation
    Given an empty graph
    And having executed:
      """
      CREATE (), (), ()
      """
    When executing query:
      """
      MATCH (n)
      REMOVE n.num
      RETURN sum(size(keys(n))) AS totalNumberOfProps
      """
    Then the result should be:
      | totalNumberOfProps |
      | 0                  |
    And no side effects
