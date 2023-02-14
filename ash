# 0 "ash.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "ash.c"
# 1 "ash.h" 1


# 1 "pandos_types.h" 1
# 10 "pandos_types.h"
# 1 "/usr/include/umps3/umps/types.h" 1 3 4
# 32 "/usr/include/umps3/umps/types.h" 3 4

# 32 "/usr/include/umps3/umps/types.h" 3 4
typedef struct dtpreg {
 unsigned int status;
 unsigned int command;
 unsigned int data0;
 unsigned int data1;
} dtpreg_t;


typedef struct termreg {
 unsigned int recv_status;
 unsigned int recv_command;
 unsigned int transm_status;
 unsigned int transm_command;
} termreg_t;

typedef union devreg {
 dtpreg_t dtp;
 termreg_t term;
} devreg_t;


typedef struct devregarea {
 unsigned int rambase;
 unsigned int ramsize;
 unsigned int execbase;
 unsigned int execsize;
 unsigned int bootbase;
 unsigned int bootsize;
 unsigned int todhi;
 unsigned int todlo;
 unsigned int intervaltimer;
 unsigned int timescale;
 unsigned int tlb_floor_addr;
 unsigned int inst_dev[5];
 unsigned int interrupt_dev[5];
 devreg_t devreg[5][8];
} devregarea_t;


typedef struct passupvector {
 unsigned int tlb_refill_handler;
 unsigned int tlb_refill_stackPtr;
 unsigned int exception_handler;
 unsigned int exception_stackPtr;
} passupvector_t;




typedef struct state {
 unsigned int entry_hi;
 unsigned int cause;
 unsigned int status;
 unsigned int pc_epc;
 unsigned int gpr[29];
 unsigned int hi;
 unsigned int lo;
} state_t;
# 127 "/usr/include/umps3/umps/types.h" 3 4
typedef struct packet {
 unsigned char dest[6];
 unsigned char src[6];
 unsigned char proto[2];
 unsigned char data[1500];
 unsigned char dummy[2];
} packet_t;
# 11 "pandos_types.h" 2
# 1 "pandos_const.h" 1
# 10 "pandos_const.h"
# 1 "/usr/include/umps3/umps/const.h" 1 3 4
# 11 "pandos_const.h" 2
# 12 "pandos_types.h" 2
# 1 "list.h" 1




# 1 "container_of.h" 1
# 6 "list.h" 2
# 1 "types.h" 1






# 6 "types.h"
typedef int bool;
# 16 "types.h"
struct list_head {
 struct list_head *next, *prev;
};


typedef unsigned int u32;

struct hlist_head {
 struct hlist_node *first;
};

struct hlist_node {
 struct hlist_node *next, **pprev;
};
# 7 "list.h" 2
# 30 "list.h"
static inline void INIT_LIST_HEAD(struct list_head *list)
{
 list->next = list;
 list->prev = list;
}







static inline bool __list_add_valid(struct list_head *new,
    struct list_head *prev,
    struct list_head *next)
{
 return 1;
}
static inline bool __list_del_entry_valid(struct list_head *entry)
{
 return 1;
}
# 60 "list.h"
static inline void __list_add(struct list_head *new,
         struct list_head *prev,
         struct list_head *next)
{
 if (!__list_add_valid(new, prev, next))
  return;

 next->prev = new;
 new->next = next;
 new->prev = prev;
 prev->next = new;
}
# 81 "list.h"
static inline void list_add(struct list_head *new, struct list_head *head)
{
 __list_add(new, head, head->next);
}
# 95 "list.h"
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
 __list_add(new, head->prev, head);
}
# 109 "list.h"
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
 next->prev = prev;
 prev->next = next;
}
# 123 "list.h"
static inline void __list_del_clearprev(struct list_head *entry)
{
 __list_del(entry->prev, entry->next);
 entry->prev = ((void *)0);
}

static inline void __list_del_entry(struct list_head *entry)
{
 if (!__list_del_entry_valid(entry))
  return;

 __list_del(entry->prev, entry->next);
}







static inline void list_del(struct list_head *entry)
{
 __list_del_entry(entry);
 entry->next = ((void *)0);
 entry->prev = ((void *)0);
}
# 157 "list.h"
static inline void list_replace(struct list_head *old,
    struct list_head *new)
{
 new->next = old->next;
 new->next->prev = new;
 new->prev = old->prev;
 new->prev->next = new;
}
# 173 "list.h"
static inline void list_replace_init(struct list_head *old,
         struct list_head *new)
{
 list_replace(old, new);
 INIT_LIST_HEAD(old);
}






