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

Feature: Comparability

    Scenario: Comparing strings and integers using > in an AND'd predicate
        Given an empty graph
        And having executed:
            """
            CREATE (root:Root)-[:T]->(:Child {var: 0}),
            (root)-[:T]->(:Child {var: 'xx'}),
            (root)-[:T]->(:Child)
            """
        When executing query:
            """
            MATCH (:Root)-->(i:Child)
            WHERE exists(i.var) AND i.var > 'x'
            RETURN i.var
            """
        Then the result should be:
            | i.var |
            | 'xx'  |
        And no side effects


    Scenario: Comparing strings and integers using > in a OR'd predicate
        Given an empty graph
        And having executed:
            """
            CREATE (root:Root)-[:T]->(:Child {var: 0}),
            (root)-[:T]->(:Child {var: 'xx'}),
            (root)-[:T]->(:Child)
            """
        When executing query:
            """
            MATCH (:Root)-->(i:Child)
            WHERE NOT exists(i.var) OR i.var > 'x'
            RETURN i.var
            """
        Then the result should be:
            | i.var |
            | 'xx'  |
            | null  |
        And no side effects

    @skip
    Scenario Outline: Comparing across types yields null, except numbers
        Given an empty graph
        And having executed:
            """
            CREATE ()-[:T]->()
            """
        When executing query:
            """
            MATCH p = (n)-[r]->()
            WITH [n, r, p, '', 1, 3.14, true, null, [], {}] AS types
            UNWIND range(0, size(types) - 1) AS i
            UNWIND range(0, size(types) - 1) AS j
            WITH types[i] AS lhs, types[j] AS rhs
            WHERE i <> j
            WITH lhs, rhs, lhs <operator> rhs AS result
            WHERE result
            RETURN lhs, rhs
            """
        Then the result should be:
            | lhs   | rhs   |
            | <lhs> | <rhs> |
        And no side effects

        Examples:
            | operator | lhs  | rhs  |
            | <        | 1    | 3.14 |
            | <=       | 1    | 3.14 |
            | >=       | 3.14 | 1    |
            | >        | 3.14 | 1    |


    Scenario Outline: Comparing lists
        Given an empty graph
        When executing query:
            """
            RETURN <lhs> >= <rhs> AS result
            """
        Then the result should be:
            | result   |
            | <result> |
        And no side effects

        Examples:
            | lhs       | rhs       | result |
            | [1, 0]    | [1]       | true   |
            | [1, null] | [1]       | true   |
            | [1, 2]    | [1, null] | null   |
            | [1, 'a']  | [1, null] | null   |
            | [1, 2]    | [3, null] | false  |
