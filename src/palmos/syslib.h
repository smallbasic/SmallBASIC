#if !defined(__syslib_h)
#define	__syslib_h

#include <sys.h>

#define	R_OK	4
#define	W_OK	2
#define	X_OK	1
#define	F_OK	0

#define sprintf		StrPrintF
typedef unsigned long time_t;

//
void	srand(unsigned int seed)	SEC(TRASH);

//
typedef long	clock_t;
#define	CLOCKS_PER_SEC	SysTicksPerSecond()
clock_t	clock();

//
int		access(const char *file, int mode);

//
struct stat {
	/* dev_t */ int         st_dev;      /* device */
	/* ino_t */ int         st_ino;      /* inode */
	/* mode_t */ int        st_mode;     /* protection */
	/* nlink_t */ int       st_nlink;    /* number of hard links */
	/* uid_t */ int         st_uid;      /* user ID of owner */
	/* gid_t */ int         st_gid;      /* group ID of owner */
	/* dev_t */ int         st_rdev;     /* device type (if inode device) */
	/* off_t */ long         st_size;     /* total size, in bytes */
	/* blksize_t */ int     st_blksize;  /* blocksize for filesystem I/O */
	/* blkcnt_t */ int      st_blocks;   /* number of blocks allocated */
	time_t        st_atime;    /* time of last access */
	time_t        st_mtime;    /* time of last modification */
	time_t        st_ctime;    /* time of last change */
	};

int		stat(const char *filename, struct stat *buf) SEC(TRASH);
char	*getenv(const char *name) SEC(TRASH);
int		putenv(char *str) SEC(TRASH);

#define strcasecmp(a,b)	StrCaselessCompare((a),(b))
#define strncasecmp(a,b,c)	StrNCaselessCompare((a),(b),(c))

#endif

