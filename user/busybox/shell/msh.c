/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* vi: set sw=4 ts=4: */
/*
 * Minix shell port for busybox
 *
 * This version of the Minix shell was adapted for use in busybox
 * by Erik Andersen <andersen@codepoet.org>
 *
 * - backtick expansion did not work properly
 *   Jonas Holmberg <jonas.holmberg@axis.com>
 *   Robert Schwebel <r.schwebel@pengutronix.de>
 *   Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include <sys/times.h>
#include <setjmp.h>

#ifdef STANDALONE
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <dirent.h>
# include <fcntl.h>
# include <ctype.h>
# include <assert.h>
# define bb_dev_null "/dev/null"
# define DEFAULT_SHELL "/proc/self/exe"
# define CONFIG_BUSYBOX_EXEC_PATH "/proc/self/exe"
# define bb_banner "busybox standalone"
# define ENABLE_FEATURE_SH_STANDALONE 0
# define bb_msg_memory_exhausted "memory exhausted"
# define xmalloc(size) malloc(size)
# define msh_main(argc,argv) main(argc,argv)
# define safe_read(fd,buf,count) read(fd,buf,count)
# define nonblock_safe_read(fd,buf,count) read(fd,buf,count)
# define NOT_LONE_DASH(s) ((s)[0] != '-' || (s)[1])
# define LONE_CHAR(s,c) ((s)[0] == (c) && !(s)[1])
# define NORETURN __attribute__ ((__noreturn__))
static int find_applet_by_name(const char *applet)
{
	return -1;
}
static char *utoa_to_buf(unsigned n, char *buf, unsigned buflen)
{
	unsigned i, out, res;
	assert(sizeof(unsigned) == 4);
	if (buflen) {
		out = 0;
		for (i = 1000000000; i; i /= 10) {
			res = n / i;
			if (res || out || i == 1) {
				if (!--buflen) break;
				out++;
				n -= res*i;
				*buf++ = '0' + res;
			}
		}
	}
	return buf;
}
static char *itoa_to_buf(int n, char *buf, unsigned buflen)
{
	if (buflen && n < 0) {
		n = -n;
		*buf++ = '-';
		buflen--;
	}
	return utoa_to_buf((unsigned)n, buf, buflen);
}
static char local_buf[12];
static char *itoa(int n)
{
	*(itoa_to_buf(n, local_buf, sizeof(local_buf))) = '\0';
	return local_buf;
}
#else
# include "busybox.h" /* for applet_names */
#endif

//#define MSHDEBUG 4

#ifdef MSHDEBUG
static int mshdbg = MSHDEBUG;

#define DBGPRINTF(x)	if (mshdbg > 0) printf x
#define DBGPRINTF0(x)	if (mshdbg > 0) printf x
#define DBGPRINTF1(x)	if (mshdbg > 1) printf x
#define DBGPRINTF2(x)	if (mshdbg > 2) printf x
#define DBGPRINTF3(x)	if (mshdbg > 3) printf x
#define DBGPRINTF4(x)	if (mshdbg > 4) printf x
#define DBGPRINTF5(x)	if (mshdbg > 5) printf x
#define DBGPRINTF6(x)	if (mshdbg > 6) printf x
#define DBGPRINTF7(x)	if (mshdbg > 7) printf x
#define DBGPRINTF8(x)	if (mshdbg > 8) printf x
#define DBGPRINTF9(x)	if (mshdbg > 9) printf x

static int mshdbg_rc = 0;

#define RCPRINTF(x)	if (mshdbg_rc) printf x

#else

#define DBGPRINTF(x)
#define DBGPRINTF0(x) ((void)0)
#define DBGPRINTF1(x) ((void)0)
#define DBGPRINTF2(x) ((void)0)
#define DBGPRINTF3(x) ((void)0)
#define DBGPRINTF4(x) ((void)0)
#define DBGPRINTF5(x) ((void)0)
#define DBGPRINTF6(x) ((void)0)
#define DBGPRINTF7(x) ((void)0)
#define DBGPRINTF8(x) ((void)0)
#define DBGPRINTF9(x) ((void)0)

#define RCPRINTF(x) ((void)0)

#endif  /* MSHDEBUG */


#if ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define DEFAULT_ROOT_PROMPT "\\u:\\w> "
# define DEFAULT_USER_PROMPT "\\u:\\w$ "
#else
# define DEFAULT_ROOT_PROMPT "# "
# define DEFAULT_USER_PROMPT "$ "
#endif


/* -------- sh.h -------- */
/*
 * shell
 */

#define LINELIM   2100
#define NPUSH     8             /* limit to input nesting */

#undef NOFILE
#define NOFILE    20            /* Number of open files */
#define NUFILE    10            /* Number of user-accessible files */
#define FDBASE    10            /* First file usable by Shell */

/*
 * values returned by wait
 */
#define	WAITSIG(s)  ((s) & 0177)
#define	WAITVAL(s)  (((s) >> 8) & 0377)
#define	WAITCORE(s) (((s) & 0200) != 0)

/*
 * library and system definitions
 */
typedef void xint;              /* base type of jmp_buf, for not broken compilers */

/*
 * shell components
 */
#define	NOBLOCK	((struct op *)NULL)
#define	NOWORD	((char *)NULL)
#define	NOWORDS	((char **)NULL)
#define	NOPIPE	((int *)NULL)

/*
 * redirection
 */
struct ioword {
	smallint io_flag;               /* action (below) */
	int io_fd;                      /* fd affected */
	char *io_name;                  /* file name */
};

#define	IOREAD	 1                      /* < */
#define	IOHERE	 2                      /* << (here file) */
#define	IOWRITE	 4                      /* > */
#define	IOCAT	 8                      /* >> */
#define	IOXHERE	 16                     /* ${}, ` in << */
#define	IODUP	 32                     /* >&digit */
#define	IOCLOSE	 64                     /* >&- */

#define	IODEFAULT (-1)                  /* "default" IO fd */


/*
 * Description of a command or an operation on commands.
 * Might eventually use a union.
 */
struct op {
	smallint op_type;               /* operation type, see Txxxx below */
	char **op_words;                /* arguments to a command */
	struct ioword **ioact;          /* IO actions (eg, < > >>) */
	struct op *left;
	struct op *right;
	char *str;                      /* identifier for case and for */
};

#define TCOM    1       /* command */
#define TPAREN  2       /* (c-list) */
#define TPIPE   3       /* a | b */
#define TLIST   4       /* a [&;] b */
#define TOR     5       /* || */
#define TAND    6       /* && */
#define TFOR    7
#define TDO     8
#define TCASE   9
#define TIF     10
#define TWHILE  11
#define TUNTIL  12
#define TELIF   13
#define TPAT    14      /* pattern in case */
#define TBRACE  15      /* {c-list} */
#define TASYNC  16      /* c & */
/* Added to support "." file expansion */
#define TDOT    17

/* Strings for names to make debug easier */
#ifdef MSHDEBUG
static const char *const T_CMD_NAMES[] = {
	"PLACEHOLDER",
	"TCOM",
	"TPAREN",
	"TPIPE",
	"TLIST",
	"TOR",
	"TAND",
	"TFOR",
	"TDO",
	"TCASE",
	"TIF",
	"TWHILE",
	"TUNTIL",
	"TELIF",
	"TPAT",
	"TBRACE",
	"TASYNC",
	"TDOT",
};
#endif

#define AREASIZE (90000)

/*
 * flags to control evaluation of words
 */
#define DOSUB    1      /* interpret $, `, and quotes */
#define DOBLANK  2      /* perform blank interpretation */
#define DOGLOB   4      /* interpret [?* */
#define DOKEY    8      /* move words with `=' to 2nd arg. list */
#define DOTRIM   16     /* trim resulting string */

#define DOALL    (DOSUB|DOBLANK|DOGLOB|DOKEY|DOTRIM)


struct brkcon {
	jmp_buf brkpt;
	struct brkcon *nextlev;
};


static smallint trapset;                        /* trap pending (signal number) */

static smallint yynerrs;                        /* yacc (flag) */

/* moved to G: static char line[LINELIM]; */

#if ENABLE_FEATURE_EDITING
static char *current_prompt;
static line_input_t *line_input_state;
#endif


/*
 * other functions
 */
static const char *rexecve(char *c, char **v, char **envp);
static char *evalstr(char *cp, int f);
static char *putn(int n);
static char *unquote(char *as);
static int rlookup(char *n);
static struct wdblock *glob(char *cp, struct wdblock *wb);
static int my_getc(int ec);
static int subgetc(char ec, int quoted);
static char **makenv(int all, struct wdblock *wb);
static char **eval(char **ap, int f);
static int setstatus(int s);
static int waitfor(int lastpid, int canintr);

static void onintr(int s);		/* SIGINT handler */

static int newenv(int f);
static void quitenv(void);
static void next(int f);
static void setdash(void);
static void onecommand(void);
static void runtrap(int i);


/* -------- area stuff -------- */

#define REGSIZE   sizeof(struct region)
#define GROWBY    (256)
/* #define SHRINKBY (64) */
#undef  SHRINKBY
#define FREE      (32767)
#define BUSY      (0)
#define ALIGN     (sizeof(int)-1)


struct region {
	struct region *next;
	int area;
};


/* -------- grammar stuff -------- */
typedef union {
	char *cp;
	char **wp;
	int i;
	struct op *o;
} YYSTYPE;

#define WORD    256
#define LOGAND  257
#define LOGOR   258
#define BREAK   259
#define IF      260
#define THEN    261
#define ELSE    262
#define ELIF    263
#define FI      264
#define CASE    265
#define ESAC    266
#define FOR     267
#define WHILE   268
#define UNTIL   269
#define DO      270
#define DONE    271
#define IN      272
/* Added for "." file expansion */
#define DOT     273

#define	YYERRCODE 300

/* flags to yylex */
#define	CONTIN 01     /* skip new lines to complete command */

static struct op *pipeline(int cf);
static struct op *andor(void);
static struct op *c_list(void);
static int synio(int cf);
static void musthave(int c, int cf);
static struct op *simple(void);
static struct op *nested(int type, int mark);
static struct op *command(int cf);
static struct op *dogroup(int onlydone);
static struct op *thenpart(void);
static struct op *elsepart(void);
static struct op *caselist(void);
static struct op *casepart(void);
static char **pattern(void);
static char **wordlist(void);
static struct op *list(struct op *t1, struct op *t2);
static struct op *block(int type, struct op *t1, struct op *t2, char **wp);
static struct op *newtp(void);
static struct op *namelist(struct op *t);
static char **copyw(void);
static void word(char *cp);
static struct ioword **copyio(void);
static struct ioword *io(int u, int f, char *cp);
static int yylex(int cf);
static int collect(int c, int c1);
static int dual(int c);
static void diag(int ec);
static char *tree(unsigned size);

/* -------- var.h -------- */

struct var {
	char *value;
	char *name;
	struct var *next;
	char status;
};

#define	COPYV	1				/* flag to setval, suggesting copy */
#define	RONLY	01				/* variable is read-only */
#define	EXPORT	02				/* variable is to be exported */
#define	GETCELL	04				/* name & value space was got with getcell */

static int yyparse(void);


/* -------- io.h -------- */
/* io buffer */
struct iobuf {
	unsigned id;            /* buffer id */
	char buf[512];          /* buffer */
	char *bufp;             /* pointer into buffer */
	char *ebufp;            /* pointer to end of buffer */
};

/* possible arguments to an IO function */
struct ioarg {
	const char *aword;
	char **awordlist;
	int afile;              /* file descriptor */
	unsigned afid;          /* buffer id */
	off_t afpos;            /* file position */
	struct iobuf *afbuf;    /* buffer for this file */
};

/* an input generator's state */
struct io {
	int (*iofn) (struct ioarg *, struct io *);
	struct ioarg *argp;
	int peekc;
	char prev;              /* previous character read by readc() */
	char nlcount;           /* for `'s */
	char xchar;             /* for `'s */
	char task;              /* reason for pushed IO */
};
/* ->task: */
#define	XOTHER	0	/* none of the below */
#define	XDOLL	1	/* expanding ${} */
#define	XGRAVE	2	/* expanding `'s */
#define	XIO	3	/* file IO */


/*
 * input generators for IO structure
 */
static int nlchar(struct ioarg *ap);
static int strchar(struct ioarg *ap);
static int qstrchar(struct ioarg *ap);
static int filechar(struct ioarg *ap);
static int herechar(struct ioarg *ap);
static int linechar(struct ioarg *ap);
static int gravechar(struct ioarg *ap, struct io *iop);
static int qgravechar(struct ioarg *ap, struct io *iop);
static int dolchar(struct ioarg *ap);
static int wdchar(struct ioarg *ap);
static void scraphere(void);
static void freehere(int area);
static void gethere(void);
static void markhere(char *s, struct ioword *iop);
static int herein(char *hname, int xdoll);
static int run(struct ioarg *argp, int (*f) (struct ioarg *));


static int eofc(void);
static int readc(void);
static void unget(int c);
static void ioecho(char c);


/*
 * IO control
 */
static void pushio(struct ioarg *argp, int (*f) (struct ioarg *));
#define PUSHIO(what,arg,gen) ((temparg.what = (arg)), pushio(&temparg,(gen)))
static int remap(int fd);
static int openpipe(int *pv);
static void closepipe(int *pv);
static struct io *setbase(struct io *ip);

/* -------- word.h -------- */

#define	NSTART	16				/* default number of words to allow for initially */

struct wdblock {
	short w_bsize;
	short w_nword;
	/* bounds are arbitrary */
	char *w_words[1];
};

static struct wdblock *addword(char *wd, struct wdblock *wb);
static struct wdblock *newword(int nw);
static char **getwords(struct wdblock *wb);

/* -------- misc stuff -------- */

static int dolabel(struct op *t, char **args);
static int dohelp(struct op *t, char **args);
static int dochdir(struct op *t, char **args);
static int doshift(struct op *t, char **args);
static int dologin(struct op *t, char **args);
static int doumask(struct op *t, char **args);
static int doexec(struct op *t, char **args);
static int dodot(struct op *t, char **args);
static int dowait(struct op *t, char **args);
static int doread(struct op *t, char **args);
static int doeval(struct op *t, char **args);
static int dotrap(struct op *t, char **args);
static int dobreak(struct op *t, char **args);
static int doexit(struct op *t, char **args);
static int doexport(struct op *t, char **args);
static int doreadonly(struct op *t, char **args);
static int doset(struct op *t, char **args);
static int dotimes(struct op *t, char **args);
static int docontinue(struct op *t, char **args);

static int forkexec(struct op *t, int *pin, int *pout, int no_fork, char **wp);
static int execute(struct op *t, int *pin, int *pout, int no_fork);
static int iosetup(struct ioword *iop, int pipein, int pipeout);
static void brkset(struct brkcon *bc);
static int getsig(char *s);
static void setsig(int n, sighandler_t f);
static int getn(char *as);
static int brkcontin(char *cp, int val);
static void rdexp(char **wp, void (*f) (struct var *), int key);
static void badid(char *s);
static void varput(char *s, int out);
static int expand(const char *cp, struct wdblock **wbp, int f);
static char *blank(int f);
static int dollar(int quoted);
static int grave(int quoted);
static void globname(char *we, char *pp);
static char *generate(char *start1, char *end1, char *middle, char *end);
static int anyspcl(struct wdblock *wb);
static void readhere(char **name, char *s, int ec);
static int xxchar(struct ioarg *ap);

struct here {
	char *h_tag;
	char h_dosub;
	struct ioword *h_iop;
	struct here *h_next;
};

static const char *const signame[] = {
	"Signal 0",
	"Hangup",
	NULL,  /* interrupt */
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"Abort",
	"Bus error",
	"Floating Point Exception",
	"Killed",
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	NULL,  /* broken pipe */
	"Alarm clock",
	"Terminated"
};


typedef int (*builtin_func_ptr)(struct op *, char **);

struct builtincmd {
	const char *name;
	builtin_func_ptr builtinfunc;
};

static const struct builtincmd builtincmds[] = {
	{ "."       , dodot      },
	{ ":"       , dolabel    },
	{ "break"   , dobreak    },
	{ "cd"      , dochdir    },
	{ "continue", docontinue },
	{ "eval"    , doeval     },
	{ "exec"    , doexec     },
	{ "exit"    , doexit     },
	{ "export"  , doexport   },
	{ "help"    , dohelp     },
	{ "login"   , dologin    },
	{ "newgrp"  , dologin    },
	{ "read"    , doread     },
	{ "readonly", doreadonly },
	{ "set"     , doset      },
	{ "shift"   , doshift    },
	{ "times"   , dotimes    },
	{ "trap"    , dotrap     },
	{ "umask"   , doumask    },
	{ "wait"    , dowait     },
	{ NULL      , NULL       },
};

