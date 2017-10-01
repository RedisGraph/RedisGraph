class QueryInfo(object):
    """
    This class contains the needed data about a query
    """

    def __init__(self, query=None, description=None, max_run_time_ms=None, expected_result=None):
        """
        QueryInfo contructor

        :param query: The query itself (string)
        :param description: The information about what the query does (string)
        :param max_run_time_ms: The max run time of the query in milliseconds (float)
        :param expected_result: The expected result of the query (list of lists, where the first list
                                is the columns names, and the rest is the result)
        """

        self.query = query
        self.description = description
        self.expected_result = expected_result
        self.max_run_time_ms = max_run_time_ms
