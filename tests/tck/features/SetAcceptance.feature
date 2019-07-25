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

Feature: SetAcceptance

@skip
  Scenario: Setting a node property to null removes the existing property
    Given an empty graph
    And having executed:
      """
      CREATE (:A {property1: 23, property2: 46})
      """
    When executing query:
      """
      MATCH (n:A)
      SET n.property1 = null
      RETURN n
      """
    Then the result should be:
      | n                    |
      | (:A {property2: 46}) |
    And the side effects should be:
      | -properties | 1 |

@skip
  Scenario: Setting a relationship property to null removes the existing property
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:REL {property1: 12, property2: 24}]->()
      """
    When executing query:
      """
      MATCH ()-[r]->()
      SET r.property1 = null
      RETURN r
      """
    Then the result should be:
      | r                      |
      | [:REL {property2: 24}] |
    And the side effects should be:
      | -properties | 1 |

@skip
  Scenario: Set a property
    Given any graph
    And having executed:
      """
      CREATE (:A {name: 'Andres'})
      """
    When executing query:
      """
      MATCH (n:A)
      WHERE n.name = 'Andres'
      SET n.name = 'Michael'
      RETURN n
      """
    Then the result should be:
      | n                      |
      | (:A {name: 'Michael'}) |
    And the side effects should be:
      | +properties | 1 |
      | -properties | 1 |

@skip
  Scenario: Set a property to an expression
    Given an empty graph
    And having executed:
      """
      CREATE (:A {name: 'Andres'})
      """
    When executing query:
      """
      MATCH (n:A)
      WHERE n.name = 'Andres'
      SET n.name = n.name + ' was here'
      RETURN n
      """
    Then the result should be:
      | n                              |
      | (:A {name: 'Andres was here'}) |
    And the side effects should be:
      | +properties | 1 |
      | -properties | 1 |

  Scenario: Set a property by selecting the node using a simple expression
    Given an empty graph
    And having executed:
      """
      CREATE (:A)
      """
    When executing query:
      """
      MATCH (n:A)
      SET (n).name = 'neo4j'
      RETURN n
      """
    Then the result should be:
      | n                    |
      | (:A {name: 'neo4j'}) |
    And the side effects should be:
      | +properties | 1 |

  Scenario: Set a property by selecting the relationship using a simple expression
    Given an empty graph
    And having executed:
      """
      CREATE ()-[:REL]->()
      """
    When executing query:
      """
      MATCH ()-[r:REL]->()
      SET (r).name = 'neo4j'
      RETURN r
      """
    Then the result should be:
      | r                      |
      | [:REL {name: 'neo4j'}] |
    And the side effects should be:
      | +properties | 1 |

@skip
  Scenario: Setting a property to null removes the property
    Given an empty graph
    And having executed:
      """
      CREATE (:A {name: 'Michael', age: 35})
      """
    When executing query:
      """
      MATCH (n)
      WHERE n.name = 'Michael'
      SET n.name = null
      RETURN n
      """
    Then the result should be:
      | n              |
      | (:A {age: 35}) |
    And the side effects should be:
      | -properties | 1 |

@skip
  Scenario: Add a label to a node
    Given an empty graph
    And having executed:
      """
      CREATE (:A)
      """
    When executing query:
      """
      MATCH (n:A)
      SET n:Foo
      RETURN n
      """
    Then the result should be:
      | n        |
      | (:A:Foo) |
    And the side effects should be:
      | +labels | 1 |

@skip
  Scenario: Adding a list property
    Given an empty graph
    And having executed:
      """
      CREATE (:A)
      """
    When executing query:
      """
      MATCH (n:A)
      SET n.numbers = [1, 2, 3]
      RETURN [i IN n.numbers | i / 2.0] AS x
      """
    Then the result should be:
      | x               |
      | [0.5, 1.0, 1.5] |
    And the side effects should be:
      | +properties | 1 |

@skip
  Scenario: Concatenate elements onto a list property
    Given any graph
    When executing query:
      """
      CREATE (a {numbers: [1, 2, 3]})
      SET a.numbers = a.numbers + [4, 5]
      RETURN a.numbers
      """
    Then the result should be:
      | a.numbers       |
      | [1, 2, 3, 4, 5] |
    And the side effects should be:
      | +nodes      | 1 |
      | +properties | 1 |

@skip
  Scenario: Concatenate elements in reverse onto a list property
    Given any graph
    When executing query:
      """
      CREATE (a {numbers: [3, 4, 5]})
      SET a.numbers = [1, 2] + a.numbers
      RETURN a.numbers
      """
    Then the result should be:
      | a.numbers       |
      | [1, 2, 3, 4, 5] |
    And the side effects should be:
      | +nodes      | 1 |
      | +properties | 1 |

@skip
  Scenario: Overwrite values when using +=
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
    Then the result should be:
      | n                             |
      | (:X {name: 'A', name2: 'C'}) |
    And the side effects should be:
      | +properties | 1 |
      | -properties | 1 |

@skip
  Scenario: Retain old values when using +=
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
    Then the result should be:
      | n                             |
      | (:X {name: 'A', name2: 'B'}) |
    And the side effects should be:
      | +properties | 1 |

@skip
  Scenario: Explicit null values in a map remove old values
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
    Then the result should be:
      | n                 |
      | (:X {name2: 'B'}) |
    And the side effects should be:
      | -properties | 1 |

@skip
  Scenario: Non-existent values in a property map are removed with SET =
    Given an empty graph
    And having executed:
      """
      CREATE (:X {name: 'A', name2: 'B'})
      """
    When executing query:
      """
      MATCH (n:X {name: 'A'})
      SET n = {name: 'B', baz: 'C'}
      RETURN n
      """
    Then the result should be:
      | n                           |
      | (:X {name: 'B', baz: 'C'}) |
    And the side effects should be:
      | +properties | 2 |
      | -properties | 2 |