static struct op *dowholefile(int /*, int*/);


/* Globals */
static char **dolv;
static int dolc;
static uint8_t exstat;
static smallint gflg;                   /* (seems to be a parse error indicator) */
static smallint interactive;            /* Is this an interactive shell */
static smallint execflg;
static smallint isbreak;                /* "break" statement was seen */
static int multiline;                   /* '\n' changed to ';' (counter) */
static struct op *outtree;              /* result from parser */
static xint *failpt;
static xint *errpt;
static struct brkcon *brklist;
static struct wdblock *wdlist;
static struct wdblock *iolist;

#ifdef MSHDEBUG
static struct var *mshdbg_var;
#endif
static struct var *vlist;		/* dictionary */
static struct var *homedir;		/* home directory */
static struct var *prompt;		/* main prompt */
static struct var *cprompt;		/* continuation prompt */
static struct var *path;		/* search path for commands */
static struct var *shell;		/* shell to interpret command files */
static struct var *ifs;			/* field separators */

static int areanum;                     /* current allocation area */
static smallint intr;                   /* interrupt pending (bool) */
static smallint heedint = 1;            /* heed interrupt signals (bool) */
static int inparse;
static char *null = (char*)"";          /* null value for variable */
static void (*qflag)(int) = SIG_IGN;
static int startl;
static int peeksym;
static int nlseen;
static int iounit = IODEFAULT;
static YYSTYPE yylval;
static char *elinep; /* done in main(): = line + sizeof(line) - 5 */

static struct here *inhere;     /* list of hear docs while parsing */
static struct here *acthere;    /* list of active here documents */
static struct region *areabot;  /* bottom of area */
static struct region *areatop;  /* top of area */
static struct region *areanxt;  /* starting point of scan */
static void *brktop;
static void *brkaddr;

#define AFID_NOBUF	(~0)
#define AFID_ID		0


/*
 * parsing & execution environment
 */
struct env {
	char *linep;
	struct io *iobase;
	struct io *iop;
	xint *errpt;		/* void * */
	int iofd;
	struct env *oenv;
};


struct globals {
	struct env global_env;
	struct ioarg temparg; // = { .afid = AFID_NOBUF };	/* temporary for PUSHIO */
	unsigned bufid; // = AFID_ID;	/* buffer id counter */
	char ourtrap[_NSIG + 1];
	char *trap[_NSIG + 1];
	struct iobuf sharedbuf; /* in main(): set to { AFID_NOBUF } */
	struct iobuf mainbuf; /* in main(): set to { AFID_NOBUF } */
	struct ioarg ioargstack[NPUSH];
	/*
	 * flags:
	 * -e: quit on error
	 * -k: look for name=value everywhere on command line
	 * -n: no execution
	 * -t: exit after reading and executing one command
	 * -v: echo as read
	 * -x: trace
	 * -u: unset variables net diagnostic
	 */
	char flags['z' - 'a' + 1];
	char filechar_cmdbuf[BUFSIZ];
	char line[LINELIM];
	char child_cmd[LINELIM];

	struct io iostack[NPUSH];

	char grave__var_name[LINELIM];
	char grave__alt_value[LINELIM];
};

#define G (*ptr_to_globals)
#define global_env      (G.global_env     )
#define temparg         (G.temparg        )
#define bufid           (G.bufid          )
#define ourtrap         (G.ourtrap        )
#define trap            (G.trap           )
#define sharedbuf       (G.sharedbuf      )
#define mainbuf         (G.mainbuf        )
#define ioargstack      (G.ioargstack     )
/* this looks weird, but is OK ... we index FLAG with 'a'...'z' */
#define FLAG            (G.flags - 'a'    )
#define filechar_cmdbuf (G.filechar_cmdbuf)
#define line            (G.line           )
#define child_cmd       (G.child_cmd      )
#define iostack         (G.iostack        )
#define INIT_G() do { \
	SET_PTR_TO_GLOBALS(xzalloc(sizeof(G))); \
	global_env.linep = line; \
	global_env.iobase = iostack; \
	global_env.iop = iostack - 1; \
	global_env.iofd = FDBASE; \
	temparg.afid = AFID_NOBUF; \
	bufid = AFID_ID; \
} while (0)


/* in substitution */
#define	INSUB()	(global_env.iop->task == XGRAVE || global_env.iop->task == XDOLL)

#define	RUN(what, arg, gen) ((temparg.what = (arg)), run(&temparg, (gen)))

#ifdef MSHDEBUG
static void print_tree(struct op *head)
{
	if (head == NULL) {
		DBGPRINTF(("PRINT_TREE: no tree\n"));
		return;
	}

	DBGPRINTF(("NODE: %p, left %p, right %p\n", head, head->left,
			   head->right));

	if (head->left)
		print_tree(head->left);

	if (head->right)
		print_tree(head->right);
}
#endif /* MSHDEBUG */


/*
 * IO functions
 */
static void prs(const char *s)
{
	if (*s)
		write(STDERR_FILENO, s, strlen(s));
}

static void prn(unsigned u)
{
	prs(itoa(u));
}

static void echo(char **wp)
{
	int i;

	prs("+");
	for (i = 0; wp[i]; i++) {
		if (i)
			prs(" ");
		prs(wp[i]);
	}
	prs("\n");
}

static void closef(int i)
{
	if (i > 2)
		close(i);
}

static void closeall(void)
{
	int u;

	for (u = NUFILE; u < NOFILE;)
		close(u++);
}


/* fail but return to process next command */
static void fail(void) NORETURN;
static void fail(void)
{
	longjmp(failpt, 1);
	/* NOTREACHED */
}

/* abort shell (or fail in subshell) */
static void leave(void) NORETURN;
static void leave(void)
{
	DBGPRINTF(("LEAVE: leave called!\n"));

	if (execflg)
		fail();
	scraphere();
	freehere(1);
	runtrap(0);
	_exit(exstat);
	/* NOTREACHED */
}

static void warn(const char *s)
{
	if (*s) {
		prs(s);
		if (!exstat)
			exstat = 255;
	}
	prs("\n");
	if (FLAG['e'])
		leave();
}

static void err(const char *s)
{
	warn(s);
	if (FLAG['n'])
		return;
	if (!interactive)
		leave();
	if (global_env.errpt)
		longjmp(global_env.errpt, 1);
	closeall();
	global_env.iop = global_env.iobase = iostack;
}


/* -------- area.c -------- */

/*
 * All memory between (char *)areabot and (char *)(areatop+1) is
 * exclusively administered by the area management routines.
 * It is assumed that sbrk() and brk() manipulate the high end.
 */

#define sbrk(X) ({ \
	void * __q = (void *)-1; \
	if (brkaddr + (int)(X) < brktop) { \
		__q = brkaddr; \
		brkaddr += (int)(X); \
	} \
	__q; \
})

static void initarea(void)
{
	brkaddr = xmalloc(AREASIZE);
	brktop = brkaddr + AREASIZE;

	while ((long) sbrk(0) & ALIGN)
		sbrk(1);
	areabot = (struct region *) sbrk(REGSIZE);

	areabot->next = areabot;
	areabot->area = BUSY;
	areatop = areabot;
	areanxt = areabot;
}

static char *getcell(unsigned nbytes)
{
	int nregio;
	struct region *p, *q;
	int i;

	if (nbytes == 0) {
		puts("getcell(0)");
		abort();
	}
	/* silly and defeats the algorithm */
	/*
	 * round upwards and add administration area
	 */
	nregio = (nbytes + (REGSIZE - 1)) / REGSIZE + 1;
	p = areanxt;
	for (;;) {
		if (p->area > areanum) {
			/*
			 * merge free cells
			 */
			while ((q = p->next)->area > areanum && q != areanxt)
				p->next = q->next;
			/*
			 * exit loop if cell big enough
			 */
			if (q >= p + nregio)
				goto found;
		}
		p = p->next;
		if (p == areanxt)
			break;
	}
	i = nregio >= GROWBY ? nregio : GROWBY;
	p = (struct region *) sbrk(i * REGSIZE);
	if (p == (struct region *) -1)
		return NULL;
	p--;
	if (p != areatop) {
		puts("not contig");
		abort();				/* allocated areas are contiguous */
	}
	q = p + i;
	p->next = q;
	p->area = FREE;
	q->next = areabot;
	q->area = BUSY;
	areatop = q;
 found:
	/*
	 * we found a FREE area big enough, pointed to by 'p', and up to 'q'
	 */
	areanxt = p + nregio;
	if (areanxt < q) {
		/*
		 * split into requested area and rest
		 */
		if (areanxt + 1 > q) {
			puts("OOM");
			abort();			/* insufficient space left for admin */
		}
		areanxt->next = q;
		areanxt->area = FREE;
		p->next = areanxt;
	}
	p->area = areanum;
	return (char *) (p + 1);
}

static void freecell(char *cp)
{
	struct region *p;

	p = (struct region *) cp;
	if (p != NULL) {
		p--;
		if (p < areanxt)
			areanxt = p;
		p->area = FREE;
	}
}
#define	DELETE(obj) freecell((char *)obj)

static void freearea(int a)
{
	struct region *p, *top;

	top = areatop;
	for (p = areabot; p != top; p = p->next)
		if (p->area >= a)
			p->area = FREE;
}

static void setarea(char *cp, int a)
{
	struct region *p;

	p = (struct region *) cp;
	if (p != NULL)
		(p - 1)->area = a;
}

static int getarea(char *cp)
{
	return ((struct region *) cp - 1)->area;
}

static void garbage(void)
{
	struct region *p, *q, *top;

	top = areatop;
	for (p = areabot; p != top; p = p->next) {
		if (p->area > areanum) {
			while ((q = p->next)->area > areanum)
				p->next = q->next;
			areanxt = p;
		}
	}
#ifdef SHRINKBY
	if (areatop >= q + SHRINKBY && q->area > areanum) {
		brk((char *) (q + 1));
		q->next = areabot;
		q->area = BUSY;
		areatop = q;
	}
#endif
}

static void *get_space(int n)
{
	char *cp;

	cp = getcell(n);
	if (cp == NULL)
		err("out of string space");
	return cp;
}

static char *strsave(const char *s, int a)
{
	char *cp;

	cp = get_space(strlen(s) + 1);
	if (cp == NULL) {
// FIXME: I highly doubt this is good.
		return (char*)"";
	}
	setarea(cp, a);
	strcpy(cp, s);
	return cp;
}


/* -------- var.c -------- */

static int eqname(const char *n1, const char *n2)
{
	for (; *n1 != '=' && *n1 != '\0'; n1++)
		if (*n2++ != *n1)
			return 0;
	return *n2 == '\0' || *n2 == '=';
}

static const char *findeq(const char *cp)
{
	while (*cp != '\0' && *cp != '=')
		cp++;
	return cp;
}

/*
 * Find the given name in the dictionary
 * and return its value.  If the name was
 * not previously there, enter it now and
 * return a null value.
 */
static struct var *lookup(const char *n)
{
// FIXME: dirty hack
	static struct var dummy;

	struct var *vp;
	const char *cp;
	char *xp;
	int c;

	if (isdigit(*n)) {
		dummy.name = (char*)n;
		for (c = 0; isdigit(*n) && c < 1000; n++)
			c = c * 10 + *n - '0';
		dummy.status = RONLY;
		dummy.value = (c <= dolc ? dolv[c] : null);
		return &dummy;
	}

	for (vp = vlist; vp; vp = vp->next)
		if (eqname(vp->name, n))
			return vp;

	cp = findeq(n);
	vp = get_space(sizeof(*vp));
	if (vp == 0 || (vp->name = get_space((int) (cp - n) + 2)) == NULL) {
		dummy.name = dummy.value = (char*)"";
		return &dummy;
	}

	xp = vp->name;
	while ((*xp = *n++) != '\0' && *xp != '=')
		xp++;
	*xp++ = '=';
	*xp = '\0';
	setarea((char *) vp, 0);
	setarea((char *) vp->name, 0);
	vp->value = null;
	vp->next = vlist;
	vp->status = GETCELL;
	vlist = vp;
	return vp;
}

/*
 * if name is not NULL, it must be
 * a prefix of the space `val',
 * and end with `='.
 * this is all so that exporting
 * values is reasonably painless.
 */
static void nameval(struct var *vp, const char *val, const char *name)
{
	const char *cp;
	char *xp;
	int fl;

	if (vp->status & RONLY) {
		xp = vp->name;
		while (*xp && *xp != '=')
			fputc(*xp++, stderr);
		err(" is read-only");
		return;
	}
	fl = 0;
	if (name == NULL) {
		xp = get_space(strlen(vp->name) + strlen(val) + 2);
		if (xp == NULL)
			return;
		/* make string: name=value */
		setarea(xp, 0);
		name = xp;
		cp = vp->name;
		while ((*xp = *cp++) != '\0' && *xp != '=')
			xp++;
		*xp++ = '=';
		strcpy(xp, val);
		val = xp;
		fl = GETCELL;
	}
	if (vp->status & GETCELL)
		freecell(vp->name);		/* form new string `name=value' */
	vp->name = (char*)name;
	vp->value = (char*)val;
	vp->status |= fl;
}

/*
 * give variable at `vp' the value `val'.
 */
static void setval(struct var *vp, const char *val)
{
	nameval(vp, val, NULL);
}

static void export(struct var *vp)
{
	vp->status |= EXPORT;
}

static void ronly(struct var *vp)
{
	if (isalpha(vp->name[0]) || vp->name[0] == '_')	/* not an internal symbol */
		vp->status |= RONLY;
}

static int isassign(const char *s)
{
	unsigned char c;
	DBGPRINTF7(("ISASSIGN: enter, s=%s\n", s));

	c = *s;
	/* no isalpha() - we shouldn't use locale */
	/* c | 0x20 - lowercase (Latin) letters */
	if (c != '_' && (unsigned)((c|0x20) - 'a') > 25)
		/* not letter */
		return 0;

	while (1) {
		c = *++s;
		if (c == '=')
			return 1;
		if (c == '\0')
			return 0;
		if (c != '_'
		 && (unsigned)(c - '0') > 9  /* not number */
		 && (unsigned)((c|0x20) - 'a') > 25 /* not letter */
		) {
			return 0;
		}
	}
}

static int assign(const char *s, int cf)
{
	const char *cp;
	struct var *vp;

	DBGPRINTF7(("ASSIGN: enter, s=%s, cf=%d\n", s, cf));

	if (!isalpha(*s) && *s != '_')
		return 0;
	for (cp = s; *cp != '='; cp++)
		if (*cp == '\0' || (!isalnum(*cp) && *cp != '_'))
			return 0;
	vp = lookup(s);
	nameval(vp, ++cp, cf == COPYV ? NULL : s);
	if (cf != COPYV)
		vp->status &= ~GETCELL;
	return 1;
}

static int checkname(char *cp)
{
	DBGPRINTF7(("CHECKNAME: enter, cp=%s\n", cp));

	if (!isalpha(*cp++) && *(cp - 1) != '_')
		return 0;
	while (*cp)
		if (!isalnum(*cp++) && *(cp - 1) != '_')
			return 0;
	return 1;
}

static void putvlist(int f, int out)
{
	struct var *vp;

	for (vp = vlist; vp; vp = vp->next) {
		if (vp->status & f && (isalpha(*vp->name) || *vp->name == '_')) {
			if (vp->status & EXPORT)
				write(out, "export ", 7);
			if (vp->status & RONLY)
				write(out, "readonly ", 9);
			write(out, vp->name, (int) (findeq(vp->name) - vp->name));
			write(out, "\n", 1);
		}
	}
}


/*
 * trap handling
 */
static void sig(int i)
{
	trapset = i;
	signal(i, sig);
}

static void runtrap(int i)
{
	char *trapstr;

	trapstr = trap[i];
	if (trapstr == NULL)
		return;

	if (i == 0)
		trap[i] = NULL;

	RUN(aword, trapstr, nlchar);
}


static void setdash(void)
{
	char *cp;
	int c;
	char m['z' - 'a' + 1];

	cp = m;
	for (c = 'a'; c <= 'z'; c++)
		if (FLAG[c])
			*cp++ = c;
	*cp = '\0';
	setval(lookup("-"), m);
}

