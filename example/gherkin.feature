Feature: Calculator

  Background:
    Given I have calculator

  Scenario: Addition
    When I enter 40
    And I enter 2
    And I press add
    Then I expect 42

  Scenario Outline: Addition of <a> and <b>
    When I enter <a>
    But I enter <b>
    * I press add
    Then I expect <sum>

    Examples:
      | a  | b  | sum |
      | 1  | 2  | 3   |
      | 40 | 2  | 42  |

  Scenario: Sum of a table
    When I enter the following values:
      | 1 | 2 | 3 |
      | 4 | 5 | 6 |
    And I press add
    Then I expect 21

  Scenario: Subtraction
    When I enter 45
    And I enter 2
    And I enter 1
    And I press sub
    Then I expect 42
