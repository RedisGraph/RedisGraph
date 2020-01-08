WITH [1, 2, 3] AS numbers
RETURN [n IN numbers | toString(n)] AS string_numbers