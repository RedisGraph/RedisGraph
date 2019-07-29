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

Feature: LabelsAcceptance

  Background:
    Given an empty graph

@skip
  Scenario: Adding a single label
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      SET n:Foo
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n) |
      | ['Foo']   |
    And the side effects should be:
      | +labels | 1 |

@skip
  Scenario: Ignore space before colon
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      SET n :Foo
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n) |
      | ['Foo']   |
    And the side effects should be:
      | +labels | 1 |

@skip
  Scenario: Adding multiple labels
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      SET n:Foo:Bar
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n)      |
      | ['Foo', 'Bar'] |
    And the side effects should be:
      | +labels | 2 |

@skip
  Scenario: Ignoring intermediate whitespace 1
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      SET n :Foo :Bar
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n)      |
      | ['Foo', 'Bar'] |
    And the side effects should be:
      | +labels | 2 |

@skip
  Scenario: Ignoring intermediate whitespace 2
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      SET n :Foo:Bar
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n)      |
      | ['Foo', 'Bar'] |
    And the side effects should be:
      | +labels | 2 |

@skip
  Scenario: Creating node without label
    When executing query:
      """
      CREATE (node)
      RETURN labels(node)
      """
    Then the result should be:
      | labels(node) |
      | []           |
    And the side effects should be:
      | +nodes | 1 |

@skip
  Scenario: Creating node with two labels
    When executing query:
      """
      CREATE (node:Foo:Bar {name: 'Mattias'})
      RETURN labels(node)
      """
    Then the result should be:
      | labels(node)   |
      | ['Foo', 'Bar'] |
    And the side effects should be:
      | +nodes      | 1 |
      | +labels     | 2 |
      | +properties | 1 |

@skip
  Scenario: Ignore space when creating node with labels
    When executing query:
      """
      CREATE (node :Foo:Bar)
      RETURN labels(node)
      """
    Then the result should be:
      | labels(node)   |
      | ['Foo', 'Bar'] |
    And the side effects should be:
      | +nodes  | 1 |
      | +labels | 2 |

@skip
  Scenario: Create node with label in pattern
    When executing query:
      """
      CREATE (n:Person)-[:OWNS]->(:Dog)
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n)  |
      | ['Person'] |
    And the side effects should be:
      | +nodes         | 2 |
      | +relationships | 1 |
      | +labels        | 2 |

@skip
  Scenario: Fail when adding a new label predicate on a node that is already bound 1
    When executing query:
      """
      CREATE (n:Foo)-[:T1]->(),
             (n:Bar)-[:T2]->()
      """
    Then a SyntaxError should be raised at compile time: VariableAlreadyBound

@skip
  Scenario: Fail when adding new label predicate on a node that is already bound 2
    When executing query:
      """
      CREATE ()<-[:T2]-(n:Foo),
             (n:Bar)<-[:T1]-()
      """
    Then a SyntaxError should be raised at compile time: VariableAlreadyBound

@skip
  Scenario: Fail when adding new label predicate on a node that is already bound 3
    When executing query:
      """
      CREATE (n:Foo)
      CREATE (n:Bar)-[:OWNS]->(:Dog)
      """
    Then a SyntaxError should be raised at compile time: VariableAlreadyBound

@skip
  Scenario: Fail when adding new label predicate on a node that is already bound 4
    When executing query:
      """
      CREATE (n {})
      CREATE (n:Bar)-[:OWNS]->(:Dog)
      """
    Then a SyntaxError should be raised at compile time: VariableAlreadyBound

@skip
  Scenario: Fail when adding new label predicate on a node that is already bound 5
    When executing query:
      """
      CREATE (n:Foo)
      CREATE (n {})-[:OWNS]->(:Dog)
      """
    Then a SyntaxError should be raised at compile time: VariableAlreadyBound

@skip
  Scenario: Using `labels()` in return clauses
    And having executed:
      """
      CREATE ()
      """
    When executing query:
      """
      MATCH (n)
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n) |
      | []        |
    And no side effects

@skip
  Scenario: Removing a label
    And having executed:
      """
      CREATE (:Foo:Bar)
      """
    When executing query:
      """
      MATCH (n)
      REMOVE n:Foo
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n) |
      | ['Bar']   |
    And the side effects should be:
      | -labels | 1 |

@skip
  Scenario: Removing a non-existent label
    And having executed:
      """
      CREATE (:Foo)
      """
    When executing query:
      """
      MATCH (n)
      REMOVE n:Bar
      RETURN labels(n)
      """
    Then the result should be:
      | labels(n) |
      | ['Foo']   |
    And no side effects
