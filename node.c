#include "node.h"
#include "edge.h"	// Not sure about this include.

Node* NewNode(const char* name) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->filters = NewVector(PropertyFilter*, 0);
	node->outgoingEdges = NewVector(Edge*, 0);
	node->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));

	strcpy(node->name, name);

	return node;
}

PropertyFilter* NewPropertyFilter(const char* property, Filter* filter) {
	PropertyFilter* propertyFilter = (PropertyFilter*)malloc(sizeof(PropertyFilter));
	propertyFilter->property = (char*)malloc(sizeof(char) * (strlen(property) + 1));

	strcpy(propertyFilter->property, property);
	propertyFilter->filter = filter;

	return propertyFilter;
}

void FreePropertyFilter(PropertyFilter* propertyFilter) {
	free(propertyFilter->property);
	FreeFilter(propertyFilter->filter);
	free(propertyFilter);
}

void NodeAddFilter(const Node* node, const char* property, Filter* filter) {
	if(ValidateFilter(filter)) {
		PropertyFilter* propertyFilter = NewPropertyFilter(property, filter);
		Vector_Push(node->filters, propertyFilter);
	}
}

int ValidateNode(const Node* node) {
	// Check if node's filters are valid.
	for (int i = 0; i < Vector_Size(node->filters); i++) {
        PropertyFilter *propertyFilter;
        Vector_Get(node->filters, i, &propertyFilter);
        if(ValidateFilter(propertyFilter->filter) == 0) {
        	return 0;
        }
    }

	return 1;
}

void FreeNode(Node* node) {
	free(node->name);

	// Free filters
	for (int i = 0; i < Vector_Size(node->filters); i++) {
        PropertyFilter* f;
        Vector_Get(node->filters, i, &f);
        FreePropertyFilter(f);
    }
    
	Vector_Free(node->filters);
	Vector_Free(node->outgoingEdges);
	free(node);
}