/*
*	PtrList
*
*	Pointer list.
*	This list stores pointers of object T. The destructor frees these pointers.
*
*	Nicholas Christopoulos
*/

#if !defined(__plist)
#define	__plist

#define PTRLIST_DEF_ALLOC	1024	// 0x1000 = 4K entries

template<class T> class PtrList {
private:
	//
	// The local list node.
	//
	class Node {
	public:
		T		*cargo;					// the cargo!
		Node	*nextPtr, *prevPtr;		// the next/prev pointers

		// list node constructor. sets the cargo and the next/prev pointers.
		Node(T *obj, Node *nextNode, Node *prevNode)
			{ cargo = obj; nextPtr = nextNode; prevPtr = prevNode; }
		};

	typedef	Node*	NodePtr;
	
public:
	// constructors
	PtrList();									// default constructor
	PtrList(const PtrList<T>&);					// constructor. copies the specified list to this list.
	virtual ~PtrList();							// destructor.

	PtrList<T>& operator=(const PtrList<T>&);	// assign operator. copies the specified list to this list.
	PtrList<T>& operator+=(const PtrList<T>&);	// append operator.

	void	clearNodes();						// clear the list.
	void	clearCargo();						// clear the node-cargo.
	void 	clear()	
				{ clearCargo(); clearNodes(); }
				
	void	add(T*);						// add the specified element at the end of the list.
	bool	empty() const;						// returns true if the list is empty 
	int		count() const	 					// returns the number of the nodes in the list.
				{ return nodeCount; } 
				
	T*		get(int idx) const;					// retrieve the 'index' element of the list
	T*		operator[](int idx)
				{ return get(idx); }

	void	setAutoDelete(bool f)	{ autoDelete = f; }

private:
	// the tail pointer
	Node	*head, *tail;

	// fast access - node table
	NodePtr	*index;
	int		idxAlloc;

	// the node counter
	int		nodeCount;

protected:
	void	resizeIndex(int newSize);
	bool	autoDelete;
	};

////////////////////////////////////////////////////////////////////////////////

// default constructor
template<class T>
PtrList<T>::PtrList()	
{
	head = tail = NULL; 
	index = new NodePtr[(idxAlloc = PTRLIST_DEF_ALLOC)];
	nodeCount = 0; 
	autoDelete = true;
}

// constructor. copies the source list to this object.
template<class T>
PtrList<T>::PtrList(const PtrList<T>& source) 
{
	head = tail = NULL;
	index = new NodePtr[(idxAlloc = PTRLIST_DEF_ALLOC)];
	nodeCount = 0;
	for ( Node *p = source.head; p; p = p->nextPtr )	
		add(p->cargo);
	autoDelete = true;
}

// clears the list. (removes all nodes)
template<class T>
void PtrList<T>::clearNodes()
{
	Node *p = head;

	// delete list
	while ( p ) {
		Node *next = p->nextPtr;
		delete p;
		p = next;
		}

	// reset object.
	head = tail = NULL;
	nodeCount = 0;

	//
	delete[] index;
	index = NULL;
	idxAlloc = 0;
}

template<class T>
void PtrList<T>::clearCargo()
{
	if	( autoDelete )	{
		Node	*p = head;
		
		while ( p ) {
			if	( p->cargo )	{
				delete p->cargo;
				p->cargo = NULL;
				}
			p = p->nextPtr;
			}
		}
}

//
template<class T>
T*	PtrList<T>::get(int idx) const
{
//	Node	*p = head;
//	int		count = 0;
//
//	while ( p ) {
//		if	( count == idx )
//			return p->cargo;
//		p = p->nextPtr;
//		count ++;
//		}
//	return head->cargo;	// null!!!!

	if	( idx >= 0 && idx < nodeCount )
		return index[idx]->cargo;
	return NULL;
}

// append operator. copies the source list to the end of this object.
template<class T>
PtrList<T>& PtrList<T>::operator+=(const PtrList<T>& source)
{
	// copy the specified list to this.
	for (Node *p = source.head; p; p = p->nextPtr )	
		add(p->cargo);

	return *this;
}

// assign operator. copies the source list to this object.
template<class T>
PtrList<T>& PtrList<T>::operator=(const PtrList<T>& source)
{
	clear();
	return this->operator+=(source);
}

// destructor.
template<class T>
PtrList<T>::~PtrList() 
{
	clear();
}

//
template<class T>
void PtrList<T>::resizeIndex(int newSize)
{
	int		i;
	NodePtr	*newTable = new NodePtr[newSize];

	if	( idxAlloc )	{
		memcpy(newTable, index, sizeof(NodePtr) * idxAlloc);
//		for ( i = 0; i < idxAlloc; i ++ )
//			newTable[i] = index[i];
		}

	delete[] index;
	
	index = newTable;
	idxAlloc = newSize;	
}

// adds the 'cargo' element to the end of the list.
template<class T>
void PtrList<T>::add(T* cargo)
{
	Node	*newNode = new Node(cargo, NULL, tail);

	if	( tail )
		tail->nextPtr = newNode;
		
	tail = newNode;
	if	( !head )
		head = tail;

	///////////////////
	// index
	if	( nodeCount >= idxAlloc )
		resizeIndex(idxAlloc + PTRLIST_DEF_ALLOC);
	index[nodeCount] = (NodePtr) newNode;
	
	//
	nodeCount ++;
}

// returns true if there is no nodes in the list.
template<class T>
bool PtrList<T>::empty() const 
{
	return (head == NULL);
}

#endif

 