static inline void list_swap(struct list_head *entry1,
        struct list_head *entry2)
{
 struct list_head *pos = entry2->prev;

 list_del(entry2);
 list_replace(entry1, entry2);
 if (pos == entry1)
  pos = entry2;
 list_add(entry1, pos);
}





static inline void list_del_init(struct list_head *entry)
{
 __list_del_entry(entry);
 INIT_LIST_HEAD(entry);
}






static inline void list_move(struct list_head *list, struct list_head *head)
{
 __list_del_entry(list);
 list_add(list, head);
}






static inline void list_move_tail(struct list_head *list,
      struct list_head *head)
{
 __list_del_entry(list);
 list_add_tail(list, head);
}
# 239 "list.h"
static inline void list_bulk_move_tail(struct list_head *head,
           struct list_head *first,
           struct list_head *last)
{
 first->prev->next = last->next;
 last->next->prev = first->prev;

 head->prev->next = first;
 first->prev = head->prev;

 last->next = head;
 head->prev = last;
}






static inline int list_is_first(const struct list_head *list, const struct list_head *head)
{
 return list->prev == head;
}






static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
 return list->next == head;
}






static inline int list_is_head(const struct list_head *list, const struct list_head *head)
{
 return list == head;
}





static inline int list_empty(const struct list_head *head)
{
 return head->next == head;
}





static inline void list_rotate_left(struct list_head *head)
{
 struct list_head *first;

 if (!list_empty(head)) {
  first = head->next;
  list_move_tail(first, head);
 }
}
# 313 "list.h"
static inline void list_rotate_to_front(struct list_head *list,
     struct list_head *head)
{





 list_move_tail(head, list);
}





static inline int list_is_singular(const struct list_head *head)
{
 return !list_empty(head) && (head->next == head->prev);
}

static inline void __list_cut_position(struct list_head *list,
  struct list_head *head, struct list_head *entry)
{
 struct list_head *new_first = entry->next;
 list->next = head->next;
 list->next->prev = list;
 list->prev = entry;
 entry->next = list;
 head->next = new_first;
 new_first->prev = head;
}
# 359 "list.h"
static inline void list_cut_position(struct list_head *list,
  struct list_head *head, struct list_head *entry)
{
 if (list_empty(head))
  return;
 if (list_is_singular(head) && !list_is_head(entry, head) && (entry != head->next))
  return;
 if (list_is_head(entry, head))
  INIT_LIST_HEAD(list);
 else
  __list_cut_position(list, head, entry);
}
# 386 "list.h"
static inline void list_cut_before(struct list_head *list,
       struct list_head *head,
       struct list_head *entry)
{
 if (head->next == entry) {
  INIT_LIST_HEAD(list);
  return;
 }
 list->next = head->next;
 list->next->prev = list;
 list->prev = entry->prev;
 list->prev->next = list;
 head->next = entry;
 entry->prev = head;
}

static inline void __list_splice(const struct list_head *list,
     struct list_head *prev,
     struct list_head *next)
{
 struct list_head *first = list->next;
 struct list_head *last = list->prev;

 first->prev = prev;
 prev->next = first;

 last->next = next;
 next->prev = last;
}






static inline void list_splice(const struct list_head *list,
    struct list_head *head)
{
 if (!list_empty(list))
  __list_splice(list, head, head->next);
}






static inline void list_splice_tail(struct list_head *list,
    struct list_head *head)
{
 if (!list_empty(list))
  __list_splice(list, head->prev, head);
}
# 447 "list.h"
static inline void list_splice_init(struct list_head *list,
        struct list_head *head)
{
 if (!list_empty(list)) {
  __list_splice(list, head, head->next);
  INIT_LIST_HEAD(list);
 }
}
# 464 "list.h"
static inline void list_splice_tail_init(struct list_head *list,
      struct list_head *head)
{
 if (!list_empty(list)) {
  __list_splice(list, head->prev, head);
  INIT_LIST_HEAD(list);
 }
}
# 798 "list.h"
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
 h->next = ((void *)0);
 h->pprev = ((void *)0);
}
# 812 "list.h"
static inline int hlist_unhashed(const struct hlist_node *h)
{
 return !h->pprev;
}
# 825 "list.h"
static inline int hlist_unhashed_lockless(const struct hlist_node *h)
{
 return !h->pprev;
}