static int newfile(char *s)
{
	int f;

	DBGPRINTF7(("NEWFILE: opening %s\n", s));

	f = 0;
	if (NOT_LONE_DASH(s)) {
		DBGPRINTF(("NEWFILE: s is %s\n", s));
		f = open(s, O_RDONLY);
		if (f < 0) {
			prs(s);
			err(": can't open");
			return 1;
		}
	}

	next(remap(f));
	return 0;
}


#ifdef UNUSED
struct op *scantree(struct op *head)
{
	struct op *dotnode;

	if (head == NULL)
		return NULL;

	if (head->left != NULL) {
		dotnode = scantree(head->left);
		if (dotnode)
			return dotnode;
	}

	if (head->right != NULL) {
		dotnode = scantree(head->right);
		if (dotnode)
			return dotnode;
	}

	if (head->op_words == NULL)
		return NULL;

	DBGPRINTF5(("SCANTREE: checking node %p\n", head));

	if ((head->op_type != TDOT) && LONE_CHAR(head->op_words[0], '.')) {
		DBGPRINTF5(("SCANTREE: dot found in node %p\n", head));
		return head;
	}

	return NULL;
}
#endif


static void onecommand(void)
{
	int i;
	jmp_buf m1;

	DBGPRINTF(("ONECOMMAND: enter, outtree=%p\n", outtree));

	while (global_env.oenv)
		quitenv();

	areanum = 1;
	freehere(areanum);
	freearea(areanum);
	garbage();
	wdlist = NULL;
	iolist = NULL;
	global_env.errpt = NULL;
	global_env.linep = line;
	yynerrs = 0;
	multiline = 0;
	inparse = 1;
	intr = 0;
	execflg = 0;

	failpt = m1;
	setjmp(failpt);		/* Bruce Evans' fix */
	failpt = m1;
	if (setjmp(failpt) || yyparse() || intr) {
		DBGPRINTF(("ONECOMMAND: this is not good.\n"));

		while (global_env.oenv)
			quitenv();
		scraphere();
		if (!interactive && intr)
			leave();
		inparse = 0;
		intr = 0;
		return;
	}

	inparse = 0;
	brklist = 0;
	intr = 0;
	execflg = 0;

	if (!FLAG['n']) {
		DBGPRINTF(("ONECOMMAND: calling execute, t=outtree=%p\n",
				   outtree));
		execute(outtree, NOPIPE, NOPIPE, /* no_fork: */ 0);
	}

	if (!interactive && intr) {
		execflg = 0;
		leave();
	}

	i = trapset;
	if (i != 0) {
		trapset = 0;
		runtrap(i);
	}
}

static int newenv(int f)
{
	struct env *ep;

	DBGPRINTF(("NEWENV: f=%d (indicates quitenv and return)\n", f));

	if (f) {
		quitenv();
		return 1;
	}

	ep = get_space(sizeof(*ep));
	if (ep == NULL) {
		while (global_env.oenv)
			quitenv();
		fail();
	}
	*ep = global_env;
	global_env.oenv = ep;
	global_env.errpt = errpt;

	return 0;
}

static void quitenv(void)
{
	struct env *ep;
	int fd;

	DBGPRINTF(("QUITENV: global_env.oenv=%p\n", global_env.oenv));

	ep = global_env.oenv;
	if (ep != NULL) {
		fd = global_env.iofd;
		global_env = *ep;
		/* should close `'d files */
		DELETE(ep);
		while (--fd >= global_env.iofd)
			close(fd);
	}
}

/*
 * Is character c in s?
 */
static int any(int c, const char *s)
{
	while (*s)
		if (*s++ == c)
			return 1;
	return 0;
}

/*
 * Is any character from s1 in s2?
 */
static int anys(const char *s1, const char *s2)
{
	while (*s1)
		if (any(*s1++, s2))
			return 1;
	return 0;
}

static char *putn(int n)
{
	return itoa(n);
}

static void next(int f)
{
	PUSHIO(afile, f, filechar);
}

static void onintr(int s UNUSED_PARAM) /* ANSI C requires a parameter */
{
	signal(SIGINT, onintr);
	intr = 1;
	if (interactive) {
		if (inparse) {
			prs("\n");
			fail();
		}
	} else if (heedint) {
		execflg = 0;
		leave();
	}
}


/* -------- gmatch.c -------- */
/*
 * int gmatch(string, pattern)
 * char *string, *pattern;
 *
 * Match a pattern as in sh(1).
 */

#define	CMASK	0377
#define	QUOTE	0200
#define	QMASK	(CMASK & ~QUOTE)
#define	NOT	'!'					/* might use ^ */

static const char *cclass(const char *p, int sub)
{
	int c, d, not, found;

	not = (*p == NOT);
	if (not != 0)
		p++;
	found = not;
	do {
		if (*p == '\0')
			return NULL;
		c = *p & CMASK;
		if (p[1] == '-' && p[2] != ']') {
			d = p[2] & CMASK;
			p++;
		} else
			d = c;
		if (c == sub || (c <= sub && sub <= d))
			found = !not;
	} while (*++p != ']');
	return found ? p + 1 : NULL;
}

static int gmatch(const char *s, const char *p)
{
	int sc, pc;

	if (s == NULL || p == NULL)
		return 0;

	while ((pc = *p++ & CMASK) != '\0') {
		sc = *s++ & QMASK;
		switch (pc) {
		case '[':
			p = cclass(p, sc);
			if (p == NULL)
				return 0;
			break;

		case '?':
			if (sc == 0)
				return 0;
			break;

		case '*':
			s--;
			do {
				if (*p == '\0' || gmatch(s, p))
					return 1;
			} while (*s++ != '\0');
			return 0;

		default:
			if (sc != (pc & ~QUOTE))
				return 0;
		}
	}
	return *s == '\0';
}


/* -------- csyn.c -------- */
/*
 * shell: syntax (C version)
 */

static void yyerror(const char *s) NORETURN;
static void yyerror(const char *s)
{
	yynerrs = 1;
	if (interactive && global_env.iop <= iostack) {
		multiline = 0;
		while (eofc() == 0 && yylex(0) != '\n')
			continue;
	}
	err(s);
	fail();
}

static void zzerr(void) NORETURN;
static void zzerr(void)
{
	yyerror("syntax error");
}

int yyparse(void)
{
	DBGPRINTF7(("YYPARSE: enter...\n"));

	startl = 1;
	peeksym = 0;
	yynerrs = 0;
	outtree = c_list();
	musthave('\n', 0);
	return yynerrs; /* 0/1 */
}

static struct op *pipeline(int cf)
{
	struct op *t, *p;
	int c;

	DBGPRINTF7(("PIPELINE: enter, cf=%d\n", cf));

	t = command(cf);

	DBGPRINTF9(("PIPELINE: t=%p\n", t));

	if (t != NULL) {
		while ((c = yylex(0)) == '|') {
			p = command(CONTIN);
			if (p == NULL) {
				DBGPRINTF8(("PIPELINE: error!\n"));
				zzerr();
			}

			if (t->op_type != TPAREN && t->op_type != TCOM) {
				/* shell statement */
				t = block(TPAREN, t, NOBLOCK, NOWORDS);
			}

			t = block(TPIPE, t, p, NOWORDS);
		}
		peeksym = c;
	}

	DBGPRINTF7(("PIPELINE: returning t=%p\n", t));
	return t;
}

static struct op *andor(void)
{
	struct op *t, *p;
	int c;

	DBGPRINTF7(("ANDOR: enter...\n"));

	t = pipeline(0);

	DBGPRINTF9(("ANDOR: t=%p\n", t));

	if (t != NULL) {
		while ((c = yylex(0)) == LOGAND || c == LOGOR) {
			p = pipeline(CONTIN);
			if (p == NULL) {
				DBGPRINTF8(("ANDOR: error!\n"));
				zzerr();
			}

			t = block(c == LOGAND ? TAND : TOR, t, p, NOWORDS);
		}

		peeksym = c;
	}

	DBGPRINTF7(("ANDOR: returning t=%p\n", t));
	return t;
}

static struct op *c_list(void)
{
	struct op *t, *p;
	int c;

	DBGPRINTF7(("C_LIST: enter...\n"));

	t = andor();

	if (t != NULL) {
		peeksym = yylex(0);
		if (peeksym == '&')
			t = block(TASYNC, t, NOBLOCK, NOWORDS);

		while ((c = yylex(0)) == ';' || c == '&'
		 || (multiline && c == '\n')
		) {
			p = andor();
			if (p== NULL)
				return t;

			peeksym = yylex(0);
			if (peeksym == '&')
				p = block(TASYNC, p, NOBLOCK, NOWORDS);

			t = list(t, p);
		}						/* WHILE */

		peeksym = c;
	}
	/* IF */
	DBGPRINTF7(("C_LIST: returning t=%p\n", t));
	return t;
}

static int synio(int cf)
{
	struct ioword *iop;
	int i;
	int c;

	DBGPRINTF7(("SYNIO: enter, cf=%d\n", cf));

	c = yylex(cf);
	if (c != '<' && c != '>') {
		peeksym = c;
		return 0;
	}

	i = yylval.i;
	musthave(WORD, 0);
	iop = io(iounit, i, yylval.cp);
	iounit = IODEFAULT;

	if (i & IOHERE)
		markhere(yylval.cp, iop);

	DBGPRINTF7(("SYNIO: returning 1\n"));
	return 1;
}

static void musthave(int c, int cf)
{
	peeksym = yylex(cf);
	if (peeksym != c) {
		DBGPRINTF7(("MUSTHAVE: error!\n"));
		zzerr();
	}

	peeksym = 0;
}

static struct op *simple(void)
{
	struct op *t;

	t = NULL;
	for (;;) {
		switch (peeksym = yylex(0)) {
		case '<':
		case '>':
			(void) synio(0);
			break;

		case WORD:
			if (t == NULL) {
				t = newtp();
				t->op_type = TCOM;
			}
			peeksym = 0;
			word(yylval.cp);
			break;

		default:
			return t;
		}
	}
}

static struct op *nested(int type, int mark)
{
	struct op *t;

	DBGPRINTF3(("NESTED: enter, type=%d, mark=%d\n", type, mark));

	multiline++;
	t = c_list();
	musthave(mark, 0);
	multiline--;
	return block(type, t, NOBLOCK, NOWORDS);
}

static struct op *command(int cf)
{
	struct op *t;
	struct wdblock *iosave;
	int c;

	DBGPRINTF(("COMMAND: enter, cf=%d\n", cf));

	iosave = iolist;
	iolist = NULL;

	if (multiline)
		cf |= CONTIN;

	while (synio(cf))
		cf = 0;

	c = yylex(cf);

	switch (c) {
	default:
		peeksym = c;
		t = simple();
		if (t == NULL) {
			if (iolist == NULL)
				return NULL;
			t = newtp();
			t->op_type = TCOM;
		}
		break;

	case '(':
		t = nested(TPAREN, ')');
		break;

	case '{':
		t = nested(TBRACE, '}');
		break;

	case FOR:
		t = newtp();
		t->op_type = TFOR;
		musthave(WORD, 0);
		startl = 1;
		t->str = yylval.cp;
		multiline++;
		t->op_words = wordlist();
		c = yylex(0);
		if (c != '\n' && c != ';')
			peeksym = c;
		t->left = dogroup(0);
		multiline--;
		break;

	case WHILE:
	case UNTIL:
		multiline++;
		t = newtp();
		t->op_type = (c == WHILE ? TWHILE : TUNTIL);
		t->left = c_list();
		t->right = dogroup(1);
		/* t->op_words = NULL; - newtp() did this */
		multiline--;
		break;

	case CASE:
		t = newtp();
		t->op_type = TCASE;
		musthave(WORD, 0);
		t->str = yylval.cp;
		startl++;
		multiline++;
		musthave(IN, CONTIN);
		startl++;

		t->left = caselist();

		musthave(ESAC, 0);
		multiline--;
		break;

	case IF:
		multiline++;
		t = newtp();
		t->op_type = TIF;
		t->left = c_list();
		t->right = thenpart();
		musthave(FI, 0);
		multiline--;
		break;

	case DOT:
		t = newtp();
		t->op_type = TDOT;

		musthave(WORD, 0);              /* gets name of file */
		DBGPRINTF7(("COMMAND: DOT clause, yylval.cp is %s\n", yylval.cp));

		word(yylval.cp);                /* add word to wdlist */
		word(NOWORD);                   /* terminate  wdlist */
		t->op_words = copyw();          /* dup wdlist */
		break;

	}

	while (synio(0))
		continue;

	t = namelist(t);
	iolist = iosave;

	DBGPRINTF(("COMMAND: returning %p\n", t));

	return t;
}

static struct op *dowholefile(int type /*, int mark*/)
{
	struct op *t;

	DBGPRINTF(("DOWHOLEFILE: enter, type=%d\n", type /*, mark*/));

	multiline++;
	t = c_list();
	multiline--;
	t = block(type, t, NOBLOCK, NOWORDS);
	DBGPRINTF(("DOWHOLEFILE: return t=%p\n", t));
	return t;
}

static struct op *dogroup(int onlydone)
{
	int c;
	struct op *mylist;

	c = yylex(CONTIN);
	if (c == DONE && onlydone)
		return NULL;
	if (c != DO)
		zzerr();
	mylist = c_list();
	musthave(DONE, 0);
	return mylist;
}

static struct op *thenpart(void)
{
	int c;
	struct op *t;

	c = yylex(0);
	if (c != THEN) {
		peeksym = c;
		return NULL;
	}
	t = newtp();
	/*t->op_type = 0; - newtp() did this */
	t->left = c_list();
	if (t->left == NULL)
		zzerr();
	t->right = elsepart();
	return t;
}

static struct op *elsepart(void)
{
	int c;
	struct op *t;

	switch (c = yylex(0)) {
	case ELSE:
		t = c_list();
		if (t == NULL)
			zzerr();
		return t;

	case ELIF:
		t = newtp();
		t->op_type = TELIF;
		t->left = c_list();
		t->right = thenpart();
		return t;

	default:
		peeksym = c;
		return NULL;
	}
}

static struct op *caselist(void)
{
	struct op *t;

	t = NULL;
	while ((peeksym = yylex(CONTIN)) != ESAC) {
		DBGPRINTF(("CASELIST, doing yylex, peeksym=%d\n", peeksym));
		t = list(t, casepart());
	}

	DBGPRINTF(("CASELIST, returning t=%p\n", t));
	return t;
}

static struct op *casepart(void)
{
	struct op *t;

	DBGPRINTF7(("CASEPART: enter...\n"));

	t = newtp();
	t->op_type = TPAT;
	t->op_words = pattern();
	musthave(')', 0);
	t->left = c_list();
	peeksym = yylex(CONTIN);
	if (peeksym != ESAC)
		musthave(BREAK, CONTIN);

	DBGPRINTF7(("CASEPART: made newtp(TPAT, t=%p)\n", t));

	return t;
}

static char **pattern(void)
{
	int c, cf;

	cf = CONTIN;
	do {
		musthave(WORD, cf);
		word(yylval.cp);
		cf = 0;
		c = yylex(0);
	} while (c == '|');
	peeksym = c;
	word(NOWORD);

	return copyw();
}

static char **wordlist(void)
{
	int c;

	c = yylex(0);
	if (c != IN) {
		peeksym = c;
		return NULL;
	}
	startl = 0;
	while ((c = yylex(0)) == WORD)
		word(yylval.cp);
	word(NOWORD);
	peeksym = c;
	return copyw();
}

/*
 * supporting functions
 */
static struct op *list(struct op *t1, struct op *t2)
{
	DBGPRINTF7(("LIST: enter, t1=%p, t2=%p\n", t1, t2));

	if (t1 == NULL)
		return t2;
	if (t2 == NULL)
		return t1;

	return block(TLIST, t1, t2, NOWORDS);
}

static struct op *block(int type, struct op *t1, struct op *t2, char **wp)
{
	struct op *t;

	DBGPRINTF7(("BLOCK: enter, type=%d (%s)\n", type, T_CMD_NAMES[type]));

	t = newtp();
	t->op_type = type;
	t->left = t1;
	t->right = t2;
	t->op_words = wp;

	DBGPRINTF7(("BLOCK: inserted %p between %p and %p\n", t, t1, t2));

	return t;
}

