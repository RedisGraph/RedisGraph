WITH [2, 2.9] AS numbers
RETURN [n IN numbers | toInteger(n)] AS int_numbers