static inline int hlist_empty(const struct hlist_head *h)
{
 return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
 struct hlist_node *next = n->next;
 struct hlist_node **pprev = n->pprev;

 *pprev = next;
 if (next)
  next->pprev = pprev;
}
# 856 "list.h"
static inline void hlist_del(struct hlist_node *n)
{
 __hlist_del(n);
 n->next = ((void *)0);
 n->pprev = ((void *)0);
}







static inline void hlist_del_init(struct hlist_node *n)
{
 if (!hlist_unhashed(n)) {
  __hlist_del(n);
  INIT_HLIST_NODE(n);
 }
}
# 885 "list.h"
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
 struct hlist_node *first = h->first;
 n->next = first;
 if (first)
  first->pprev = &n->next;
 h->first = n;
 n->pprev = &h->first;
}






static inline void hlist_add_before(struct hlist_node *n,
        struct hlist_node *next)
{
 n->pprev = next->pprev;
 n->next = next;
 next->pprev = &n->next;
 *(n->pprev) = n;
}






static inline void hlist_add_behind(struct hlist_node *n,
        struct hlist_node *prev)
{
 n->next = prev->next;
 prev->next = n;
 n->pprev = &prev->next;

 if (n->next)
  n->next->pprev = &n->next;
}
# 933 "list.h"
static inline void hlist_add_fake(struct hlist_node *n)
{
 n->pprev = &n->next;
}





static inline bool hlist_fake(struct hlist_node *h)
{
 return h->pprev == &h->next;
}
# 955 "list.h"
static inline bool
hlist_is_singular_node(struct hlist_node *n, struct hlist_head *h)
{
 return !n->next && n->pprev == &h->first;
}
# 969 "list.h"
static inline void hlist_move_list(struct hlist_head *old,
       struct hlist_head *new)
{
 new->first = old->first;
 if (new->first)
  new->first->pprev = &new->first;
 old->first = ((void *)0);
}
# 13 "pandos_types.h" 2







typedef signed int cpu_t;
typedef unsigned int memaddr;


typedef struct nsd_t {

    int n_type;


    struct list_head n_link;
} nsd_t, *nsd_PTR;


typedef struct pcb_t {

    struct list_head p_list;


    struct pcb_t *p_parent;
    struct list_head p_child;
    struct list_head p_sib;


    state_t p_s;
    cpu_t p_time;


    int *p_semAdd;


    nsd_t *namespaces[(0 + 1)];
} pcb_t, *pcb_PTR;



typedef struct semd_t {

    int *s_key;

    struct list_head s_procq;


    struct hlist_node s_link;

    struct list_head s_freelink;
} semd_t, *semd_PTR;
# 4 "ash.h" 2


# 1 "hashtable.h" 1
# 11 "hashtable.h"
# 1 "hash.h" 1
# 52 "hash.h"
static inline u32 __hash_32_generic(u32 val)
{
 return val * 0x61C88647;
}

static inline u32 hash_32(u32 val, unsigned int bits)
{

 return __hash_32_generic(val) >> (32 - bits);
}

static inline u32 hash_ptr(const void *ptr, unsigned int bits)
{
 return hash_32((unsigned long)ptr, bits);
}


static inline u32 hash32_ptr(const void *ptr)
{
 unsigned long val = (unsigned long)ptr;

 return (u32)val;
}
# 12 "hashtable.h" 2
# 1 "log2.h" 1
# 13 "hashtable.h" 2
# 1 "/usr/include/stdio.h" 1 3 4
# 27 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
# 33 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 392 "/usr/include/features.h" 3 4
# 1 "/usr/include/features-time64.h" 1 3 4
# 20 "/usr/include/features-time64.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 21 "/usr/include/features-time64.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 1 3 4
# 19 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 20 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 2 3 4
# 22 "/usr/include/features-time64.h" 2 3 4
# 393 "/usr/include/features.h" 2 3 4
# 486 "/usr/include/features.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 1 3 4
# 559 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 560 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/long-double.h" 1 3 4
# 561 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 2 3 4
# 487 "/usr/include/features.h" 2 3 4
# 510 "/usr/include/features.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 1 3 4
# 10 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/gnu/stubs-64.h" 1 3 4
# 11 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 2 3 4
# 511 "/usr/include/features.h" 2 3 4
# 34 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 2 3 4
# 28 "/usr/include/stdio.h" 2 3 4





# 1 "/usr/lib/gcc/x86_64-linux-gnu/11/include/stddef.h" 1 3 4
# 209 "/usr/lib/gcc/x86_64-linux-gnu/11/include/stddef.h" 3 4