/* See if given string is a shell multiline (FOR, IF, etc) */
static int rlookup(char *n)
{
	struct res {
		char r_name[6];
		int16_t r_val;
	};
	static const struct res restab[] = {
		{ "for"  , FOR    },
		{ "case" , CASE   },
		{ "esac" , ESAC   },
		{ "while", WHILE  },
		{ "do"   , DO     },
		{ "done" , DONE   },
		{ "if"   , IF     },
		{ "in"   , IN     },
		{ "then" , THEN   },
		{ "else" , ELSE   },
		{ "elif" , ELIF   },
		{ "until", UNTIL  },
		{ "fi"   , FI     },
		{ ";;"   , BREAK  },
		{ "||"   , LOGOR  },
		{ "&&"   , LOGAND },
		{ "{"    , '{'    },
		{ "}"    , '}'    },
		{ "."    , DOT    },
		{ },
	};

	const struct res *rp;

	DBGPRINTF7(("RLOOKUP: enter, n is %s\n", n));

	for (rp = restab; rp->r_name[0]; rp++)
		if (strcmp(rp->r_name, n) == 0) {
			DBGPRINTF7(("RLOOKUP: match, returning %d\n", rp->r_val));
			return rp->r_val;	/* Return numeric code for shell multiline */
		}

	DBGPRINTF7(("RLOOKUP: NO match, returning 0\n"));
	return 0;					/* Not a shell multiline */
}

static struct op *newtp(void)
{
	struct op *t;

	t = (struct op *) tree(sizeof(*t));
	memset(t, 0, sizeof(*t));

	DBGPRINTF3(("NEWTP: allocated %p\n", t));

	return t;
}

static struct op *namelist(struct op *t)
{
	DBGPRINTF7(("NAMELIST: enter, t=%p, type %s, iolist=%p\n", t,
				T_CMD_NAMES[t->op_type], iolist));

	if (iolist) {
		iolist = addword((char *) NULL, iolist);
		t->ioact = copyio();
	} else
		t->ioact = NULL;

	if (t->op_type != TCOM) {
		if (t->op_type != TPAREN && t->ioact != NULL) {
			t = block(TPAREN, t, NOBLOCK, NOWORDS);
			t->ioact = t->left->ioact;
			t->left->ioact = NULL;
		}
		return t;
	}

	word(NOWORD);
	t->op_words = copyw();

	return t;
}

static char **copyw(void)
{
	char **wd;

	wd = getwords(wdlist);
	wdlist = NULL;
	return wd;
}

static void word(char *cp)
{
	wdlist = addword(cp, wdlist);
}

static struct ioword **copyio(void)
{
	struct ioword **iop;

	iop = (struct ioword **) getwords(iolist);
	iolist = NULL;
	return iop;
}

static struct ioword *io(int u, int f, char *cp)
{
	struct ioword *iop;

	iop = (struct ioword *) tree(sizeof(*iop));
	iop->io_fd = u;
	iop->io_flag = f;
	iop->io_name = cp;
	iolist = addword((char *) iop, iolist);
	return iop;
}

static int yylex(int cf)
{
	int c, c1;
	int atstart;

	c = peeksym;
	if (c > 0) {
		peeksym = 0;
		if (c == '\n')
			startl = 1;
		return c;
	}

	nlseen = 0;
	atstart = startl;
	startl = 0;
	yylval.i = 0;
	global_env.linep = line;

/* MALAMO */
	line[LINELIM - 1] = '\0';

 loop:
	while ((c = my_getc(0)) == ' ' || c == '\t')	/* Skip whitespace */
		continue;

	switch (c) {
	default:
		if (any(c, "0123456789")) {
			c1 = my_getc(0);
			unget(c1);
			if (c1 == '<' || c1 == '>') {
				iounit = c - '0';
				goto loop;
			}
			*global_env.linep++ = c;
			c = c1;
		}
		break;

	case '#':	/* Comment, skip to next newline or End-of-string */
		while ((c = my_getc(0)) != '\0' && c != '\n')
			continue;
		unget(c);
		goto loop;

	case 0:
		DBGPRINTF5(("YYLEX: return 0, c=%d\n", c));
		return c;

	case '$':
		DBGPRINTF9(("YYLEX: found $\n"));
		*global_env.linep++ = c;
		c = my_getc(0);
		if (c == '{') {
			c = collect(c, '}');
			if (c != '\0')
				return c;
			goto pack;
		}
		break;

	case '`':
	case '\'':
	case '"':
		c = collect(c, c);
		if (c != '\0')
			return c;
		goto pack;

	case '|':
	case '&':
	case ';':
		startl = 1;
		/* If more chars process them, else return NULL char */
		c1 = dual(c);
		if (c1 != '\0')
			return c1;
		return c;

	case '^':
		startl = 1;
		return '|';
	case '>':
	case '<':
		diag(c);
		return c;

	case '\n':
		nlseen++;
		gethere();
		startl = 1;
		if (multiline || cf & CONTIN) {
			if (interactive && global_env.iop <= iostack) {
#if ENABLE_FEATURE_EDITING
				current_prompt = cprompt->value;
#else
				prs(cprompt->value);
#endif
			}
			if (cf & CONTIN)
				goto loop;
		}
		return c;

	case '(':
	case ')':
		startl = 1;
		return c;
	}

	unget(c);

 pack:
	while ((c = my_getc(0)) != '\0' && !any(c, "`$ '\"\t;&<>()|^\n")) {
		if (global_env.linep >= elinep)
			err("word too long");
		else
			*global_env.linep++ = c;
	};

	unget(c);

	if (any(c, "\"'`$"))
		goto loop;

	*global_env.linep++ = '\0';

	if (atstart) {
		c = rlookup(line);
		if (c != 0) {
			startl = 1;
			return c;
		}
	}

	yylval.cp = strsave(line, areanum);
	return WORD;
}


static int collect(int c, int c1)
{
	char s[2];

	DBGPRINTF8(("COLLECT: enter, c=%d, c1=%d\n", c, c1));

	*global_env.linep++ = c;
	while ((c = my_getc(c1)) != c1) {
		if (c == 0) {
			unget(c);
			s[0] = c1;
			s[1] = 0;
			prs("no closing ");
			yyerror(s);
			return YYERRCODE;
		}
		if (interactive && c == '\n' && global_env.iop <= iostack) {
#if ENABLE_FEATURE_EDITING
			current_prompt = cprompt->value;
#else
			prs(cprompt->value);
#endif
		}
		*global_env.linep++ = c;
	}

	*global_env.linep++ = c;

	DBGPRINTF8(("COLLECT: return 0, line is %s\n", line));

	return 0;
}

/* "multiline commands" helper func */
/* see if next 2 chars form a shell multiline */
static int dual(int c)
{
	char s[3];
	char *cp = s;

	DBGPRINTF8(("DUAL: enter, c=%d\n", c));

	*cp++ = c;              /* c is the given "peek" char */
	*cp++ = my_getc(0);     /* get next char of input */
	*cp = '\0';             /* add EOS marker */

	c = rlookup(s);	        /* see if 2 chars form a shell multiline */
	if (c == 0)
		unget(*--cp);   /* String is not a shell multiline, put peek char back */

	return c;               /* String is multiline, return numeric multiline (restab) code */
}

static void diag(int ec)
{
	int c;

	DBGPRINTF8(("DIAG: enter, ec=%d\n", ec));

	c = my_getc(0);
	if (c == '>' || c == '<') {
		if (c != ec)
			zzerr();
		yylval.i = (ec == '>' ? IOWRITE | IOCAT : IOHERE);
		c = my_getc(0);
	} else
		yylval.i = (ec == '>' ? IOWRITE : IOREAD);
	if (c != '&' || yylval.i == IOHERE)
		unget(c);
	else
		yylval.i |= IODUP;
}

static char *tree(unsigned size)
{
	char *t;

	t = getcell(size);
	if (t == NULL) {
		DBGPRINTF2(("TREE: getcell(%d) failed!\n", size));
		prs("command line too complicated\n");
		fail();
		/* NOTREACHED */
	}
	return t;
}


/* VARARGS1 */
/* ARGSUSED */

/* -------- exec.c -------- */

static struct op **find1case(struct op *t, const char *w)
{
	struct op *t1;
	struct op **tp;
	char **wp;
	char *cp;

	if (t == NULL) {
		DBGPRINTF3(("FIND1CASE: enter, t==NULL, returning.\n"));
		return NULL;
	}

	DBGPRINTF3(("FIND1CASE: enter, t->op_type=%d (%s)\n", t->op_type,
				T_CMD_NAMES[t->op_type]));

	if (t->op_type == TLIST) {
		tp = find1case(t->left, w);
		if (tp != NULL) {
			DBGPRINTF3(("FIND1CASE: found one to the left, returning tp=%p\n", tp));
			return tp;
		}
		t1 = t->right;			/* TPAT */
	} else
		t1 = t;

	for (wp = t1->op_words; *wp;) {
		cp = evalstr(*wp++, DOSUB);
		if (cp && gmatch(w, cp)) {
			DBGPRINTF3(("FIND1CASE: returning &t1->left= %p.\n",
						&t1->left));
			return &t1->left;
		}
	}

	DBGPRINTF(("FIND1CASE: returning NULL\n"));
	return NULL;
}

static struct op *findcase(struct op *t, const char *w)
{
	struct op **tp;

	tp = find1case(t, w);
	return tp != NULL ? *tp : NULL;
}

/*
 * execute tree
 */

static int execute(struct op *t, int *pin, int *pout, int no_fork)
{
	struct op *t1;
	volatile int i, rv, a;
	const char *cp;
	char **wp, **wp2;
	struct var *vp;
	struct op *outtree_save;
	struct brkcon bc;

#if __GNUC__
	/* Avoid longjmp clobbering */
	(void) &wp;
#endif

	if (t == NULL) {
		DBGPRINTF4(("EXECUTE: enter, t==null, returning.\n"));
		return 0;
	}

	DBGPRINTF(("EXECUTE: t=%p, t->op_type=%d (%s), t->op_words is %s\n", t,
			   t->op_type, T_CMD_NAMES[t->op_type],
			   ((t->op_words == NULL) ? "NULL" : t->op_words[0])));

	rv = 0;
	a = areanum++;
	wp2 = t->op_words;
	wp = (wp2 != NULL)
		? eval(wp2, t->op_type == TCOM ? DOALL : DOALL & ~DOKEY)
		: NULL;

	switch (t->op_type) {
	case TDOT:
		DBGPRINTF3(("EXECUTE: TDOT\n"));

		outtree_save = outtree;

		newfile(evalstr(t->op_words[0], DOALL));

		t->left = dowholefile(TLIST /*, 0*/);
		t->right = NULL;

		outtree = outtree_save;

		if (t->left)
			rv = execute(t->left, pin, pout, /* no_fork: */ 0);
		if (t->right)
			rv = execute(t->right, pin, pout, /* no_fork: */ 0);
		break;

	case TPAREN:
		rv = execute(t->left, pin, pout, /* no_fork: */ 0);
		break;

	case TCOM:
		rv = forkexec(t, pin, pout, no_fork, wp);
		break;

	case TPIPE:
		{
			int pv[2];

			rv = openpipe(pv);
			if (rv < 0)
				break;
			pv[0] = remap(pv[0]);
			pv[1] = remap(pv[1]);
			(void) execute(t->left, pin, pv, /* no_fork: */ 0);
			rv = execute(t->right, pv, pout, /* no_fork: */ 0);
		}
		break;

	case TLIST:
		(void) execute(t->left, pin, pout, /* no_fork: */ 0);
		rv = execute(t->right, pin, pout, /* no_fork: */ 0);
		break;

	case TASYNC:
		{
			smallint hinteractive = interactive;

			DBGPRINTF7(("EXECUTE: TASYNC clause, calling vfork()...\n"));

			i = vfork();
			if (i == 0) { /* child */
				signal(SIGINT, SIG_IGN);
				signal(SIGQUIT, SIG_IGN);
				if (interactive)
					signal(SIGTERM, SIG_DFL);
				interactive = 0;
				if (pin == NULL) {
					close(0);
					xopen(bb_dev_null, O_RDONLY);
				}
				_exit(execute(t->left, pin, pout, /* no_fork: */ 1));
			}
			interactive = hinteractive;
			if (i != -1) {
				setval(lookup("!"), putn(i));
				closepipe(pin);
				if (interactive) {
					prs(putn(i));
					prs("\n");
				}
			} else
				rv = -1;
			setstatus(rv);
		}
		break;

	case TOR:
	case TAND:
		rv = execute(t->left, pin, pout, /* no_fork: */ 0);
		t1 = t->right;
		if (t1 != NULL && (rv == 0) == (t->op_type == TAND))
			rv = execute(t1, pin, pout, /* no_fork: */ 0);
		break;

	case TFOR:
		if (wp == NULL) {
			wp = dolv + 1;
			i = dolc;
			if (i < 0)
				i = 0;
		} else {
			i = -1;
			while (*wp++ != NULL)
				continue;
		}
		vp = lookup(t->str);
		while (setjmp(bc.brkpt))
			if (isbreak)
				goto broken;
		/* Restore areanum value. It may be incremented by execute()
		 * below, and then "continue" may jump back to setjmp above */
		areanum = a + 1;
		freearea(areanum + 1);
		brkset(&bc);
		for (t1 = t->left; i-- && *wp != NULL;) {
			setval(vp, *wp++);
			rv = execute(t1, pin, pout, /* no_fork: */ 0);
		}
		brklist = brklist->nextlev;
		break;

	case TWHILE:
	case TUNTIL:
		while (setjmp(bc.brkpt))
			if (isbreak)
				goto broken;
		/* Restore areanum value. It may be incremented by execute()
		 * below, and then "continue" may jump back to setjmp above */
		areanum = a + 1;
		freearea(areanum + 1);
		brkset(&bc);
		t1 = t->left;
		while ((execute(t1, pin, pout, /* no_fork: */ 0) == 0) == (t->op_type == TWHILE))
			rv = execute(t->right, pin, pout, /* no_fork: */ 0);
		brklist = brklist->nextlev;
		break;

	case TIF:
	case TELIF:
		if (t->right != NULL) {
			rv = !execute(t->left, pin, pout, /* no_fork: */ 0) ?
				execute(t->right->left, pin, pout, /* no_fork: */ 0) :
				execute(t->right->right, pin, pout, /* no_fork: */ 0);
		}
		break;

	case TCASE:
		cp = evalstr(t->str, DOSUB | DOTRIM);
		if (cp == NULL)
			cp = "";

		DBGPRINTF7(("EXECUTE: TCASE, t->str is %s, cp is %s\n",
					((t->str == NULL) ? "NULL" : t->str),
					((cp == NULL) ? "NULL" : cp)));

		t1 = findcase(t->left, cp);
		if (t1 != NULL) {
			DBGPRINTF7(("EXECUTE: TCASE, calling execute(t=%p, t1=%p)...\n", t, t1));
			rv = execute(t1, pin, pout, /* no_fork: */ 0);
			DBGPRINTF7(("EXECUTE: TCASE, back from execute(t=%p, t1=%p)...\n", t, t1));
		}
		break;

	case TBRACE:
/*
		iopp = t->ioact;
		if (i)
			while (*iopp)
				if (iosetup(*iopp++, pin!=NULL, pout!=NULL)) {
					rv = -1;
					break;
				}
*/
		if (rv >= 0) {
			t1 = t->left;
			if (t1) {
				rv = execute(t1, pin, pout, /* no_fork: */ 0);
			}
		}
		break;

	};

 broken:
// Restoring op_words is most likely not needed now: see comment in forkexec()
// (also take a look at exec builtin (doexec) - it touches t->op_words)
	t->op_words = wp2;
	isbreak = 0;
	freehere(areanum);
	freearea(areanum);
	areanum = a;
	if (interactive && intr) {
		closeall();
		fail();
	}

	i = trapset;
	if (i != 0) {
		trapset = 0;
		runtrap(i);
	}

	DBGPRINTF(("EXECUTE: returning from t=%p, rv=%d\n", t, rv));
	return rv;
}

static builtin_func_ptr inbuilt(const char *s)
{
	const struct builtincmd *bp;

	for (bp = builtincmds; bp->name; bp++)
		if (strcmp(bp->name, s) == 0)
			return bp->builtinfunc;
	return NULL;
}

