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

Feature: NullAcceptance

  Scenario: Property existence check on non-null node
    Given an empty graph
    And having executed:
      """
      CREATE ({exists: 42})
      """
    When executing query:
      """
      MATCH (n)
      RETURN exists(n.missing),
             exists(n.exists)
      """
    Then the result should be:
      | exists(n.missing) | exists(n.exists) |
      | false             | true             |
    And no side effects

  Scenario: Property existence check on optional non-null node
    Given an empty graph
    And having executed:
      """
      CREATE ({exists: 42})
      """
    When executing query:
      """
      OPTIONAL MATCH (n)
      RETURN exists(n.missing),
             exists(n.exists)
      """
    Then the result should be:
      | exists(n.missing) | exists(n.exists) |
      | false             | true             |
    And no side effects

@skip
  Scenario: Property existence check on null node
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (n)
      RETURN exists(n.missing)
      """
    Then the result should be:
      | exists(n.missing) |
      | null              |
    And no side effects

  Scenario: Ignore null when setting property
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      SET a.num = 42
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

@skip
  Scenario: Ignore null when removing property
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      REMOVE a.num
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

@skip
  Scenario: Ignore null when setting properties using an appending map
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      SET a += {num: 42}
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

@skip
  Scenario: Ignore null when setting properties using an overriding map
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      SET a = {num: 42}
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

@skip
  Scenario: Ignore null when setting label
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      SET a:L
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

@skip
  Scenario: Ignore null when removing label
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      REMOVE a:L
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

  Scenario: Ignore null when deleting node
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH (a:DoesNotExist)
      DELETE a
      RETURN a
      """
    Then the result should be:
      | a    |
      | null |
    And no side effects

  Scenario: Ignore null when deleting relationship
    Given an empty graph
    When executing query:
      """
      OPTIONAL MATCH ()-[r:DoesNotExist]-()
      DELETE r
      RETURN r
      """
    Then the result should be:
      | r    |
      | null |
    And no side effects
