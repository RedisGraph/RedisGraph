
from redis.commands.graph.execution_plan import Operation, ExecutionPlan


def locate_operation(op: Operation, name: str):
    if op.name == name:
        return op
    if op.children:
        for child in op.children:
            res = locate_operation(child, name)
            if res:
                return res
    return None

# Iterate recursively the operation's children and return the number of operations with a specific name
def count_operation(op: Operation, name: str):
    res = 0
    if op.name == name:
        res = 1
    if op.children:
        for child in op.children:
            res += count_operation(child, name)
    return res
