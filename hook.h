#ifndef VMHOOK_HOOK_H
#define VMHOOK_HOOK_H

#include <linux/ftrace.h>

#define DEFINE_ORIGINAL_2(fname, hook_ptr)	\
	typeof(&fname) orig_fn = (typeof(&fname)) (hook_ptr)->original_function

#define DEFINE_ORIGINAL(fname) DEFINE_ORIGINAL_2(fname##_hooked, &fname##_hook)

#define DEFINE_HOOK_2(addstart, rettype, fname, ...)	\
	addstart fthook_t fname##_hook;			\
	__attribute__((no_caller_saved_registers)) addstart rettype fname##_hooked (__VA_ARGS__)

#define DEFINE_HOOK(rettype, fname, ...) DEFINE_HOOK_2(, rettype, fname, __VA_ARGS__)
#define DEFINE_STATIC_HOOK(rettype, fname, ...) DEFINE_HOOK_2(static, rettype, fname, __VA_ARGS__)
#define DECLARE_HOOK(rettype, fname, ...) DEFINE_HOOK_2(extern, reettype, fname, __VA_ARGS__)

#define DEFINE_HOOK_GETTER(fname) uintptr_t fname##_ptr_getter (void)

#define HLIST_NAME_ENTRY(fname) { #fname, 0, (void *)fname##_hooked, &fname##_hook }
#define HLIST_GETTER_ENTRY(fname) { NULL, fname##_ptr_getter, (void *)fname##_hooked, &fname##_hook }

typedef struct fthook
{
	uintptr_t original_function;
	void *hook_function;
	struct ftrace_ops ops;
	int active;
} fthook_t;

typedef struct fthinit
{
	const char *symbol_name;
	uintptr_t (*symbol_getter) (void);
	void *hook_function;
	fthook_t *hook;
} fthinit_t;

extern int start_hook(fthook_t *hook, uintptr_t hooked_function, void *hook_function);
extern int end_hook(fthook_t *hook);
extern int start_hook_list(const fthinit_t *hook_list, size_t size);
extern int end_hook_list(const fthinit_t *hook_list, size_t size);

#endif