# 209 "/usr/lib/gcc/x86_64-linux-gnu/11/include/stddef.h" 3 4
typedef long unsigned int size_t;
# 34 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/lib/gcc/x86_64-linux-gnu/11/include/stdarg.h" 1 3 4
# 40 "/usr/lib/gcc/x86_64-linux-gnu/11/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 37 "/usr/include/stdio.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/types.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 1 3 4
# 19 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 20 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 2 3 4
# 29 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;






typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;



typedef long int __quad_t;
typedef unsigned long int __u_quad_t;







typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
# 141 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/typesizes.h" 1 3 4
# 142 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/time64.h" 1 3 4
# 143 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;




typedef int __sig_atomic_t;
# 39 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/__fpos_t.h" 1 3 4




# 1 "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h" 1 3 4
# 13 "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h" 3 4
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
# 6 "/usr/include/x86_64-linux-gnu/bits/types/__fpos_t.h" 2 3 4




typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
# 40 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/__fpos64_t.h" 1 3 4
# 10 "/usr/include/x86_64-linux-gnu/bits/types/__fpos64_t.h" 3 4
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
# 41 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/__FILE.h" 1 3 4



struct _IO_FILE;
typedef struct _IO_FILE __FILE;
# 42 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/FILE.h" 1 3 4



struct _IO_FILE;


typedef struct _IO_FILE FILE;
# 43 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h" 1 3 4
# 35 "/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h" 3 4
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;




typedef void _IO_lock_t;





struct _IO_FILE
{
  int _flags;


  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;


  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;
  int _flags2;
  __off_t _old_offset;


  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];

  _IO_lock_t *_lock;







  __off64_t _offset;

  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  size_t __pad5;
  int _mode;

  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];
};
# 44 "/usr/include/stdio.h" 2 3 4
# 52 "/usr/include/stdio.h" 3 4
typedef __gnuc_va_list va_list;
# 63 "/usr/include/stdio.h" 3 4
typedef __off_t off_t;
# 77 "/usr/include/stdio.h" 3 4
typedef __ssize_t ssize_t;






typedef __fpos_t fpos_t;
# 133 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stdio_lim.h" 1 3 4
# 134 "/usr/include/stdio.h" 2 3 4
# 143 "/usr/include/stdio.h" 3 4
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;






extern int remove (const char *__filename) __attribute__ ((__nothrow__ , __leaf__));

extern int rename (const char *__old, const char *__new) __attribute__ ((__nothrow__ , __leaf__));



extern int renameat (int __oldfd, const char *__old, int __newfd,
       const char *__new) __attribute__ ((__nothrow__ , __leaf__));
# 178 "/usr/include/stdio.h" 3 4
extern int fclose (FILE *__stream);
# 188 "/usr/include/stdio.h" 3 4
extern FILE *tmpfile (void)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
# 205 "/usr/include/stdio.h" 3 4
extern char *tmpnam (char[20]) __attribute__ ((__nothrow__ , __leaf__)) ;




extern char *tmpnam_r (char __s[20]) __attribute__ ((__nothrow__ , __leaf__)) ;
# 222 "/usr/include/stdio.h" 3 4
extern char *tempnam (const char *__dir, const char *__pfx)
   __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (__builtin_free, 1)));






extern int fflush (FILE *__stream);
# 239 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked (FILE *__stream);
# 258 "/usr/include/stdio.h" 3 4
extern FILE *fopen (const char *__restrict __filename,
      const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *freopen (const char *__restrict __filename,
        const char *__restrict __modes,
        FILE *__restrict __stream) ;
# 293 "/usr/include/stdio.h" 3 4
extern FILE *fdopen (int __fd, const char *__modes) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
# 308 "/usr/include/stdio.h" 3 4
extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
# 328 "/usr/include/stdio.h" 3 4
extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__ , __leaf__));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__ , __leaf__));




extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__ , __leaf__));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));







extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...);




extern int printf (const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg);




extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));



extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));
# 403 "/usr/include/stdio.h" 3 4
extern int vdprintf (int __fd, const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));







extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) ;




extern int scanf (const char *__restrict __format, ...) ;

extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__));





# 1 "/usr/include/x86_64-linux-gnu/bits/floatn.h" 1 3 4
# 119 "/usr/include/x86_64-linux-gnu/bits/floatn.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/floatn-common.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/floatn-common.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/long-double.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/floatn-common.h" 2 3 4
# 120 "/usr/include/x86_64-linux-gnu/bits/floatn.h" 2 3 4
# 431 "/usr/include/stdio.h" 2 3 4



extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc99_fscanf")

                               ;
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc99_scanf")
                              ;
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) __asm__ ("" "__isoc99_sscanf") __attribute__ ((__nothrow__ , __leaf__))

                      ;
# 459 "/usr/include/stdio.h" 3 4
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) ;





extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;


extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__format__ (__scanf__, 2, 0)));





extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0))) ;
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vscanf")

     __attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vsscanf") __attribute__ ((__nothrow__ , __leaf__))



     __attribute__ ((__format__ (__scanf__, 2, 0)));
# 513 "/usr/include/stdio.h" 3 4
extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);






extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
# 538 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked (FILE *__stream);
# 549 "/usr/include/stdio.h" 3 4
extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);
# 565 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);







extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __attribute__ ((__access__ (__write_only__, 1, 2)));
# 632 "/usr/include/stdio.h" 3 4
extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) ;
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) ;







extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) ;







extern int fputs (const char *__restrict __s, FILE *__restrict __stream);





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s);
# 702 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream);







extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);
# 736 "/usr/include/stdio.h" 3 4
extern int fseeko (FILE *__stream, __off_t __off, int __whence);




extern __off_t ftello (FILE *__stream) ;
# 760 "/usr/include/stdio.h" 3 4
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, const fpos_t *__pos);
# 786 "/usr/include/stdio.h" 3 4
extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;



extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;







extern void perror (const char *__s);




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
# 823 "/usr/include/stdio.h" 3 4
extern int pclose (FILE *__stream);





extern FILE *popen (const char *__command, const char *__modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (pclose, 1))) ;






extern char *ctermid (char *__s) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__access__ (__write_only__, 1)));
# 867 "/usr/include/stdio.h" 3 4
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
# 885 "/usr/include/stdio.h" 3 4
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);
# 902 "/usr/include/stdio.h" 3 4

# 14 "hashtable.h" 2
# 34 "hashtable.h"

# 34 "hashtable.h"
static inline void __hash_init(struct hlist_head *ht, unsigned int sz)
{
 unsigned int i;

 for (i = 0; i < sz; i++)
  ((&ht[i])->first = 
# 39 "hashtable.h" 3 4
 ((void *)0)
# 39 "hashtable.h"
 );
}
# 76 "hashtable.h"
static inline bool hash_hashed(struct hlist_node *node)
{
 return !hlist_unhashed(node);
}

static inline bool __hash_empty(struct hlist_head *ht, unsigned int sz)
{
 unsigned int i;

 for (i = 0; i < sz; i++)
  if (!hlist_empty(&ht[i]))
   return 0;

 return 1;
}
# 105 "hashtable.h"
static inline void hash_del(struct hlist_node *node)
{
 hlist_del_init(node);
}
# 7 "ash.h" 2
# 1 "pcb.h" 1
# 10 "pcb.h"
void initPcbs();
void freePcb(pcb_t *p);
pcb_t *allocPcb();



void mkEmptyProcQ(struct list_head *head);
int emptyProcQ(struct list_head *head);
void insertProcQ(struct list_head *head, pcb_t *p);
pcb_t *headProcQ(struct list_head *head);
pcb_t *removeProcQ(struct list_head *head);
pcb_t *outProcQ(struct list_head *head, pcb_t *p);



int emptyChild(pcb_t *p);
void insertChild(pcb_t *prnt, pcb_t *p);
pcb_t *removeChild(pcb_t *p);
pcb_t *outChild(pcb_t *p);
# 8 "ash.h" 2



int insertBlocked(int *semAdd, pcb_t *p);
int sem_empty();
int counter();
pcb_t *removeBlocked(int *semAdd);
pcb_t *outBlocked(pcb_t *p);
pcb_t *headBlocked(int *semAdd);
void initASH();
# 2 "ash.c" 2

semd_t semd_table[20];



semd_t *semdFree_h;

int i = 0;
# 20 "ash.c"
struct hlist_head semd_h[1 << (5)] = { [0 ... ((1 << (5)) - 1)] = { .first = 
# 20 "ash.c" 3 4
((void *)0) 
# 20 "ash.c"
} };






