/**
*	@file mem.h
*
*	Memory manager unix/palm
*
*	@author Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

/**
*	@defgroup mem memory manager
*/

#if !defined(_sb_mem_h)
#define	_sb_mem_h

#if defined(MALLOC_LIMITED)
#include <string.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_PalmOS)
	#define memset(a,b,c)	MemSet((a),(c),(b))			/**< standard memset()			@ingroup mem */
#endif

#if !defined(NULL)
	#define NULL	(void*)0L
#endif

#define ENABLE_TMPLIST

#if defined(_Win32)
#include <winsock2.h>	// there are conflicts for data types: byte, int32
#endif

/*
*	data-types
*/
#if defined(MALLOC_LIMITED)
typedef	unsigned long	MemHandle;
#define	MemPtrNew(x)	malloc(x)
#define	MemPtrFree(x)	free(x)
#endif
typedef	MemHandle		mem_t;

//#if !defined(_Win32)	// ???
//typedef	unsigned char	byte;
//#endif
#define byte	unsigned char
typedef	byte*			byte_p_t;
typedef	char*			char_p_t;

// 16-bit integer
typedef	int short		int16;
typedef	unsigned short	word;

// 32-bit integer
#if defined(_PalmOS)
typedef	long			int32;
typedef	unsigned long	dword;
#else	/* Unix, Win32 */
#if !defined(_Win32)
typedef	int				int32;
#endif
typedef	unsigned int	dword;
#endif

// old code
#if !defined(_Win32) && !defined(_VTOS) && !defined(__CYGWIN__)
typedef	dword	DWORD;
typedef	word	WORD;
typedef	byte	BYTE;
#endif

// code 
typedef byte        	code_t;		/**< basic code unit 			(unsigned)		@ingroup mem */

#if	defined(OS_ADDR16)				// work with 16bits
#define	ADDR16						// just for short defs

typedef int16			fcode_t;	// buildin function code 		(signed)
typedef int16			pcode_t;	// buildin procedure code		(signed)
typedef word        	addr_t;		// memory address				(unsigned)
#define INVALID_ADDR    0xFFFF		// invalid address value		(unsigned)
typedef int16        	bid_t;		// IDs (labels, variables, etc)	(signed)
#define	OS_ADDRSZ		2			// size of address pointer (always 2 for 16b mode)
#define	OS_CODESZ		2			// size of buildin func/proc ptrs (always 2 for 16b mode)
#define	OS_STRLEN		2			// size of strings

#else
#if defined(__BORLANDC__)

// that is a real shit... int32 is defined on winsock.h!
#define	int32			int

typedef int				fcode_t;
typedef int				pcode_t;
typedef int				bid_t;
typedef unsigned int	addr_t;
#else
typedef int32			fcode_t;
typedef int32			pcode_t;
typedef int32			bid_t;

#ifndef __CYGWIN__
typedef dword	        addr_t;
#endif

#endif
#define INVALID_ADDR    0xFFFFFFFF
#define	OS_ADDRSZ		4			// size of address pointer (always 4 for 32b addresses)
#define	OS_CODESZ		4			// size of buildin func/proc ptrs (always 4 for 32b mode)
#define	OS_STRLEN		4			// size of strings

#endif

// ---------------------------------------------

#define	ADDRSZ			OS_ADDRSZ
#define	CODESZ			OS_CODESZ
#define	BC_CTRLSZ		(ADDRSZ+ADDRSZ)

#if defined(OS_PREC64)
#define	OS_INTSZ		8			// size of integer
#define	OS_REALSZ		16			// size of real
#else
#define	OS_INTSZ		4			// size of integer
#define	OS_REALSZ		8			// size of real
#endif

// ---------------------------------------------
#if !defined(_FRANKLIN_EBM)

// Dynamic RAM
#if defined(_PalmOS) || defined(MALLOC_LIMITED)
/**
*	@ingroup mem
*
*	allocate a memory block
*
*	@param size is the block's size
*	@return a pointer to the block
*/
void	*tmp_alloc(dword size);

/**
*	@ingroup mem
*
*	frees a memory block
*
*	@param ptr a pointer to the block
*/
void	tmp_free(void *ptr);
#else
#define	tmp_alloc(s)	tmp_allocX((s), __FILE__, __LINE__)
#define	tmp_free(p)		tmp_freeX((p), __FILE__, __LINE__)

