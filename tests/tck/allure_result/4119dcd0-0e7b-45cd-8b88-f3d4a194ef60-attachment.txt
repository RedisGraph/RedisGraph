UNWIND [null, '', ' tru ', 'f alse'] AS things
RETURN toBoolean(things) AS b