static int forkexec(struct op *t, int *pin, int *pout, int no_fork, char **wp)
{
	pid_t newpid;
	int i;
	builtin_func_ptr bltin = NULL;
	const char *bltin_name = NULL;
	const char *cp;
	struct ioword **iopp;
	int resetsig;
	char **owp;
	int forked;

	int *hpin = pin;
	int *hpout = pout;
	char *hwp;
	smallint hinteractive;
	smallint hintr;
	smallint hexecflg;
	struct brkcon *hbrklist;

#if __GNUC__
	/* Avoid longjmp clobbering */
	(void) &pin;
	(void) &pout;
	(void) &wp;
	(void) &bltin;
	(void) &cp;
	(void) &resetsig;
	(void) &owp;
#endif

	DBGPRINTF(("FORKEXEC: t=%p, pin %p, pout %p, no_fork %d\n", t, pin,
			pout, no_fork));
	DBGPRINTF7(("FORKEXEC: t->op_words is %s\n",
			((t->op_words == NULL) ? "NULL" : t->op_words[0])));
	owp = wp;
	resetsig = 0;
	if (t->op_type == TCOM) {
		while (*wp++ != NULL)
			continue;
		cp = *wp;

		/* strip all initial assignments */
		/* FIXME: not correct wrt PATH=yyy command etc */
		if (FLAG['x']) {
			DBGPRINTF9(("FORKEXEC: echo'ing, cp=%p, wp=%p, owp=%p\n",
						cp, wp, owp));
			echo(cp ? wp : owp);
		}

		if (cp == NULL) {
			if (t->ioact == NULL) {
				while ((cp = *owp++) != NULL && assign(cp, COPYV))
					continue;
				DBGPRINTF(("FORKEXEC: returning setstatus(0)\n"));
				return setstatus(0);
			}
		} else { /* cp != NULL */
			bltin_name = cp;
			bltin = inbuilt(cp);
		}
	}

	forked = 0;
	// We were pointing t->op_words to temporary (expanded) arg list:
	// t->op_words = wp;
	// and restored it later (in execute()), but "break"
	// longjmps away (at "Run builtin" below), leaving t->op_words clobbered!
	// See http://bugs.busybox.net/view.php?id=846.
	// Now we do not touch t->op_words, but separately pass wp as param list
	// to builtins
	DBGPRINTF(("FORKEXEC: bltin %p, no_fork %d, owp %p\n", bltin,
			no_fork, owp));
	/* Don't fork if it is a lone builtin (not in pipe)
	 * OR we are told to _not_ fork */
	if ((!bltin || pin || pout)   /* not lone bltin AND */
	 && !no_fork                  /* not told to avoid fork */
	) {
		/* Save values in case child alters them after vfork */
		hpin = pin;
		hpout = pout;
		hwp = *wp;
		hinteractive = interactive;
		hintr = intr;
		hbrklist = brklist;
		hexecflg = execflg;

		DBGPRINTF3(("FORKEXEC: calling vfork()...\n"));
		newpid = vfork();
		if (newpid == -1) {
			DBGPRINTF(("FORKEXEC: ERROR, can't vfork()!\n"));
			return -1;
		}

		if (newpid > 0) {  /* Parent */
			/* Restore values */
			pin = hpin;
			pout = hpout;
			*wp = hwp;
			interactive = hinteractive;
			intr = hintr;
			brklist = hbrklist;
			execflg = hexecflg;

			closepipe(pin);
			return (pout == NULL ? setstatus(waitfor(newpid, 0)) : 0);
		}

		/* Child */
		DBGPRINTF(("FORKEXEC: child process, bltin=%p (%s)\n", bltin, bltin_name));
		if (interactive) {
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			resetsig = 1;
		}
		interactive = 0;
		intr = 0;
		forked = 1;
		brklist = 0;
		execflg = 0;
	}

	if (owp)
		while ((cp = *owp++) != NULL && assign(cp, COPYV))
			if (!bltin)
				export(lookup(cp));

	if (pin) { /* NB: close _first_, then move fds! */
		close(pin[1]);
		xmove_fd(pin[0], 0);
	}
	if (pout) {
		close(pout[0]);
		xmove_fd(pout[1], 1);
	}

	iopp = t->ioact;
	if (iopp) {
		if (bltin && bltin != doexec) {
			prs(bltin_name);
			err(": can't redirect shell command");
			if (forked)
				_exit(-1);
			return -1;
		}
		while (*iopp) {
			if (iosetup(*iopp++, pin != NULL, pout != NULL)) {
				/* system-detected error */
				if (forked)
					_exit(-1);
				return -1;
			}
		}
	}

	if (bltin) {
		if (forked || pin || pout) {
			/* Builtin in pipe: disallowed */
			/* TODO: allow "exec"? */
			prs(bltin_name);
			err(": can't run builtin as part of pipe");
			if (forked)
				_exit(-1);
			return -1;
		}
		/* Run builtin */
		i = setstatus(bltin(t, wp));
		if (forked)
			_exit(i);
		DBGPRINTF(("FORKEXEC: returning i=%d\n", i));
		return i;
	}

	/* should use FIOCEXCL */
	for (i = FDBASE; i < NOFILE; i++)
		close(i);
	if (resetsig) {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
	}

	if (t->op_type == TPAREN)
		_exit(execute(t->left, NOPIPE, NOPIPE, /* no_fork: */ 1));
	if (wp[0] == NULL)
		_exit(EXIT_SUCCESS);

	cp = rexecve(wp[0], wp, makenv(0, NULL));
	prs(wp[0]);
	prs(": ");
	err(cp);
	if (!execflg)
		trap[0] = NULL;

	DBGPRINTF(("FORKEXEC: calling leave(), pid=%d\n", getpid()));

	leave();
	/* NOTREACHED */
	return 0;
}

/*
 * 0< 1> are ignored as required
 * within pipelines.
 */
static int iosetup(struct ioword *iop, int pipein, int pipeout)
{
	int u = -1;
	char *cp = NULL;
	const char *msg;

	DBGPRINTF(("IOSETUP: iop %p, pipein %i, pipeout %i\n", iop,
			   pipein, pipeout));

	if (iop->io_fd == IODEFAULT)	/* take default */
		iop->io_fd = iop->io_flag & (IOREAD | IOHERE) ? 0 : 1;

	if (pipein && iop->io_fd == 0)
		return 0;

	if (pipeout && iop->io_fd == 1)
		return 0;

	msg = iop->io_flag & (IOREAD | IOHERE) ? "open" : "create";
	if ((iop->io_flag & IOHERE) == 0) {
		cp = iop->io_name; /* huh?? */
		cp = evalstr(cp, DOSUB | DOTRIM);
		if (cp == NULL)
			return 1;
	}

	if (iop->io_flag & IODUP) {
		if (cp[1] || (!isdigit(*cp) && *cp != '-')) {
			prs(cp);
			err(": illegal >& argument");
			return 1;
		}
		if (*cp == '-')
			iop->io_flag = IOCLOSE;
		iop->io_flag &= ~(IOREAD | IOWRITE);
	}

	switch (iop->io_flag) {
	case IOREAD:
		u = open(cp, O_RDONLY);
		break;

	case IOHERE:
	case IOHERE | IOXHERE:
		u = herein(iop->io_name, iop->io_flag & IOXHERE);
		cp = (char*)"here file";
		break;

	case IOWRITE | IOCAT:
		u = open(cp, O_WRONLY);
		if (u >= 0) {
			lseek(u, (long) 0, SEEK_END);
			break;
		}
		/* fall through to creation if >>file doesn't exist */

	case IOWRITE:
		u = creat(cp, 0666);
		break;

	case IODUP:
		u = dup2(*cp - '0', iop->io_fd);
		break;

	case IOCLOSE:
		close(iop->io_fd);
		return 0;
	}

	if (u < 0) {
		prs(cp);
		prs(": can't ");
		warn(msg);
		return 1;
	}
	xmove_fd(u, iop->io_fd);
	return 0;
}

/*
 * Enter a new loop level (marked for break/continue).
 */
static void brkset(struct brkcon *bc)
{
	bc->nextlev = brklist;
	brklist = bc;
}

/*
 * Wait for the last process created.
 * Print a message for each process found
 * that was killed by a signal.
 * Ignore interrupt signals while waiting
 * unless `canintr' is true.
 */
static int waitfor(int lastpid, int canintr)
{
	int pid, rv;
	int s;
	smallint oheedint = heedint;

	heedint = 0;
	rv = 0;
	do {
		pid = wait(&s);
		if (pid == -1) {
			if (errno != EINTR || canintr)
				break;
		} else {
			rv = WAITSIG(s);
			if (rv != 0) {
				if (rv < ARRAY_SIZE(signame)) {
					if (signame[rv] != NULL) {
						if (pid != lastpid) {
							prn(pid);
							prs(": ");
						}
						prs(signame[rv]);
					}
				} else {
					if (pid != lastpid) {
						prn(pid);
						prs(": ");
					}
					prs("Signal ");
					prn(rv);
					prs(" ");
				}
				if (WAITCORE(s))
					prs(" - core dumped");
				if (rv >= ARRAY_SIZE(signame) || signame[rv])
					prs("\n");
				rv |= 0x80;
			} else
				rv = WAITVAL(s);
		}
	} while (pid != lastpid);
	heedint = oheedint;
	if (intr) {
		if (interactive) {
			if (canintr)
				intr = 0;
		} else {
			if (exstat == 0)
				exstat = rv;
			onintr(0);
		}
	}
	return rv;
}

static int setstatus(int s)
{
	exstat = s;
	setval(lookup("?"), putn(s));
	return s;
}

/*
 * PATH-searching interface to execve.
 * If getenv("PATH") were kept up-to-date,
 * execvp might be used.
 */
static const char *rexecve(char *c, char **v, char **envp)
{
	const char *sp;
	char *tp;
	int asis = 0;
	char *name = c;

	if (ENABLE_FEATURE_SH_STANDALONE) {
		if (find_applet_by_name(name) >= 0) {
			/* We have to exec here since we vforked.  Running
			 * run_applet_and_exit() won't work and bad things
			 * will happen. */
			execve(bb_busybox_exec_path, v, envp);
		}
	}

	DBGPRINTF(("REXECVE: c=%p, v=%p, envp=%p\n", c, v, envp));

	sp = any('/', c) ? "" : path->value;
	asis = (*sp == '\0');
	while (asis || *sp != '\0') {
		asis = 0;
		tp = global_env.linep;
		for (; *sp != '\0'; tp++) {
			*tp = *sp++;
			if (*tp == ':') {
				asis = (*sp == '\0');
				break;
			}
		}
		if (tp != global_env.linep)
			*tp++ = '/';
		strcpy(tp, c);

		DBGPRINTF3(("REXECVE: global_env.linep is %s\n", global_env.linep));

		execve(global_env.linep, v, envp);

		switch (errno) {
		case ENOEXEC:
			/* File is executable but file format isnt recognized */
			/* Run it as a shell script */
			/* (execve above didnt do it itself, unlike execvp) */
			*v = global_env.linep;
			v--;
			tp = *v;
			*v = (char*)DEFAULT_SHELL;
			execve(DEFAULT_SHELL, v, envp);
			*v = tp;
			return "no shell";

		case ENOMEM:
			return (char *) bb_msg_memory_exhausted;

		case E2BIG:
			return "argument list too long";
		}
	}
	if (errno == ENOENT) {
		exstat = 127; /* standards require this */
		return "not found";
	}
	exstat = 126; /* mimic bash */
	return "can't execute";
}

/*
 * Run the command produced by generator `f'
 * applied to stream `arg'.
 */
static int run(struct ioarg *argp, int (*f) (struct ioarg *))
{
	struct op *otree;
	struct wdblock *swdlist;
	struct wdblock *siolist;
	jmp_buf ev, rt;
	xint *ofail;
	int rv;

#if __GNUC__
	/* Avoid longjmp clobbering */
	(void) &rv;
#endif

	DBGPRINTF(("RUN: enter, areanum %d, outtree %p, failpt %p\n",
			   areanum, outtree, failpt));

	areanum++;
	swdlist = wdlist;
	siolist = iolist;
	otree = outtree;
	ofail = failpt;
	rv = -1;

	errpt = ev;
	if (newenv(setjmp(errpt)) == 0) {
		wdlist = NULL;
		iolist = NULL;
		pushio(argp, f);
		global_env.iobase = global_env.iop;
		yynerrs = 0;
		failpt = rt;
		if (setjmp(failpt) == 0 && yyparse() == 0)
			rv = execute(outtree, NOPIPE, NOPIPE, /* no_fork: */ 0);
		quitenv();
	} else {
		DBGPRINTF(("RUN: error from newenv()!\n"));
	}

	wdlist = swdlist;
	iolist = siolist;
	failpt = ofail;
	outtree = otree;
	freearea(areanum--);

	return rv;
}

/* -------- do.c -------- */

/*
 * built-in commands: doX
 */

static int dohelp(struct op *t UNUSED_PARAM, char **args UNUSED_PARAM)
{
	int col;
	const struct builtincmd *x;

	puts("\nBuilt-in commands:\n"
	     "-------------------");

	col = 0;
	x = builtincmds;
	while (x->name) {
		col += printf("%c%s", ((col == 0) ? '\t' : ' '), x->name);
		if (col > 60) {
			bb_putchar('\n');
			col = 0;
		}
		x++;
	}
#if ENABLE_FEATURE_SH_STANDALONE
	{
		const char *applet = applet_names;

		while (*applet) {
			col += printf("%c%s", ((col == 0) ? '\t' : ' '), applet);
			if (col > 60) {
				bb_putchar('\n');
				col = 0;
			}
			applet += strlen(applet) + 1;
		}
	}
#endif
	puts("\n");
	return EXIT_SUCCESS;
}

static int dolabel(struct op *t UNUSED_PARAM, char **args UNUSED_PARAM)
{
	return 0;
}

static int dochdir(struct op *t UNUSED_PARAM, char **args)
{
	const char *cp, *er;

	cp = args[1];
	if (cp == NULL) {
		cp = homedir->value;
		if (cp != NULL)
			goto do_cd;
		er = ": no home directory";
	} else {
 do_cd:
		if (chdir(cp) >= 0)
			return 0;
		er = ": bad directory";
	}
	prs(cp != NULL ? cp : "cd");
	err(er);
	return 1;
}

static int doshift(struct op *t UNUSED_PARAM, char **args)
{
	int n;

	n = args[1] ? getn(args[1]) : 1;
	if (dolc < n) {
		err("nothing to shift");
		return 1;
	}
	dolv[n] = dolv[0];
	dolv += n;
	dolc -= n;
	setval(lookup("#"), putn(dolc));
	return 0;
}

/*
 * execute login and newgrp directly
 */
static int dologin(struct op *t UNUSED_PARAM, char **args)
{
	const char *cp;

	if (interactive) {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
	}
	cp = rexecve(args[0], args, makenv(0, NULL));
	prs(args[0]);
	prs(": ");
	err(cp);
	return 1;
}

static int doumask(struct op *t UNUSED_PARAM, char **args)
{
	int i;
	char *cp;

	cp = args[1];
	if (cp == NULL) {
		i = umask(0);
		umask(i);
		printf("%04o\n", i);
	} else {
		i = bb_strtou(cp, NULL, 8);
		if (errno) {
			err("umask: bad octal number");
			return 1;
		}
		umask(i);
	}
	return 0;
}

static int doexec(struct op *t, char **args)
{
	jmp_buf ex;
	xint *ofail;
	char **sv_words;

	t->ioact = NULL;
	if (!args[1])
		return 1;

	execflg = 1;
	ofail = failpt;
	failpt = ex;

	sv_words = t->op_words;
	t->op_words = args + 1;
// TODO: test what will happen with "exec break" -
// will it leave t->op_words pointing to garbage?
// (see http://bugs.busybox.net/view.php?id=846)
	if (setjmp(failpt) == 0)
		execute(t, NOPIPE, NOPIPE, /* no_fork: */ 1);
	t->op_words = sv_words;

	failpt = ofail;
	execflg = 0;

	return 1;
}

static int dodot(struct op *t UNUSED_PARAM, char **args)
{
	int i;
	const char *sp;
	char *tp;
	char *cp;
	int maltmp;

	DBGPRINTF(("DODOT: enter, t=%p, tleft %p, tright %p, global_env.linep is %s\n",
		t, t->left, t->right, ((global_env.linep == NULL) ? "NULL" : global_env.linep)));

	cp = args[1];
	if (cp == NULL) {
		DBGPRINTF(("DODOT: bad args, ret 0\n"));
		return 0;
	}
	DBGPRINTF(("DODOT: cp is %s\n", cp));

	sp = any('/', cp) ? ":" : path->value;

	DBGPRINTF(("DODOT: sp is %s,  global_env.linep is %s\n",
			   ((sp == NULL) ? "NULL" : sp),
			   ((global_env.linep == NULL) ? "NULL" : global_env.linep)));

	while (*sp) {
		tp = global_env.linep;
		while (*sp && (*tp = *sp++) != ':')
			tp++;
		if (tp != global_env.linep)
			*tp++ = '/';
		strcpy(tp, cp);

		/* Original code */
		i = open(global_env.linep, O_RDONLY);
		if (i >= 0) {
			exstat = 0;
			maltmp = remap(i);
			DBGPRINTF(("DODOT: remap=%d, exstat=%d, global_env.iofd %d, i %d, global_env.linep is %s\n",
				maltmp, exstat, global_env.iofd, i, global_env.linep));

			next(maltmp);		/* Basically a PUSHIO */

			DBGPRINTF(("DODOT: returning exstat=%d\n", exstat));

			return exstat;
		}
	} /* while */

	prs(cp);
	err(": not found");

	return -1;
}