/**
*	@ingroup mem
*
*	allocate a memory block (use tmp_alloc())
*
*	@param size is the block's size
*	@return a pointer to the block
*/
void	*tmp_allocX(dword size, const char *file, int line);

/**
*	@ingroup mem
*
*	frees a memory block (use tmp_free())
*
*	@param ptr a pointer to the block
*/
void	tmp_freeX(void *ptr, const char *file, int line);
#endif

/**
*	@ingroup mem
*
*	reallocate
*
*	@param ptr a pointer to the block
*	@param size is the size of the new block
*	@return a pointer to newly allocated block
*/
void    *tmp_realloc(void *ptr, dword size);

/**
*	@ingroup mem
*
*	clones a string
*
*	@param str the string
*	@return a pointer to newly allocated string
*/
char	*tmp_strdup(const char *source);

// Storage RAM
#if defined(_PalmOS) || defined(MALLOC_LIMITED)

/**
*	@ingroup mem
*
*	allocate a memory block
*
*	@param size is the block's size
*	@return a handle to the block
*/
mem_t	mem_alloc(dword size);
#else
#define	mem_alloc(s)	mem_allocX((s), __FILE__, __LINE__)

/**
*	@ingroup mem
*
*	allocate a memory block (use mem_alloc())
*
*	@param size is the block's size
*	@return a handle to the block
*/
mem_t	mem_allocX(dword size, const char *file, int line);
#endif

/**
*	@ingroup mem
*
*	reallocates a memory block
*
*	@param handle the block's handle
*	@param size is the size of the new block
*	@return a handle to the new block
*/
mem_t	mem_realloc(mem_t handle, dword new_size);

/**
*	@ingroup mem
*
*	locks a memory block (in theory, it moves the block to application's space)
*
*	@param handle the block's handle
*	@return a pointer to the block
*/
void*	mem_lock(mem_t h);

/**
*	@ingroup mem
*
*	locks a memory block (in theory, it moves the block back to storage device)
*
*	@param handle the block's handle
*/
void	mem_unlock(mem_t h);

/**
*	@ingroup mem
*
*	frees a memory block
*
*	@param handle the block's handle
*/
#if defined(_PalmOS) || defined(MALLOC_LIMITED)
void	mem_free(mem_t h);
#else
void	mem_freeX(mem_t h, const char *file, int line);
#define	mem_free(h)		mem_freeX(h, __FILE__, __LINE__)
#endif

/**
*	@ingroup mem
*
*	clones a string by using mem_alloc()
*
*	@param str the string
*	@return a handle to newly allocated string
*/
mem_t	mem_new_text(const char *text);

/**
*	@ingroup mem
*
*	returns the size of the memory bloc
*
*	@param h the handle
*	@return the size of the memory bloc
*/
dword	mem_handle_size(mem_t h);

#endif // _FRANKLIN_EBM

#if defined(MALLOC_LIMITED)
dword	MemPtrSize(void *ptr);
dword	MemHandleSize(mem_t h);
#endif

/*
*	standard list (with handles) --- dynamic single-linked list, stored in STORAGE RAM
*/
//#define ENABLE_MEMLIST
#if defined(ENABLE_MEMLIST)

/**
*	@ingroup mem
*
* 	@struct mem_list_node_s
*
*	all-purpose list using memory-handles. node structure
*/
struct mem_list_node_s	{
	mem_t	data;		  			/**< handle to memory block */
	struct mem_list_node_s *next;	/**< pointer to next node */
	};
typedef struct mem_list_node_s	mnode_t;

/**
*	@ingroup mem
*
* 	@struct mem_list_s
*
*	all-purpose list using memory-handles.
*/
struct mem_list_s	{
	mnode_t		*head, *tail;		/**< head and tail pointers */
	int			count;				/**< the number of the elements */
	};
typedef struct mem_list_s mlist_t;

/**
*	@ingroup mem
*
*	create an all-purpose list.
*	these lists are using handle-related allocation routines.
*
*	@param lst the list	
*/
void	mlist_init(mlist_t *lst)			SEC(TRASH);

