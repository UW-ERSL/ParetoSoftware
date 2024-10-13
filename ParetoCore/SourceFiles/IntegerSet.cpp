#include "IntegerSet.h"
#include "stdlib.h"
#include "stdio.h"

//******************************************************************
//						IntegerSet
//******************************************************************
IntegerSet::IntegerSet(void)
{
	init();
}
//******************************************************************
//						IntegerSet
//******************************************************************
void IntegerSet::init(void)
{
	nElements = 0;
	elements = (int *) malloc(1*sizeof(int));
	currentMemSize = 1;
}
//******************************************************************
//						IntegerSet
//******************************************************************
void IntegerSet::insertCheckForUnique(int a)
{
	for (int i = 0; i < nElements; i++) {
		if (elements[i] == a)
			return; // no double entries in a set
	}
	if (nElements == currentMemSize) {
		elements = (int *) realloc(elements,(currentMemSize+MEM_BLOCK_SET)*sizeof(int));
		currentMemSize += MEM_BLOCK_SET;
	}
	// Not found
	elements[nElements] = a;
	nElements++;
}
//******************************************************************
//						IntegerSet
//******************************************************************
void IntegerSet::insertNoCheckForUnique(int a)
// Same as insertCheckForUnique except we do not verify if the element already exists
{
	if (nElements == currentMemSize) {
		elements = (int *) realloc(elements,(currentMemSize+MEM_BLOCK_SET)*sizeof(int));
		currentMemSize += MEM_BLOCK_SET;
		
	}
	// Not found
	elements[nElements] = a;
	nElements++;
}
//******************************************************************
//						isEmpty
//******************************************************************
bool IntegerSet::isEmpty(void)
{
	if (nElements == 0)
		return true;
	else
		return false;
}
//******************************************************************
//						clear
//******************************************************************
void IntegerSet::clear(void)
{
	nElements = 0;
	if (elements) free(elements);
	elements = (int *) malloc(1*sizeof(int));
	currentMemSize = 1;
}
//******************************************************************
//						size
//******************************************************************
int IntegerSet::size(void)
{
	return (nElements);
}

//******************************************************************
//						minElement
//******************************************************************
int IntegerSet::minElement(void)
{
	int minVal = elements[0];
	for (int i = 1; i < nElements; i++) 
		minVal = (elements[i] <minVal)? elements[i]:minVal;
	return (minVal);
}

//******************************************************************
//						find
//******************************************************************
int IntegerSet::find(int a)
{
	for (int i = 0; i < nElements; i++) {
		if (elements[i] == a)
			return (i); 
	}
	return (-1);
}
//******************************************************************
//						elementAt
//******************************************************************
int IntegerSet::elementAt(int i)
{
	if ((i <0) || (i >= nElements)) {
		//printf("IntegerSet: Accessing an element outside the set\n");
		return (0);
	}

	return (elements[i]);
}
//******************************************************************
//						IntegerSet
//******************************************************************
void IntegerSet::print(void)
{
	for (int i = 0; i < nElements; i++) 
		printf("%d: %d \n",i,elements[i]);
}
//******************************************************************
//						~IntegerSet
//******************************************************************
IntegerSet::~IntegerSet()
{
	free(elements);
}