static int dowait(struct op *t UNUSED_PARAM, char **args)
{
	int i;
	char *cp;

	cp = args[1];
	if (cp != NULL) {
		i = getn(cp);
		if (i == 0)
			return 0;
	} else
		i = -1;
	setstatus(waitfor(i, 1));
	return 0;
}

static int doread(struct op *t UNUSED_PARAM, char **args)
{
	char *cp, **wp;
	int nb = 0;
	int nl = 0;

	if (args[1] == NULL) {
		err("Usage: read name ...");
		return 1;
	}
	for (wp = args + 1; *wp; wp++) {
		for (cp = global_env.linep; !nl && cp < elinep - 1; cp++) {
			nb = nonblock_safe_read(STDIN_FILENO, cp, sizeof(*cp));
			if (nb != sizeof(*cp))
				break;
			nl = (*cp == '\n');
			if (nl || (wp[1] && any(*cp, ifs->value)))
				break;
		}
		*cp = '\0';
		if (nb <= 0)
			break;
		setval(lookup(*wp), global_env.linep);
	}
	return nb <= 0;
}

static int doeval(struct op *t UNUSED_PARAM, char **args)
{
	return RUN(awordlist, args + 1, wdchar);
}

static int dotrap(struct op *t UNUSED_PARAM, char **args)
{
	int n, i;
	int resetsig;

	if (args[1] == NULL) {
		for (i = 0; i <= _NSIG; i++)
			if (trap[i]) {
				prn(i);
				prs(": ");
				prs(trap[i]);
				prs("\n");
			}
		return 0;
	}
	resetsig = isdigit(args[1][0]);
	for (i = resetsig ? 1 : 2; args[i] != NULL; ++i) {
		n = getsig(args[i]);
		freecell(trap[n]);
		trap[n] = 0;
		if (!resetsig) {
			if (args[1][0] != '\0') {
				trap[n] = strsave(args[1], 0);
				setsig(n, sig);
			} else
				setsig(n, SIG_IGN);
		} else {
			if (interactive) {
				if (n == SIGINT)
					setsig(n, onintr);
				else
					setsig(n, n == SIGQUIT ? SIG_IGN : SIG_DFL);
			} else
				setsig(n, SIG_DFL);
		}
	}
	return 0;
}

static int getsig(char *s)
{
	int n;

	n = getn(s);
	if (n < 0 || n > _NSIG) {
		err("trap: bad signal number");
		n = 0;
	}
	return n;
}

static void setsig(int n, sighandler_t f)
{
	if (n == 0)
		return;
	if (signal(n, SIG_IGN) != SIG_IGN || ourtrap[n]) {
		ourtrap[n] = 1;
		signal(n, f);
	}
}

static int getn(char *as)
{
	char *s;
	int n, m;

	s = as;
	m = 1;
	if (*s == '-') {
		m = -1;
		s++;
	}
	for (n = 0; isdigit(*s); s++)
		n = (n * 10) + (*s - '0');
	if (*s) {
		prs(as);
		err(": bad number");
	}
	return n * m;
}

static int dobreak(struct op *t UNUSED_PARAM, char **args)
{
	return brkcontin(args[1], 1);
}

static int docontinue(struct op *t UNUSED_PARAM, char **args)
{
	return brkcontin(args[1], 0);
}

static int brkcontin(char *cp, int val)
{
	struct brkcon *bc;
	int nl;

	nl = cp == NULL ? 1 : getn(cp);
	if (nl <= 0)
		nl = 999;
	do {
		bc = brklist;
		if (bc == NULL)
			break;
		brklist = bc->nextlev;
	} while (--nl);
	if (nl) {
		err("bad break/continue level");
		return 1;
	}
	isbreak = (val != 0);
	longjmp(bc->brkpt, 1);
	/* NOTREACHED */
}

static int doexit(struct op *t UNUSED_PARAM, char **args)
{
	char *cp;

	execflg = 0;
	cp = args[1];
	if (cp != NULL)
		setstatus(getn(cp));

	DBGPRINTF(("DOEXIT: calling leave(), t=%p\n", t));

	leave();
	/* NOTREACHED */
	return 0;
}

static int doexport(struct op *t UNUSED_PARAM, char **args)
{
	rdexp(args + 1, export, EXPORT);
	return 0;
}

static int doreadonly(struct op *t UNUSED_PARAM, char **args)
{
	rdexp(args + 1, ronly, RONLY);
	return 0;
}

static void rdexp(char **wp, void (*f) (struct var *), int key)
{
	DBGPRINTF6(("RDEXP: enter, wp=%p, func=%p, key=%d\n", wp, f, key));
	DBGPRINTF6(("RDEXP: *wp=%s\n", *wp));

	if (*wp != NULL) {
		for (; *wp != NULL; wp++) {
			if (isassign(*wp)) {
				char *cp;

				assign(*wp, COPYV);
				for (cp = *wp; *cp != '='; cp++)
					continue;
				*cp = '\0';
			}
			if (checkname(*wp))
				(*f) (lookup(*wp));
			else
				badid(*wp);
		}
	} else
		putvlist(key, 1);
}

static void badid(char *s)
{
	prs(s);
	err(": bad identifier");
}

static int doset(struct op *t UNUSED_PARAM, char **args)
{
	struct var *vp;
	char *cp;
	int n;

	cp = args[1];
	if (cp == NULL) {
		for (vp = vlist; vp; vp = vp->next)
			varput(vp->name, 1);
		return 0;
	}
	if (*cp == '-') {
		args++;
		if (*++cp == 0)
			FLAG['x'] = FLAG['v'] = 0;
		else {
			for (; *cp; cp++) {
				switch (*cp) {
				case 'e':
					if (!interactive)
						FLAG['e']++;
					break;

				default:
					if (*cp >= 'a' && *cp <= 'z')
						FLAG[(int) *cp]++;
					break;
				}
			}
		}
		setdash();
	}
	if (args[1]) {
		args[0] = dolv[0];
		for (n = 1; args[n]; n++)
			setarea((char *) args[n], 0);
		dolc = n - 1;
		dolv = args;
		setval(lookup("#"), putn(dolc));
		setarea((char *) (dolv - 1), 0);
	}
	return 0;
}

static void varput(char *s, int out)
{
	if (isalnum(*s) || *s == '_') {
		write(out, s, strlen(s));
		write(out, "\n", 1);
	}
}


/*
 * Copyright (c) 1999 Herbert Xu <herbert@debian.org>
 * This file contains code for the times builtin.
 */
static void times_fmt(char *buf, clock_t val, unsigned clk_tck)
{
	unsigned min, sec;
	if (sizeof(val) > sizeof(int))
		sec = ((unsigned long)val) / clk_tck;
	else
		sec = ((unsigned)val) / clk_tck;
	min = sec / 60;
#if ENABLE_DESKTOP
	sprintf(buf, "%um%u.%03us", min, (sec - min * 60),
	/* msec: */ ((unsigned)(val - (clock_t)sec * clk_tck)) * 1000 / clk_tck
	);
#else
	sprintf(buf, "%um%us", min, (sec - min * 60));
#endif
}

static int dotimes(struct op *t UNUSED_PARAM, char **args UNUSED_PARAM)
{
	struct tms buf;
	unsigned clk_tck = sysconf(_SC_CLK_TCK);
	/* How much do we need for "NmN.NNNs" ? */
	enum { TIMEBUF_SIZE = sizeof(int)*3 + sizeof(int)*3 + 6 };
	char u[TIMEBUF_SIZE], s[TIMEBUF_SIZE];
	char cu[TIMEBUF_SIZE], cs[TIMEBUF_SIZE];

	times(&buf);

	times_fmt(u, buf.tms_utime, clk_tck);
	times_fmt(s, buf.tms_stime, clk_tck);
	times_fmt(cu, buf.tms_cutime, clk_tck);
	times_fmt(cs, buf.tms_cstime, clk_tck);

	printf("%s %s\n%s %s\n", u, s, cu, cs);
	return 0;
}


/* -------- eval.c -------- */

/*
 * ${}
 * `command`
 * blank interpretation
 * quoting
 * glob
 */

static char **eval(char **ap, int f)
{
	struct wdblock *wb;
	char **wp;
	char **wf;
	jmp_buf ev;

#if __GNUC__
	/* Avoid longjmp clobbering */
	(void) &wp;
	(void) &ap;
#endif

	DBGPRINTF4(("EVAL: enter, f=%d\n", f));

	wp = NULL;
	wb = NULL;
	wf = NULL;
	errpt = ev;
	if (newenv(setjmp(errpt)) == 0) {
		while (*ap && isassign(*ap))
			expand(*ap++, &wb, f & ~DOGLOB);
		if (FLAG['k']) {
			for (wf = ap; *wf; wf++) {
				if (isassign(*wf))
					expand(*wf, &wb, f & ~DOGLOB);
			}
		}
		for (wb = addword((char *) NULL, wb); *ap; ap++) {
			if (!FLAG['k'] || !isassign(*ap))
				expand(*ap, &wb, f & ~DOKEY);
		}
		wb = addword((char *) 0, wb);
		wp = getwords(wb);
		quitenv();
	} else
		gflg = 1;

	return gflg ? (char **) NULL : wp;
}


/*
 * Make the exported environment from the exported
 * names in the dictionary. Keyword assignments
 * will already have been done.
 */
static char **makenv(int all, struct wdblock *wb)
{
	struct var *vp;

	DBGPRINTF5(("MAKENV: enter, all=%d\n", all));

	for (vp = vlist; vp; vp = vp->next)
		if (all || vp->status & EXPORT)
			wb = addword(vp->name, wb);
	wb = addword((char *) 0, wb);
	return getwords(wb);
}

static int expand(const char *cp, struct wdblock **wbp, int f)
{
	jmp_buf ev;
	char *xp;

#if __GNUC__
	/* Avoid longjmp clobbering */
	(void) &cp;
#endif

	DBGPRINTF3(("EXPAND: enter, f=%d\n", f));

	gflg = 0;

	if (cp == NULL)
		return 0;

	if (!anys("$`'\"", cp) && !anys(ifs->value, cp)
	 && ((f & DOGLOB) == 0 || !anys("[*?", cp))
	) {
		xp = strsave(cp, areanum);
		if (f & DOTRIM)
			unquote(xp);
		*wbp = addword(xp, *wbp);
		return 1;
	}
	errpt = ev;
	if (newenv(setjmp(errpt)) == 0) {
		PUSHIO(aword, cp, strchar);
		global_env.iobase = global_env.iop;
		while ((xp = blank(f)) && gflg == 0) {
			global_env.linep = xp;
			xp = strsave(xp, areanum);
			if ((f & DOGLOB) == 0) {
				if (f & DOTRIM)
					unquote(xp);
				*wbp = addword(xp, *wbp);
			} else
				*wbp = glob(xp, *wbp);
		}
		quitenv();
	} else
		gflg = 1;
	return gflg == 0;
}

static char *evalstr(char *cp, int f)
{
	struct wdblock *wb;

	DBGPRINTF6(("EVALSTR: enter, cp=%p, f=%d\n", cp, f));

	wb = NULL;
	if (expand(cp, &wb, f)) {
		if (wb == NULL || wb->w_nword == 0
		 || (cp = wb->w_words[0]) == NULL
		) {
// TODO: I suspect that
// char *evalstr(char *cp, int f)  is actually
// const char *evalstr(const char *cp, int f)!
			cp = (char*)"";
		}
		DELETE(wb);
	} else
		cp = NULL;
	return cp;
}


/*
 * Blank interpretation and quoting
 */
static char *blank(int f)
{
	int c, c1;
	char *sp;
	int scanequals, foundequals;

	DBGPRINTF3(("BLANK: enter, f=%d\n", f));

	sp = global_env.linep;
	scanequals = f & DOKEY;
	foundequals = 0;

 loop:
	c = subgetc('"', foundequals);
	switch (c) {
	case 0:
		if (sp == global_env.linep)
			return 0;
		*global_env.linep++ = 0;
		return sp;

	default:
		if (f & DOBLANK && any(c, ifs->value))
			goto loop;
		break;

	case '"':
	case '\'':
		scanequals = 0;
		if (INSUB())
			break;
		for (c1 = c; (c = subgetc(c1, 1)) != c1;) {
			if (c == 0)
				break;
			if (c == '\'' || !any(c, "$`\""))
				c |= QUOTE;
			*global_env.linep++ = c;
		}
		c = 0;
	}
	unget(c);
	if (!isalpha(c) && c != '_')
		scanequals = 0;
	for (;;) {
		c = subgetc('"', foundequals);
		if (c == 0 ||
			f & (DOBLANK && any(c, ifs->value)) ||
			(!INSUB() && any(c, "\"'"))) {
			scanequals = 0;
			unget(c);
			if (any(c, "\"'"))
				goto loop;
			break;
		}
		if (scanequals) {
			if (c == '=') {
				foundequals = 1;
				scanequals = 0;
			} else if (!isalnum(c) && c != '_')
				scanequals = 0;
		}
		*global_env.linep++ = c;
	}
	*global_env.linep++ = 0;
	return sp;
}

/*
 * Get characters, substituting for ` and $
 */
static int subgetc(char ec, int quoted)
{
	char c;

	DBGPRINTF3(("SUBGETC: enter, quoted=%d\n", quoted));

 again:
	c = my_getc(ec);
	if (!INSUB() && ec != '\'') {
		if (c == '`') {
			if (grave(quoted) == 0)
				return 0;
			global_env.iop->task = XGRAVE;
			goto again;
		}
		if (c == '$') {
			c = dollar(quoted);
			if (c == 0) {
				global_env.iop->task = XDOLL;
				goto again;
			}
		}
	}
	return c;
}

/*
 * Prepare to generate the string returned by ${} substitution.
 */
static int dollar(int quoted)
{
	int otask;
	struct io *oiop;
	char *dolp;
	char *s, c, *cp = NULL;
	struct var *vp;

	DBGPRINTF3(("DOLLAR: enter, quoted=%d\n", quoted));

	c = readc();
	s = global_env.linep;
	if (c != '{') {
		*global_env.linep++ = c;
		if (isalpha(c) || c == '_') {
			while ((c = readc()) != 0 && (isalnum(c) || c == '_'))
				if (global_env.linep < elinep)
					*global_env.linep++ = c;
			unget(c);
		}
		c = 0;
	} else {
		oiop = global_env.iop;
		otask = global_env.iop->task;

		global_env.iop->task = XOTHER;
		while ((c = subgetc('"', 0)) != 0 && c != '}' && c != '\n')
			if (global_env.linep < elinep)
				*global_env.linep++ = c;
		if (oiop == global_env.iop)
			global_env.iop->task = otask;
		if (c != '}') {
			err("unclosed ${");
			gflg = 1;
			return c;
		}
	}
	if (global_env.linep >= elinep) {
		err("string in ${} too long");
		gflg = 1;
		global_env.linep -= 10;
	}
	*global_env.linep = 0;
	if (*s)
		for (cp = s + 1; *cp; cp++)
			if (any(*cp, "=-+?")) {
				c = *cp;
				*cp++ = 0;
				break;
			}
	if (s[1] == 0 && (*s == '*' || *s == '@')) {
		if (dolc > 1) {
			/* currently this does not distinguish $* and $@ */
			/* should check dollar */
			global_env.linep = s;
			PUSHIO(awordlist, dolv + 1, dolchar);
			return 0;
		} else {				/* trap the nasty ${=} */
			s[0] = '1';
			s[1] = '\0';
		}
	}
	vp = lookup(s);
	dolp = vp->value;
	if (dolp == null) {
		switch (c) {
		case '=':
			if (isdigit(*s)) {
				err("can't use ${...=...} with $n");
				gflg = 1;
				break;
			}
			setval(vp, cp);
			dolp = vp->value;
			break;

		case '-':
			dolp = strsave(cp, areanum);
			break;

		case '?':
			if (*cp == 0) {
				prs("missing value for ");
				err(s);
			} else
				err(cp);
			gflg = 1;
			break;
		}
	} else if (c == '+')
		dolp = strsave(cp, areanum);
	if (FLAG['u'] && dolp == null) {
		prs("unset variable: ");
		err(s);
		gflg = 1;
	}
	global_env.linep = s;
	PUSHIO(aword, dolp, quoted ? qstrchar : strchar);
	return 0;
}