/**
*	@ingroup mem
*
*	destroys a list.
*	also, frees all memory blocks that are stored in the list.
*
*	@param lst the list	
*/
void	mlist_clear(mlist_t *lst)			SEC(TRASH);	

/**
*	@ingroup mem
*
*	adds an element to the list. 
*	the memory block will not be copied, so, do not free it.
*
*	@param lst the list	
*	@param h the handle to be stored
*/
mnode_t	*mlist_add(mlist_t *lst, mem_t h)	SEC(TRASH);
#endif


/*
*	temporary list --- dynamic single-linked list, stored in DYNAMIC RAM
*/
//#define ENABLE_TMPLIST
#if defined(ENABLE_TMPLIST)

/**
*	@ingroup mem
*
* 	@struct tmpmem_list_node_s
*
*	all-purpose list using memory-pointer. node structure
*/
struct tmpmem_list_node_s	{
	void	*data;
	struct tmpmem_list_node_s
			*next;
	};
typedef struct tmpmem_list_node_s	tmpnode_t;

/**
*	@ingroup mem
*
* 	@struct tmpmem_list_s
*
*	all-purpose list using malloc().
*/
struct tmpmem_list_s	{
	tmpnode_t	*head, *tail;
	int			count;
	};
typedef struct tmpmem_list_s tmplist_t;

/**
*	@ingroup mem
*
*	create an all-purpose temporary list.
*	temporary lists are using non-handle-related allocation routines.
*
*	@param lst the list	
*/
void	tmplist_init(tmplist_t *lst)		SEC(TRASH);

/**
*	@ingroup mem
*
*	destroys a temporary list.
*	also, frees all memory blocks that are stored in the list.
*
*	@param lst the list	
*/
void	tmplist_clear(tmplist_t *lst)		SEC(TRASH);

/**
*	@ingroup mem
*
*	adds an element to the list.
*	the memory block will not be copied, so, do not free it.
*
*	@param lst the list	
*	@param ptr the data pointer
*	@param size the size of the data
*/
tmpnode_t	*tmplist_add(tmplist_t *lst, void *ptr, int size)	SEC(TRASH);
#endif


/**
*	@ingroup mem
*
*	write to logfile
*
*	@param buf is the string to write
*/
void	lwrite(const char *buf)				SEC(TRASH);

/**
*	@ingroup mem
*
*	write to logfile (printf-style)
*
*	@param fmt is the format
*	@param ... the format's parameters
*/
void	lprintf(const char *fmt, ...)		SEC(TRASH);

/**
*	@ingroup mem
*
*	write to logfile (printf-style).
*	it is supposed to add more info about the system.
*
*	@param fmt is the format
*	@param ... the format's parameters
*/
void	lg(const char *fmt, ...)			SEC(TRASH);

// Virtual memory
//#define ENABLE_VMM
#if defined(ENABLE_VMM)
/**
*	@ingroup mem
*
*	initialize virtual-memory
*/
void	vm_init(void)						SEC(TRASH);

/**
*	@ingroup mem
*
*	close virtual-memory
*/
void	vm_close(void)						SEC(TRASH);

/**
*	@ingroup mem
*
*	vm allocate memory (using handle)
*
*	@param size is the block's size
*	@return on success the handle; otherwise returns -1	
*/
int		vm_halloc(word size)				SEC(TRASH);

/**
*	@ingroup mem
*
*	vm release memory (using handle)
*
*	@param idx the memory-block handle
*/
void	vm_hfree(int idx)					SEC(TRASH);

/**
*	@ingroup mem
*
*	vm lock a memory block. maps the memory block to process's memory
*
*	@param idx the memory-block handle
*	@return the pointer
*/
void*	vm_lock(int idx)					SEC(TRASH);

/**
*	@ingroup mem
*
*	vm unlock a memory block. moves the memory block back to storage device.
*
*	@param idx the memory-block handle
*/
void	vm_unlock(int idx)					SEC(TRASH);

void*	vm_alloc(word size)					SEC(TRASH);
void	vm_free(void*)						SEC(TRASH);
#endif

#if defined(__cplusplus)
}
#endif

#include "vmt.h"

#endif