int insertBlocked(int *semAdd, pcb_t *p)
{
    if (semAdd != 
# 29 "ash.c" 3 4
                 ((void *)0) 
# 29 "ash.c"
                      && p != 
# 29 "ash.c" 3 4
                              ((void *)0)
# 29 "ash.c"
                                  )
    {

        semd_t *tmp;

        for (tmp = ({ typeof((&semd_h[hash_32((int)semAdd, ( __builtin_constant_p((sizeof(semd_h)/sizeof(*semd_h))) ? ( ((sizeof(semd_h)/sizeof(*semd_h))) < 2 ? 0 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 63) ? 63 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 62) ? 62 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 61) ? 61 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 60) ? 60 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 59) ? 59 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 58) ? 58 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 57) ? 57 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 56) ? 56 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 55) ? 55 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 54) ? 54 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 53) ? 53 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 52) ? 52 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 51) ? 51 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 50) ? 50 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 49) ? 49 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 48) ? 48 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 47) ? 47 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 46) ? 46 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 45) ? 45 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 44) ? 44 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 43) ? 43 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 42) ? 42 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 41) ? 41 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 40) ? 40 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 39) ? 39 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 38) ? 38 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 37) ? 37 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 36) ? 36 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 35) ? 35 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 34) ? 34 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 33) ? 33 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 32) ? 32 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 31) ? 31 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 30) ? 30 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 29) ? 29 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 28) ? 28 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 27) ? 27 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 26) ? 26 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 25) ? 25 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 24) ? 24 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 23) ? 23 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 22) ? 22 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 21) ? 21 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 20) ? 20 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 19) ? 19 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 18) ? 18 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 17) ? 17 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 16) ? 16 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 15) ? 15 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 14) ? 14 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 13) ? 13 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 12) ? 12 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 11) ? 11 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 10) ? 10 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 9) ? 9 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 8) ? 8 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 7) ? 7 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 6) ? 6 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 5) ? 5 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 4) ? 4 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 3) ? 3 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 2) ? 2 : 1) : -1))])->first) ____ptr = ((&semd_h[hash_32((int)semAdd, ( __builtin_constant_p((sizeof(semd_h)/sizeof(*semd_h))) ? ( ((sizeof(semd_h)/sizeof(*semd_h))) < 2 ? 0 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 63) ? 63 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 62) ? 62 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 61) ? 61 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 60) ? 60 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 59) ? 59 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 58) ? 58 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 57) ? 57 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 56) ? 56 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 55) ? 55 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 54) ? 54 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 53) ? 53 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 52) ? 52 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 51) ? 51 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 50) ? 50 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 49) ? 49 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 48) ? 48 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 47) ? 47 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 46) ? 46 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 45) ? 45 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 44) ? 44 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 43) ? 43 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 42) ? 42 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 41) ? 41 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 40) ? 40 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 39) ? 39 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 38) ? 38 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 37) ? 37 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 36) ? 36 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 35) ? 35 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 34) ? 34 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 33) ? 33 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 32) ? 32 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 31) ? 31 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 30) ? 30 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 29) ? 29 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 28) ? 28 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 27) ? 27 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 26) ? 26 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 25) ? 25 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 24) ? 24 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 23) ? 23 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 22) ? 22 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 21) ? 21 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 20) ? 20 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 19) ? 19 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 18) ? 18 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 17) ? 17 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 16) ? 16 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 15) ? 15 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 14) ? 14 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 13) ? 13 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 12) ? 12 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 11) ? 11 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 10) ? 10 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 9) ? 9 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 8) ? 8 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 7) ? 7 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 6) ? 6 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 5) ? 5 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 4) ? 4 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 3) ? 3 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 2) ? 2 : 1) : -1))])->first); ____ptr ? ({ void *__mptr = (void *)(____ptr); ((typeof(*(tmp)) *)(__mptr - __builtin_offsetof(typeof(*(tmp)), s_link))); }) : 
# 34 "ash.c" 3 4
       ((void *)0)
