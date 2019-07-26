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

Feature: KeysAcceptance

  Scenario: Using `keys()` on a single node, non-empty result
    Given an empty graph
    And having executed:
      """
      CREATE ({name: 'Andres', surname: 'Lopez'})
      """
    When executing query:
      """
      MATCH (n)
      UNWIND keys(n) AS x
      RETURN DISTINCT x AS theProps
      """
    Then the result should be:
      | theProps  |
      | 'name'    |
      | 'surname' |
    And no side effects

  Scenario: Using `keys()` on multiple nodes, non-empty result
    Given an empty graph
    And having executed:
      """
      CREATE ({name: 'Andres', surname: 'Lopez'}),
             ({otherName: 'Andres', otherSurname: 'Lopez'})
      """
    When executing query:
      """
      MATCH (n)
      UNWIND keys(n) AS x
      RETURN DISTINCT x AS theProps
      """
    Then the result should be:
      | theProps       |
      | 'name'         |
      | 'surname'      |
      | 'otherName'    |
      | 'otherSurname' |
    And no side effects

  Scenario: Using `keys()` on a single node, empty result
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      UNWIND keys(n) AS x
      RETURN DISTINCT x AS theProps
      """
    Then the result should be:
      | theProps |
    And no side effects

  Scenario: Using `keys()` on an optionally matched node
    Given an empty graph
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      OPTIONAL MATCH (n)
      UNWIND keys(n) AS x
      RETURN DISTINCT x AS theProps
      """
    Then the result should be:
      | theProps |
    And no side effects

  Scenario: Using `keys()` on a relationship, non-empty result
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:KNOWS {status: 'bad', year: '2015'}]->()
      """
    When executing query:
      """
      MATCH ()-[r:KNOWS]-()
      UNWIND keys(r) AS x
      RETURN DISTINCT x AS theProps
      """
    Then the result should be:
      | theProps |
      | 'status' |
      | 'year'   |
    And no side effects

  Scenario: Using `keys()` on a relationship, empty result
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:KNOWS]->()
      """
    When executing query:
      """
      MATCH ()-[r:KNOWS]-()
      UNWIND keys(r) AS x
      RETURN DISTINCT x AS theProps
      """
    Then the result should be:
      | theProps |
    And no side effects

  Scenario: Using `keys()` on an optionally matched relationship
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:KNOWS]->()
      """
    When executing query:
      """
      OPTIONAL MATCH ()-[r:KNOWS]-()
      UNWIND keys(r) AS x
      RETURN DISTINCT x AS theProps
      """
    Then the result should be:
      | theProps |
    And no side effects

  Scenario: Using `keys()` on a literal map
    Given any graph
    When executing query:
      """
      RETURN keys({name: 'Alice', age: 38, address: {city: 'London', residential: true}}) AS k
      """
    Then the result should be (ignoring element order for lists):
      | k                          |
      | ['name', 'age', 'address'] |
    And no side effects

  Scenario: Using `keys()` on a parameter map
    Given any graph
    And parameters are:
      | param | {name: 'Alice', age: 38, address: {city: 'London', residential: true}} |
    When executing query:
      """
      RETURN keys($param) AS k
      """
    Then the result should be (ignoring element order for lists):
      | k                          |
      | ['address', 'name', 'age'] |
    And no side effects
