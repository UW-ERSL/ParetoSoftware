//this is sample usage

namespace ivbool3d{
class VecBoolean
{
	Object**m_items;
	int m_count;
	int m_limit;
	Processor*m_proc;
public:
	
	VecBoolean(IVBOOLCREATEPROC proc);
	~VecBoolean();
	int Insert();
	int Process(int i0,int i1,boolean_mode mode,unsigned int flags);
	int Polygonize(int i0);
	Object*Get(int index);
};
};

