UNWIND ['a', 'b', 'B', null, 'abc', 'abc1'] AS i
RETURN max(i)