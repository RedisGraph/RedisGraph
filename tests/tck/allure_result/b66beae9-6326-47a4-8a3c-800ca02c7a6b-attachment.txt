WITH [3.4, 3] AS numbers
RETURN [n IN numbers | toFloat(n)] AS float_numbers