Feature: Calculator

  Scenario: Addition
    Given I have calculator
     When I enter 40
     When I enter 2
     When I press add
     Then I expect 42

  Scenario: Subtraction
    Given I have calculator
     When I enter 45
     When I enter 2
     When I enter 1
     When I press sub
     Then I expect 42

