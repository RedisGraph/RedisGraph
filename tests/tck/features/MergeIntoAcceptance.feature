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

Feature: MergeIntoAcceptance

  Background:
    Given an empty graph
    And having executed:
      """
      CREATE (:A {name: 'A'}), (:B {name: 'B'})
      """

  Scenario: Updating one property with ON CREATE
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'})
      MERGE (a)-[r:TYPE]->(b)
        ON CREATE SET r.name = 'foo'
      """
    Then the result should be empty
    And the side effects should be:
      | +relationships | 1 |
      | +properties    | 1 |
    When executing control query:
      """
      MATCH ()-[r:TYPE]->()
      RETURN [key IN keys(r) | key + '->' + r[key]] AS keyValue
      """
    Then the result should be:
      | keyValue      |
      | ['name->foo'] |

  Scenario: Null-setting one property with ON CREATE
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'})
      MERGE (a)-[r:TYPE]->(b)
        ON CREATE SET r.name = null
      """
    Then the result should be empty
    And the side effects should be:
      | +relationships | 1 |
    When executing control query:
      """
      MATCH ()-[r:TYPE]->()
      RETURN [key IN keys(r) | key + '->' + r[key]] AS keyValue
      """
    Then the result should be:
      | keyValue |
      | []       |

  Scenario: Copying properties from node with ON CREATE
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'})
      MERGE (a)-[r:TYPE]->(b)
        ON CREATE SET r = a
      """
    Then the result should be empty
    And the side effects should be:
      | +relationships | 1 |
      | +properties    | 1 |
    When executing control query:
      """
      MATCH ()-[r:TYPE]->()
      RETURN [key IN keys(r) | key + '->' + r[key]] AS keyValue
      """
    Then the result should be:
      | keyValue    |
      | ['name->A'] |

  Scenario: Copying properties from node with ON MATCH
    And having executed:
      """
      MATCH (a:A), (b:B)
      CREATE (a)-[:TYPE {name: 'bar'}]->(b)
      """
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'})
      MERGE (a)-[r:TYPE]->(b)
        ON MATCH SET r = a
      """
    Then the result should be empty
    And the side effects should be:
      | +properties | 1 |
      | -properties | 1 |
    When executing control query:
      """
      MATCH ()-[r:TYPE]->()
      RETURN [key IN keys(r) | key + '->' + r[key]] AS keyValue
      """
    Then the result should be:
      | keyValue    |
      | ['name->A'] |

  Scenario: Copying properties from literal map with ON CREATE
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'})
      MERGE (a)-[r:TYPE]->(b)
      ON CREATE SET r += {name: 'bar', name2: 'baz'}
      """
    Then the result should be empty
    And the side effects should be:
      | +relationships | 1 |
      | +properties    | 2 |
    When executing control query:
      """
      MATCH ()-[r:TYPE]->()
      RETURN [key IN keys(r) | key + '->' + r[key]] AS keyValue
      """
    Then the result should be (ignoring element order for lists):
      | keyValue                    |
      | ['name->bar', 'name2->baz'] |

  Scenario: Copying properties from literal map with ON MATCH
    And having executed:
      """
      MATCH (a:A), (b:B)
      CREATE (a)-[:TYPE {name: 'bar'}]->(b)
      """
    When executing query:
      """
      MATCH (a {name: 'A'}), (b {name: 'B'})
      MERGE (a)-[r:TYPE]->(b)
        ON MATCH SET r += {name: 'baz', name2: 'baz'}
      """
    Then the result should be empty
    And the side effects should be:
      | +properties | 2 |
      | -properties | 1 |
    When executing control query:
      """
      MATCH ()-[r:TYPE]->()
      RETURN [key IN keys(r) | key + '->' + r[key]] AS keyValue
      """
    Then the result should be (ignoring element order for lists):
      | keyValue                    |
      | ['name->baz', 'name2->baz'] |
