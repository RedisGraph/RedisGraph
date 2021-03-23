# Regular Path Queries

Regular paths constraints allow one to use well-known regular expressions over edge labels as constraints for paths. 

Let, for example, one wants to explore ways to go from one city to another by using a car and plane or bus and train.

```
MATCH (a) - / (:bus :train* :bus) | (:car :plane :car) / -> (b)
RETURN b  
```

Here ```|``` denotes an alternative between two variants of the path, and ```*``` means zero or more repetition of the label. So, one is ready to travel by train with connections, but for plane prefer one direct flight. 