/*
 * Run the command in `...` and read its output.
 */

static int grave(int quoted)
{
	/* moved to G: static char child_cmd[LINELIM]; */

	const char *cp;
	int i;
	int j;
	int pf[2];
	const char *src;
	char *dest;
	int count;
	int ignore;
	int ignore_once;
	char *argument_list[4];
	struct wdblock *wb = NULL;

#if __GNUC__
	/* Avoid longjmp clobbering */
	(void) &cp;
#endif

	for (cp = global_env.iop->argp->aword; *cp != '`'; cp++) {
		if (*cp == 0) {
			err("no closing `");
			return 0;
		}
	}

	/* string copy with dollar expansion */
	src = global_env.iop->argp->aword;
	dest = child_cmd;
	count = 0;
	ignore = 0;
	ignore_once = 0;
	while ((*src != '`') && (count < LINELIM)) {
		if (*src == '\'')
			ignore = !ignore;
		if (*src == '\\')
			ignore_once = 1;
		if (*src == '$' && !ignore && !ignore_once) {
			struct var *vp;
			/* moved to G to reduce stack usage
			char var_name[LINELIM];
			char alt_value[LINELIM];
			*/
#define var_name (G.grave__var_name)
#define alt_value (G.grave__alt_value)
			int var_index = 0;
			int alt_index = 0;
			char operator = 0;
			int braces = 0;
			char *value;

			src++;
			if (*src == '{') {
				braces = 1;
				src++;
			}

			var_name[var_index++] = *src++;
			while (isalnum(*src) || *src=='_')
				var_name[var_index++] = *src++;
			var_name[var_index] = 0;

			if (braces) {
				switch (*src) {
				case '}':
					break;
				case '-':
				case '=':
				case '+':
				case '?':
					operator = * src;
					break;
				default:
					err("unclosed ${\n");
					return 0;
				}
				if (operator) {
					src++;
					while (*src && (*src != '}')) {
						alt_value[alt_index++] = *src++;
					}
					alt_value[alt_index] = 0;
					if (*src != '}') {
						err("unclosed ${\n");
						return 0;
					}
				}
				src++;
			}

			if (isalpha(*var_name)) {
				/* let subshell handle it instead */

				char *namep = var_name;

				*dest++ = '$';
				if (braces)
					*dest++ = '{';
				while (*namep)
					*dest++ = *namep++;
				if (operator) {
					char *altp = alt_value;
					*dest++ = operator;
					while (*altp)
						*dest++ = *altp++;
				}
				if (braces)
					*dest++ = '}';

				wb = addword(lookup(var_name)->name, wb);
			} else {
				/* expand */

				vp = lookup(var_name);
				if (vp->value != null)
					value = (operator == '+') ?
						alt_value : vp->value;
				else if (operator == '?') {
					err(alt_value);
					return 0;
				} else if (alt_index && (operator != '+')) {
					value = alt_value;
					if (operator == '=')
						setval(vp, value);
				} else
					continue;

				while (*value && (count < LINELIM)) {
					*dest++ = *value++;
					count++;
				}
			}
#undef var_name
#undef alt_value
		} else {
			*dest++ = *src++;
			count++;
			ignore_once = 0;
		}
	}
	*dest = '\0';

	if (openpipe(pf) < 0)
		return 0;

	while ((i = vfork()) == -1 && errno == EAGAIN)
		continue;

	DBGPRINTF3(("GRAVE: i is %p\n", io));

	if (i < 0) {
		closepipe(pf);
		err((char *) bb_msg_memory_exhausted);
		return 0;
	}
	if (i != 0) {
		waitpid(i, NULL, 0); // safe_waitpid?
		global_env.iop->argp->aword = ++cp;
		close(pf[1]);
		PUSHIO(afile, remap(pf[0]),
			(int (*)(struct ioarg *)) ((quoted) ? qgravechar : gravechar));
		return 1;
	}
	/* allow trapped signals */
	/* XXX - Maybe this signal stuff should go as well? */
	for (j = 0; j <= _NSIG; j++)
		if (ourtrap[j] && signal(j, SIG_IGN) != SIG_IGN)
			signal(j, SIG_DFL);

	/* Testcase where below checks are needed:
	 * close stdout & run this script:
	 *  files=`ls`
	 *  echo "$files" >zz
	 */
	xmove_fd(pf[1], 1);
	if (pf[0] != 1)
		close(pf[0]);

	argument_list[0] = (char *) DEFAULT_SHELL;
	argument_list[1] = (char *) "-c";
	argument_list[2] = child_cmd;
	argument_list[3] = NULL;

	cp = rexecve(argument_list[0], argument_list, makenv(1, wb));
	prs(argument_list[0]);
	prs(": ");
	err(cp);
	_exit(EXIT_FAILURE);
}


static char *unquote(char *as)
{
	char *s;

	s = as;
	if (s != NULL)
		while (*s)
			*s++ &= ~QUOTE;
	return as;
}

/* -------- glob.c -------- */

/*
 * glob
 */

#define	scopy(x) strsave((x), areanum)
#define	BLKSIZ	512
#define	NDENT	((BLKSIZ+sizeof(struct dirent)-1)/sizeof(struct dirent))

static struct wdblock *cl, *nl;
static const char spcl[] ALIGN1= "[?*";

static struct wdblock *glob(char *cp, struct wdblock *wb)
{
	int i;
	char *pp;

	if (cp == 0)
		return wb;
	i = 0;
	for (pp = cp; *pp; pp++)
		if (any(*pp, spcl))
			i++;
		else if (!any(*pp & ~QUOTE, spcl))
			*pp &= ~QUOTE;
	if (i != 0) {
		for (cl = addword(scopy(cp), NULL); anyspcl(cl); cl = nl) {
			nl = newword(cl->w_nword * 2);
			for (i = 0; i < cl->w_nword; i++) {	/* for each argument */
				for (pp = cl->w_words[i]; *pp; pp++)
					if (any(*pp, spcl)) {
						globname(cl->w_words[i], pp);
						break;
					}
				if (*pp == '\0')
					nl = addword(scopy(cl->w_words[i]), nl);
			}
			for (i = 0; i < cl->w_nword; i++)
				DELETE(cl->w_words[i]);
			DELETE(cl);
		}
		if (cl->w_nword) {
			for (i = 0; i < cl->w_nword; i++)
				unquote(cl->w_words[i]);
			qsort_string_vector(cl->w_words, cl->w_nword);
			for (i = 0; i < cl->w_nword; i++)
				wb = addword(cl->w_words[i], wb);
			DELETE(cl);
			return wb;
		}
	}
	wb = addword(unquote(cp), wb);
	return wb;
}

static void globname(char *we, char *pp)
{
	char *np, *cp;
	char *name, *gp, *dp;
	int k;
	DIR *dirp;
	struct dirent *de;
	char dname[NAME_MAX + 1];
	struct stat dbuf;

	for (np = we; np != pp; pp--)
		if (pp[-1] == '/')
			break;
	dp = cp = get_space((int) (pp - np) + 3);
	while (np < pp)
		*cp++ = *np++;
	*cp++ = '.';
	*cp = '\0';
	gp = cp = get_space(strlen(pp) + 1);
	while (*np && *np != '/')
		*cp++ = *np++;
	*cp = '\0';
	dirp = opendir(dp);
	if (dirp == 0) {
		DELETE(dp);
		DELETE(gp);
		return;
	}
	dname[NAME_MAX] = '\0';
	while ((de = readdir(dirp)) != NULL) {
		/* XXX Hmmm... What this could be? (abial) */
		/* if (ent[j].d_ino == 0) continue;
		 */
		strncpy(dname, de->d_name, NAME_MAX);
		if (dname[0] == '.')
			if (*gp != '.')
				continue;
		for (k = 0; k < NAME_MAX; k++)
			if (any(dname[k], spcl))
				dname[k] |= QUOTE;
		if (gmatch(dname, gp)) {
			name = generate(we, pp, dname, np);
			if (*np && !anys(np, spcl)) {
				if (stat(name, &dbuf)) {
					DELETE(name);
					continue;
				}
			}
			nl = addword(name, nl);
		}
	}
	closedir(dirp);
	DELETE(dp);
	DELETE(gp);
}

/*
 * generate a pathname as below.
 * start..end1 / middle end
 * the slashes come for free
 */
static char *generate(char *start1, char *end1, char *middle, char *end)
{
	char *p;
	char *op, *xp;

	p = op = get_space((int)(end1 - start1) + strlen(middle) + strlen(end) + 2);
	xp = start1;
	while (xp != end1)
		*op++ = *xp++;
	xp = middle;
	while (*xp != '\0')
		*op++ = *xp++;
	strcpy(op, end);
	return p;
}

static int anyspcl(struct wdblock *wb)
{
	int i;
	char **wd;

	wd = wb->w_words;
	for (i = 0; i < wb->w_nword; i++)
		if (anys(spcl, *wd++))
			return 1;
	return 0;
}


/* -------- word.c -------- */

static struct wdblock *newword(int nw)
{
	struct wdblock *wb;

	wb = get_space(sizeof(*wb) + nw * sizeof(char *));
	wb->w_bsize = nw;
	wb->w_nword = 0;
	return wb;
}

static struct wdblock *addword(char *wd, struct wdblock *wb)
{
	struct wdblock *wb2;
	int nw;

	if (wb == NULL)
		wb = newword(NSTART);
	nw = wb->w_nword;
	if (nw >= wb->w_bsize) {
		wb2 = newword(nw * 2);
		memcpy((char *) wb2->w_words, (char *) wb->w_words,
			   nw * sizeof(char *));
		wb2->w_nword = nw;
		DELETE(wb);
		wb = wb2;
	}
	wb->w_words[wb->w_nword++] = wd;
	return wb;
}

static char **getwords(struct wdblock *wb)
{
	char **wd;
	int nb;

	if (wb == NULL)
		return NULL;
	if (wb->w_nword == 0) {
		DELETE(wb);
		return NULL;
	}
	nb = sizeof(*wd) * wb->w_nword;
	wd = get_space(nb);
	memcpy(wd, wb->w_words, nb);
	DELETE(wb);			/* perhaps should done by caller */
	return wd;
}


/* -------- io.c -------- */

/*
 * shell IO
 */

static int my_getc(int ec)
{
	int c;

	if (global_env.linep > elinep) {
		while ((c = readc()) != '\n' && c)
			continue;
		err("input line too long");
		gflg = 1;
		return c;
	}
	c = readc();
	if ((ec != '\'') && (ec != '`') && (global_env.iop->task != XGRAVE)) {
		if (c == '\\') {
			c = readc();
			if (c == '\n' && ec != '\"')
				return my_getc(ec);
			c |= QUOTE;
		}
	}
	return c;
}

static void unget(int c)
{
	if (global_env.iop >= global_env.iobase)
		global_env.iop->peekc = c;
}

static int eofc(void)
{
	return global_env.iop < global_env.iobase || (global_env.iop->peekc == 0 && global_env.iop->prev == 0);
}

static int readc(void)
{
	int c;

	RCPRINTF(("READC: global_env.iop %p, global_env.iobase %p\n", global_env.iop, global_env.iobase));

	for (; global_env.iop >= global_env.iobase; global_env.iop--) {
		RCPRINTF(("READC: global_env.iop %p, peekc 0x%x\n", global_env.iop, global_env.iop->peekc));
		c = global_env.iop->peekc;
		if (c != '\0') {
			global_env.iop->peekc = 0;
			return c;
		}
		if (global_env.iop->prev != 0) {
			c = (*global_env.iop->iofn)(global_env.iop->argp, global_env.iop);
			if (c != '\0') {
				if (c == -1) {
					global_env.iop++;
					continue;
				}
				if (global_env.iop == iostack)
					ioecho(c);
				global_env.iop->prev = c;
				return c;
			}
			if (global_env.iop->task == XIO && global_env.iop->prev != '\n') {
				global_env.iop->prev = 0;
				if (global_env.iop == iostack)
					ioecho('\n');
				return '\n';
			}
		}
		if (global_env.iop->task == XIO) {
			if (multiline) {
				global_env.iop->prev = 0;
				return 0;
			}
			if (interactive && global_env.iop == iostack + 1) {
#if ENABLE_FEATURE_EDITING
				current_prompt = prompt->value;
#else
				prs(prompt->value);
#endif
			}
		}
	}							/* FOR */

	if (global_env.iop >= iostack) {
		RCPRINTF(("READC: return 0, global_env.iop %p\n", global_env.iop));
		return 0;
	}

	DBGPRINTF(("READC: leave()...\n"));
	leave();
	/* NOTREACHED */
	return 0;
}

static void ioecho(char c)
{
	if (FLAG['v'])
		write(STDERR_FILENO, &c, sizeof c);
}

static void pushio(struct ioarg *argp, int (*fn) (struct ioarg *))
{
	DBGPRINTF(("PUSHIO: argp %p, argp->afid 0x%x, global_env.iop %p\n", argp,
			   argp->afid, global_env.iop));

	/* Set env ptr for io source to next array spot and check for array overflow */
	if (++global_env.iop >= &iostack[NPUSH]) {
		global_env.iop--;
		err("Shell input nested too deeply");
		gflg = 1;
		return;
	}

	/* We did not overflow the NPUSH array spots so setup data structs */

	global_env.iop->iofn = (int (*)(struct ioarg *, struct io *)) fn;	/* Store data source func ptr */

	if (argp->afid != AFID_NOBUF)
		global_env.iop->argp = argp;
	else {

		global_env.iop->argp = ioargstack + (global_env.iop - iostack);	/* MAL - index into stack */
		*global_env.iop->argp = *argp;	/* copy data from temp area into stack spot */

		/* MAL - mainbuf is for 1st data source (command line?) and all nested use a single shared buffer? */

		if (global_env.iop == &iostack[0])
			global_env.iop->argp->afbuf = &mainbuf;
		else
			global_env.iop->argp->afbuf = &sharedbuf;

		/* MAL - if not a termimal AND (commandline OR readable file) then give it a buffer id? */
		/* This line appears to be active when running scripts from command line */
		if ((isatty(global_env.iop->argp->afile) == 0)
			&& (global_env.iop == &iostack[0]
				|| lseek(global_env.iop->argp->afile, 0L, SEEK_CUR) != -1)) {
			if (++bufid == AFID_NOBUF)	/* counter rollover check, AFID_NOBUF = 11111111  */
				bufid = AFID_ID;	/* AFID_ID = 0 */

			global_env.iop->argp->afid = bufid;	/* assign buffer id */
		}

		DBGPRINTF(("PUSHIO: iostack %p,  global_env.iop %p, afbuf %p\n",
				   iostack, global_env.iop, global_env.iop->argp->afbuf));
		DBGPRINTF(("PUSHIO: mbuf %p, sbuf %p, bid %d, global_env.iop %p\n",
				   &mainbuf, &sharedbuf, bufid, global_env.iop));

	}

	global_env.iop->prev = ~'\n';
	global_env.iop->peekc = 0;
	global_env.iop->xchar = 0;
	global_env.iop->nlcount = 0;

	if (fn == filechar || fn == linechar)
		global_env.iop->task = XIO;
	else if (fn == (int (*)(struct ioarg *)) gravechar
	      || fn == (int (*)(struct ioarg *)) qgravechar)
		global_env.iop->task = XGRAVE;
	else
		global_env.iop->task = XOTHER;
}

static struct io *setbase(struct io *ip)
{
	struct io *xp;

	xp = global_env.iobase;
	global_env.iobase = ip;
	return xp;
}

/*
 * Input generating functions
 */

/*
 * Produce the characters of a string, then a newline, then NUL.
 */
static int nlchar(struct ioarg *ap)
{
	char c;

	if (ap->aword == NULL)
		return '\0';
	c = *ap->aword++;
	if (c == '\0') {
		ap->aword = NULL;
		return '\n';
	}
	return c;
}

/*
 * Given a list of words, produce the characters
 * in them, with a space after each word.
 */
