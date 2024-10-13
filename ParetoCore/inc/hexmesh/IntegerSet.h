#ifndef _INTEGERSET_
#define _INTEGERSET_

#define MEM_BLOCK_SET 10

class IntegerSet {
private:
	int			nElements;
	int			*elements;
	int			currentMemSize;
public:
			     IntegerSet(void);
	void		 init(void);
	bool		 isEmpty(void);
	void		 insertCheckForUnique(int a);
	void		 insertNoCheckForUnique(int a);
	void		 print(void);
	void		 clear(void);
	int			 size(void);
	int			 minElement(void);
	int			 find(int a);
	int			 elementAt(int i);
				 ~IntegerSet();	
};

#endif