# 34 "ash.c"
       ; }); tmp; tmp = ({ typeof((tmp)->s_link.next) ____ptr = ((tmp)->s_link.next); ____ptr ? ({ void *__mptr = (void *)(____ptr); ((typeof(*(tmp)) *)(__mptr - __builtin_offsetof(typeof(*(tmp)), s_link))); }) : 
# 34 "ash.c" 3 4
       ((void *)0)
# 34 "ash.c"
       ; }))
        {

            if (tmp == 
# 37 "ash.c" 3 4
                      ((void *)0) 
# 37 "ash.c"
                           && semdFree_h != 
# 37 "ash.c" 3 4
                                            ((void *)0)
# 37 "ash.c"
                                                )
            {
                i++;

                tmp = semdFree_h;
                semdFree_h = ({ void *__mptr = (void *)(semdFree_h->s_freelink.next); ((semd_t *)(__mptr - __builtin_offsetof(semd_t, s_freelink))); });

                tmp->s_freelink.next = 
# 44 "ash.c" 3 4
                                      ((void *)0)
# 44 "ash.c"
                                          ;
                tmp->s_key = semAdd;

                mkEmptyProcQ(&(tmp->s_procq));
                p->p_semAdd = semAdd;
                insertProcQ(&(tmp->s_procq), p);

                hlist_add_head(&(tmp->s_link), &semd_h[hash_32((int)semAdd, ( __builtin_constant_p((sizeof(semd_h)/sizeof(*semd_h))) ? ( ((sizeof(semd_h)/sizeof(*semd_h))) < 2 ? 0 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 63) ? 63 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 62) ? 62 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 61) ? 61 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 60) ? 60 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 59) ? 59 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 58) ? 58 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 57) ? 57 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 56) ? 56 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 55) ? 55 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 54) ? 54 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 53) ? 53 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 52) ? 52 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 51) ? 51 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 50) ? 50 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 49) ? 49 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 48) ? 48 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 47) ? 47 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 46) ? 46 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 45) ? 45 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 44) ? 44 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 43) ? 43 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 42) ? 42 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 41) ? 41 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 40) ? 40 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 39) ? 39 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 38) ? 38 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 37) ? 37 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 36) ? 36 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 35) ? 35 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 34) ? 34 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 33) ? 33 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 32) ? 32 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 31) ? 31 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 30) ? 30 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 29) ? 29 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 28) ? 28 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 27) ? 27 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 26) ? 26 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 25) ? 25 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 24) ? 24 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 23) ? 23 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 22) ? 22 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 21) ? 21 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 20) ? 20 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 19) ? 19 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 18) ? 18 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 17) ? 17 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 16) ? 16 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 15) ? 15 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 14) ? 14 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 13) ? 13 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 12) ? 12 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 11) ? 11 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 10) ? 10 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 9) ? 9 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 8) ? 8 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 7) ? 7 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 6) ? 6 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 5) ? 5 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 4) ? 4 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 3) ? 3 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 2) ? 2 : 1) : -1))]);
                return 0;
            }

            else if (tmp == 
# 55 "ash.c" 3 4
                           ((void *)0) 
# 55 "ash.c"
                                && semdFree_h == 
# 55 "ash.c" 3 4
                                                 ((void *)0)
# 55 "ash.c"
                                                     )
                return 1;

            else
            {
                p->p_semAdd = semAdd;
                insertProcQ(&(tmp->s_procq), p);
                return 0;
            }
        }
    }
}

int counter() {
    return i;
}

int sem_empty()
{
    if (semdFree_h == 
# 74 "ash.c" 3 4
                     ((void *)0)
# 74 "ash.c"
                         )
        return 1;
    else
        return 0;
}