static int wdchar(struct ioarg *ap)
{
	char c;
	char **wl;

	wl = ap->awordlist;
	if (wl == NULL)
		return 0;
	if (*wl != NULL) {
		c = *(*wl)++;
		if (c != 0)
			return c & 0177;
		ap->awordlist++;
		return ' ';
	}
	ap->awordlist = NULL;
	return '\n';
}

/*
 * Return the characters of a list of words,
 * producing a space between them.
 */
static int dolchar(struct ioarg *ap)
{
	char *wp;

	wp = *ap->awordlist++;
	if (wp != NULL) {
		PUSHIO(aword, wp, *ap->awordlist == NULL ? strchar : xxchar);
		return -1;
	}
	return 0;
}

static int xxchar(struct ioarg *ap)
{
	int c;

	if (ap->aword == NULL)
		return 0;
	c = *ap->aword++;
	if (c == '\0') {
		ap->aword = NULL;
		return ' ';
	}
	return c;
}

/*
 * Produce the characters from a single word (string).
 */
static int strchar(struct ioarg *ap)
{
	if (ap->aword == NULL)
		return 0;
	return *ap->aword++;
}

/*
 * Produce quoted characters from a single word (string).
 */
static int qstrchar(struct ioarg *ap)
{
	int c;

	if (ap->aword == NULL)
		return 0;
	c = *ap->aword++;
	if (c)
		c |= QUOTE;
	return c;
}

/*
 * Return the characters from a file.
 */
static int filechar(struct ioarg *ap)
{
	int i;
	char c;
	struct iobuf *bp = ap->afbuf;

	if (ap->afid != AFID_NOBUF) {
		i = (ap->afid != bp->id);
		if (i || bp->bufp == bp->ebufp) {
			if (i)
				lseek(ap->afile, ap->afpos, SEEK_SET);

			i = nonblock_safe_read(ap->afile, bp->buf, sizeof(bp->buf));
			if (i <= 0) {
				closef(ap->afile);
				return 0;
			}

			bp->id = ap->afid;
			bp->bufp = bp->buf;
			bp->ebufp = bp->bufp + i;
		}

		ap->afpos++;
		return *bp->bufp++ & 0177;
	}
#if ENABLE_FEATURE_EDITING
	if (interactive && isatty(ap->afile)) {
		/* moved to G: static char filechar_cmdbuf[BUFSIZ]; */
		static int position = 0, size = 0;

		while (size == 0 || position >= size) {
			size = read_line_input(current_prompt, filechar_cmdbuf, BUFSIZ, line_input_state);
			if (size < 0) /* Error/EOF */
				exit(EXIT_SUCCESS);
			position = 0;
			/* if Ctrl-C, size == 0 and loop will repeat */
		}
		c = filechar_cmdbuf[position];
		position++;
		return c;
	}
#endif
	i = nonblock_safe_read(ap->afile, &c, sizeof(c));
	return i == sizeof(c) ? (c & 0x7f) : (closef(ap->afile), 0);
}

/*
 * Return the characters from a here temp file.
 */
static int herechar(struct ioarg *ap)
{
	char c;

	if (nonblock_safe_read(ap->afile, &c, sizeof(c)) != sizeof(c)) {
		close(ap->afile);
		c = '\0';
	}
	return c;
}

/*
 * Return the characters produced by a process (`...`).
 * Quote them if required, and remove any trailing newline characters.
 */
static int gravechar(struct ioarg *ap, struct io *iop)
{
	int c;

	c = qgravechar(ap, iop) & ~QUOTE;
	if (c == '\n')
		c = ' ';
	return c;
}

static int qgravechar(struct ioarg *ap, struct io *iop)
{
	int c;

	DBGPRINTF3(("QGRAVECHAR: enter, ap=%p, iop=%p\n", ap, iop));

	if (iop->xchar) {
		if (iop->nlcount) {
			iop->nlcount--;
			return '\n' | QUOTE;
		}
		c = iop->xchar;
		iop->xchar = 0;
	} else if ((c = filechar(ap)) == '\n') {
		iop->nlcount = 1;
		while ((c = filechar(ap)) == '\n')
			iop->nlcount++;
		iop->xchar = c;
		if (c == 0)
			return c;
		iop->nlcount--;
		c = '\n';
	}
	return c != 0 ? c | QUOTE : 0;
}

/*
 * Return a single command (usually the first line) from a file.
 */
static int linechar(struct ioarg *ap)
{
	int c;

	c = filechar(ap);
	if (c == '\n') {
		if (!multiline) {
			closef(ap->afile);
			ap->afile = -1;		/* illegal value */
		}
	}
	return c;
}

/*
 * Remap fd into shell's fd space
 */
static int remap(int fd)
{
	int i;
	int map[NOFILE];
	int newfd;

	DBGPRINTF(("REMAP: fd=%d, global_env.iofd=%d\n", fd, global_env.iofd));

	if (fd < global_env.iofd) {
		for (i = 0; i < NOFILE; i++)
			map[i] = 0;

		do {
			map[fd] = 1;
			newfd = dup(fd);
			fd = newfd;
		} while (fd >= 0 && fd < global_env.iofd);

		for (i = 0; i < NOFILE; i++)
			if (map[i])
				close(i);

		if (fd < 0)
			err("too many files open in shell");
	}

	return fd;
}

static int openpipe(int *pv)
{
	int i;

	i = pipe(pv);
	if (i < 0)
		err("can't create pipe - try again");
	return i;
}

static void closepipe(int *pv)
{
	if (pv != NULL) {
		close(pv[0]);
		close(pv[1]);
	}
}


/* -------- here.c -------- */

/*
 * here documents
 */

static void markhere(char *s, struct ioword *iop)
{
	struct here *h, *lh;

	DBGPRINTF7(("MARKHERE: enter, s=%p\n", s));

	h = get_space(sizeof(struct here));
	if (h == NULL)
		return;

	h->h_tag = evalstr(s, DOSUB);
	if (h->h_tag == 0)
		return;

	h->h_iop = iop;
	iop->io_name = 0;
	h->h_next = NULL;
	if (inhere == 0)
		inhere = h;
	else {
		for (lh = inhere; lh != NULL; lh = lh->h_next) {
			if (lh->h_next == 0) {
				lh->h_next = h;
				break;
			}
		}
	}
	iop->io_flag |= IOHERE | IOXHERE;
	for (s = h->h_tag; *s; s++) {
		if (*s & QUOTE) {
			iop->io_flag &= ~IOXHERE;
			*s &= ~QUOTE;
		}
	}
	h->h_dosub = ((iop->io_flag & IOXHERE) ? '\0' : '\'');
}

static void gethere(void)
{
	struct here *h, *hp;

	DBGPRINTF7(("GETHERE: enter...\n"));

	/* Scan here files first leaving inhere list in place */
	for (hp = h = inhere; h != NULL; hp = h, h = h->h_next)
		readhere(&h->h_iop->io_name, h->h_tag, h->h_dosub /* NUL or ' */);

	/* Make inhere list active - keep list intact for scraphere */
	if (hp != NULL) {
		hp->h_next = acthere;
		acthere = inhere;
		inhere = NULL;
	}
}

static void readhere(char **name, char *s, int ec)
{
	int tf;
	char tname[30] = ".msh_XXXXXX";
	int c;
	jmp_buf ev;
	char myline[LINELIM + 1];
	char *thenext;

	DBGPRINTF7(("READHERE: enter, name=%p, s=%p\n", name, s));

	tf = mkstemp(tname);
	if (tf < 0)
		return;

	*name = strsave(tname, areanum);
	errpt = ev;
	if (newenv(setjmp(errpt)) != 0)
		unlink(tname);
	else {
		pushio(global_env.iop->argp, (int (*)(struct ioarg *)) global_env.iop->iofn);
		global_env.iobase = global_env.iop;
		for (;;) {
			if (interactive && global_env.iop <= iostack) {
#if ENABLE_FEATURE_EDITING
				current_prompt = cprompt->value;
#else
				prs(cprompt->value);
#endif
			}
			thenext = myline;
			while ((c = my_getc(ec)) != '\n' && c) {
				if (ec == '\'')
					c &= ~QUOTE;
				if (thenext >= &myline[LINELIM]) {
					c = 0;
					break;
				}
				*thenext++ = c;
			}
			*thenext = 0;
			if (strcmp(s, myline) == 0 || c == 0)
				break;
			*thenext++ = '\n';
			write(tf, myline, (int) (thenext - myline));
		}
		if (c == 0) {
			prs("here document `");
			prs(s);
			err("' unclosed");
		}
		quitenv();
	}
	close(tf);
}

/*
 * open here temp file.
 * if unquoted here, expand here temp file into second temp file.
 */
static int herein(char *hname, int xdoll)
{
	int hf;
	int tf;

#if __GNUC__
	/* Avoid longjmp clobbering */
	(void) &tf;
#endif
	if (hname == NULL)
		return -1;

	DBGPRINTF7(("HEREIN: hname is %s, xdoll=%d\n", hname, xdoll));

	hf = open(hname, O_RDONLY);
	if (hf < 0)
		return -1;

	if (xdoll) {
		char c;
		char tname[30] = ".msh_XXXXXX";
		jmp_buf ev;

		tf = mkstemp(tname);
		if (tf < 0)
			return -1;
		errpt = ev;
		if (newenv(setjmp(errpt)) == 0) {
			PUSHIO(afile, hf, herechar);
			setbase(global_env.iop);
			while ((c = subgetc(0, 0)) != 0) {
				c &= ~QUOTE;
				write(tf, &c, sizeof c);
			}
			quitenv();
		} else
			unlink(tname);
		close(tf);
		tf = open(tname, O_RDONLY);
		unlink(tname);
		return tf;
	}
	return hf;
}

static void scraphere(void)
{
	struct here *h;

	DBGPRINTF7(("SCRAPHERE: enter...\n"));

	for (h = inhere; h != NULL; h = h->h_next) {
		if (h->h_iop && h->h_iop->io_name)
			unlink(h->h_iop->io_name);
	}
	inhere = NULL;
}

/* unlink here temp files before a freearea(area) */
static void freehere(int area)
{
	struct here *h, *hl;

	DBGPRINTF6(("FREEHERE: enter, area=%d\n", area));

	hl = NULL;
	for (h = acthere; h != NULL; h = h->h_next) {
		if (getarea((char *) h) >= area) {
			if (h->h_iop->io_name != NULL)
				unlink(h->h_iop->io_name);
			if (hl == NULL)
				acthere = h->h_next;
			else
				hl->h_next = h->h_next;
		} else {
			hl = h;
		}
	}
}


/* -------- sh.c -------- */
/*
 * shell
 */

int msh_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int msh_main(int argc, char **argv)
{
	int f;
	char *s;
	int cflag;
	char *name, **ap;
	int (*iof) (struct ioarg *);

	INIT_G();

	sharedbuf.id = AFID_NOBUF;
	mainbuf.id = AFID_NOBUF;
	elinep = line + sizeof(line) - 5;

#if ENABLE_FEATURE_EDITING
	line_input_state = new_line_input_t(FOR_SHELL);
#endif

	DBGPRINTF(("MSH_MAIN: argc %d, environ %p\n", argc, environ));

	initarea();
	ap = environ;
	if (ap != NULL) {
		while (*ap)
			assign(*ap++, !COPYV);
		for (ap = environ; *ap;)
			export(lookup(*ap++));
	}
	closeall();
	areanum = 1;

	shell = lookup("SHELL");
	if (shell->value == null)
		setval(shell, (char *)DEFAULT_SHELL);
	export(shell);

	homedir = lookup("HOME");
	if (homedir->value == null)
		setval(homedir, "/");
	export(homedir);

	setval(lookup("$"), putn(getpid()));

	path = lookup("PATH");
	if (path->value == null) {
		/* Can be merged with same string elsewhere in bbox */
		if (geteuid() == 0)
			setval(path, bb_default_root_path);
		else
			setval(path, bb_default_path);
	}
	export(path);

	ifs = lookup("IFS");
	if (ifs->value == null)
		setval(ifs, " \t\n");

#ifdef MSHDEBUG
	mshdbg_var = lookup("MSHDEBUG");
	if (mshdbg_var->value == null)
		setval(mshdbg_var, "0");
#endif

	prompt = lookup("PS1");
#if ENABLE_FEATURE_EDITING_FANCY_PROMPT
	if (prompt->value == null)
#endif
		setval(prompt, DEFAULT_USER_PROMPT);
	if (geteuid() == 0) {
		setval(prompt, DEFAULT_ROOT_PROMPT);
		prompt->status &= ~EXPORT;
	}
	cprompt = lookup("PS2");
#if ENABLE_FEATURE_EDITING_FANCY_PROMPT
	if (cprompt->value == null)
#endif
		setval(cprompt, "> ");

	iof = filechar;
	cflag = 0;
	name = *argv++;
	if (--argc >= 1) {
		if (argv[0][0] == '-' && argv[0][1] != '\0') {
			for (s = argv[0] + 1; *s; s++)
				switch (*s) {
				case 'c':
					prompt->status &= ~EXPORT;
					cprompt->status &= ~EXPORT;
					setval(prompt, "");
					setval(cprompt, "");
					cflag = 1;
					if (--argc > 0)
						PUSHIO(aword, *++argv, iof = nlchar);
					break;

				case 'q':
					qflag = SIG_DFL;
					break;

				case 's':
					/* standard input */
					break;

				case 't':
					prompt->status &= ~EXPORT;
					setval(prompt, "");
					iof = linechar;
					break;

				case 'i':
					interactive = 1;
				default:
					if (*s >= 'a' && *s <= 'z')
						FLAG[(int) *s]++;
				}
		} else {
			argv--;
			argc++;
		}

		if (iof == filechar && --argc > 0) {
			setval(prompt, "");
			setval(cprompt, "");
			prompt->status &= ~EXPORT;
			cprompt->status &= ~EXPORT;

/* Shell is non-interactive, activate printf-based debug */
#ifdef MSHDEBUG
			mshdbg = mshdbg_var->value[0] - '0';
			if (mshdbg < 0)
				mshdbg = 0;
#endif
			DBGPRINTF(("MSH_MAIN: calling newfile()\n"));

			name = *++argv;
			if (newfile(name))
				exit(EXIT_FAILURE);  /* Exit on error */
		}
	}

	setdash();

	/* This won't be true if PUSHIO has been called, say from newfile() above */
	if (global_env.iop < iostack) {
		PUSHIO(afile, 0, iof);
		if (isatty(0) && isatty(1) && !cflag) {
			interactive = 1;
#if !ENABLE_FEATURE_SH_EXTRA_QUIET
#ifdef MSHDEBUG
			printf("\n\n%s built-in shell (msh with debug)\n", bb_banner);
#else
			printf("\n\n%s built-in shell (msh)\n", bb_banner);
#endif
			printf("Enter 'help' for a list of built-in commands.\n\n");
#endif
		}
	}

	signal(SIGQUIT, qflag);
	if (name && name[0] == '-') {
		interactive = 1;
		f = open(".profile", O_RDONLY);
		if (f >= 0)
			next(remap(f));
		f = open("/etc/profile", O_RDONLY);
		if (f >= 0)
			next(remap(f));
	}
	if (interactive)
		signal(SIGTERM, sig);

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);

/* Handle "msh SCRIPT VAR=val params..." */
/* Disabled: bash does not do it! */
#if 0
	argv++;
	/* skip leading args of the form VAR=val */
	while (*argv && assign(*argv, !COPYV)) {
		argc--;
		argv++;
	}
	argv--;
#endif
	dolv = argv;
	dolc = argc;
	dolv[0] = name;

	setval(lookup("#"), putn((--dolc < 0) ? (dolc = 0) : dolc));

	DBGPRINTF(("MSH_MAIN: begin FOR loop, interactive %d, global_env.iop %p, iostack %p\n", interactive, global_env.iop, iostack));

	for (;;) {
		if (interactive && global_env.iop <= iostack) {
#if ENABLE_FEATURE_EDITING
			current_prompt = prompt->value;
#else
			prs(prompt->value);
#endif
		}
		onecommand();
		/* Ensure that getenv("PATH") stays current */
		setenv("PATH", path->value, 1);
	}

	DBGPRINTF(("MSH_MAIN: returning.\n"));
}


/*
 * Copyright (c) 1987,1997, Prentice Hall
 * All rights reserved.
 *
 * Redistribution and use of the MINIX operating system in source and
 * binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * Neither the name of Prentice Hall nor the names of the software
 * authors or contributors may be used to endorse or promote
 * products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS, AUTHORS, AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL PRENTICE HALL OR ANY AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
