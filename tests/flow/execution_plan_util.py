from redisgraph.execution_plan import Operation, ExecutionPlan

def locate_operation(op: Operation, name: str):
    if op.name == name:
        return op
    if op.children:
        for child in op.children:
            res = locate_operation(child, name)
            if res:
                return res
    return None