pcb_t *removeBlocked(int *semAdd)
{
    if (semAdd == 
# 88 "ash.c" 3 4
                 ((void *)0)
# 88 "ash.c"
                     )
        return 
# 89 "ash.c" 3 4
              ((void *)0)
# 89 "ash.c"
                  ;
    semd_t *tmp;

    for (tmp = ({ typeof((&semd_h[hash_32((int)semAdd, ( __builtin_constant_p((sizeof(semd_h)/sizeof(*semd_h))) ? ( ((sizeof(semd_h)/sizeof(*semd_h))) < 2 ? 0 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 63) ? 63 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 62) ? 62 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 61) ? 61 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 60) ? 60 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 59) ? 59 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 58) ? 58 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 57) ? 57 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 56) ? 56 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 55) ? 55 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 54) ? 54 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 53) ? 53 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 52) ? 52 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 51) ? 51 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 50) ? 50 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 49) ? 49 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 48) ? 48 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 47) ? 47 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 46) ? 46 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 45) ? 45 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 44) ? 44 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 43) ? 43 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 42) ? 42 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 41) ? 41 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 40) ? 40 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 39) ? 39 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 38) ? 38 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 37) ? 37 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 36) ? 36 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 35) ? 35 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 34) ? 34 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 33) ? 33 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 32) ? 32 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 31) ? 31 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 30) ? 30 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 29) ? 29 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 28) ? 28 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 27) ? 27 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 26) ? 26 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 25) ? 25 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 24) ? 24 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 23) ? 23 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 22) ? 22 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 21) ? 21 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 20) ? 20 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 19) ? 19 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 18) ? 18 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 17) ? 17 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 16) ? 16 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 15) ? 15 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 14) ? 14 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 13) ? 13 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 12) ? 12 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 11) ? 11 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 10) ? 10 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 9) ? 9 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 8) ? 8 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 7) ? 7 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 6) ? 6 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 5) ? 5 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 4) ? 4 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 3) ? 3 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 2) ? 2 : 1) : -1))])->first) ____ptr = ((&semd_h[hash_32((int)semAdd, ( __builtin_constant_p((sizeof(semd_h)/sizeof(*semd_h))) ? ( ((sizeof(semd_h)/sizeof(*semd_h))) < 2 ? 0 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 63) ? 63 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 62) ? 62 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 61) ? 61 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 60) ? 60 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 59) ? 59 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 58) ? 58 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 57) ? 57 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 56) ? 56 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 55) ? 55 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 54) ? 54 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 53) ? 53 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 52) ? 52 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 51) ? 51 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 50) ? 50 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 49) ? 49 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 48) ? 48 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 47) ? 47 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 46) ? 46 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 45) ? 45 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 44) ? 44 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 43) ? 43 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 42) ? 42 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 41) ? 41 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 40) ? 40 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 39) ? 39 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 38) ? 38 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 37) ? 37 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 36) ? 36 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 35) ? 35 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 34) ? 34 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 33) ? 33 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 32) ? 32 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 31) ? 31 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 30) ? 30 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 29) ? 29 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 28) ? 28 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 27) ? 27 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 26) ? 26 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 25) ? 25 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 24) ? 24 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 23) ? 23 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 22) ? 22 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 21) ? 21 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 20) ? 20 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 19) ? 19 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 18) ? 18 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 17) ? 17 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 16) ? 16 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 15) ? 15 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 14) ? 14 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 13) ? 13 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 12) ? 12 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 11) ? 11 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 10) ? 10 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 9) ? 9 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 8) ? 8 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 7) ? 7 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 6) ? 6 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 5) ? 5 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 4) ? 4 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 3) ? 3 : ((sizeof(semd_h)/sizeof(*semd_h))) & (1ULL << 2) ? 2 : 1) : -1))])->first); ____ptr ? ({ void *__mptr = (void *)(____ptr); ((typeof(*(tmp)) *)(__mptr - __builtin_offsetof(typeof(*(tmp)), s_link))); }) : 
# 92 "ash.c" 3 4
   ((void *)0)
# 92 "ash.c"
   ; }); tmp; tmp = ({ typeof((tmp)->s_link.next) ____ptr = ((tmp)->s_link.next); ____ptr ? ({ void *__mptr = (void *)(____ptr); ((typeof(*(tmp)) *)(__mptr - __builtin_offsetof(typeof(*(tmp)), s_link))); }) : 
# 92 "ash.c" 3 4
   ((void *)0)
# 92 "ash.c"
   ; }))
    {


        if (tmp == 
# 96 "ash.c" 3 4
                  ((void *)0)
# 96 "ash.c"
                      )
            return 
# 97 "ash.c" 3 4
                  ((void *)0)
# 97 "ash.c"
                      ;

        else
        {

            pcb_t *p = removeProcQ(&(tmp->s_procq));
            if (list_empty(&(tmp->s_procq)))
            {

                hash_del(&(tmp->s_link));

                tmp->s_freelink.next = &(semdFree_h->s_freelink);
                semdFree_h = tmp;
            }
            return p;
        }
    }
}






pcb_t *outBlocked(pcb_t *p) {}



pcb_t *headBlocked(int *semAdd) {}



void initASH()
{
    semdFree_h = &semd_table[0];
    semdFree_h->s_freelink.next = 
# 132 "ash.c" 3 4
                                 ((void *)0)
# 132 "ash.c"
                                     ;
    semdFree_h->s_freelink.prev = 
# 133 "ash.c" 3 4
                                 ((void *)0)
# 133 "ash.c"
                                     ;
    semd_t *tmp = semdFree_h;
    for (int i = 1; i < 20; i++)
    {
        tmp->s_freelink.next = &(semd_table[i].s_freelink);

        tmp = ({ void *__mptr = (void *)(tmp->s_freelink.next); ((semd_t *)(__mptr - __builtin_offsetof(semd_t, s_freelink))); });
        tmp->s_freelink.prev = 
# 140 "ash.c" 3 4
                              ((void *)0)
# 140 "ash.c"
                                  ;
        tmp->s_freelink.next = 
# 141 "ash.c" 3 4
                              ((void *)0)
# 141 "ash.c"
                                  ;
    }
}
