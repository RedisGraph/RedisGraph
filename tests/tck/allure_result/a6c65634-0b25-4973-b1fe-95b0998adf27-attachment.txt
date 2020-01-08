UNWIND split('one1two', '1') AS item
RETURN count(item) AS item