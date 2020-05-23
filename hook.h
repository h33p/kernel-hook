#ifndef VMHOOK_HOOK_H
#define VMHOOK_HOOK_H

#include <linux/ftrace.h>

#define DEFINE_ORIGINAL_2(fname, hook_ptr)	\
	typeof(&fname) orig_fn = (typeof(&fname)) (hook_ptr)->original_function

#define DEFINE_ORIGINAL(fname) DEFINE_ORIGINAL_2(fname##_fn_hooked, &fname##_hook)


#define DEFINE_INTERCEPT_HOOK_2(addstart, fname) \
	addstart fthook_t fname##_hook; \
	addstart void notrace fname##_int_hooked(unsigned long ip, unsigned long parent_ip, struct ftrace_ops *ops, struct pt_regs *regs)

#define DEFINE_FUNCTION_HOOK_2(addstart, rettype, fname, ...) \
	static rettype fname##_fn_hooked (__VA_ARGS__); \
	DEFINE_INTERCEPT_HOOK_2(addstart, fname) \
	{ \
		if (!within_module(parent_ip, THIS_MODULE)) \
			regs->ip = (uintptr_t)fname##_fn_hooked; \
	} \
	__attribute__((no_caller_saved_registers)) static rettype fname##_fn_hooked (__VA_ARGS__)

#define DEFINE_FUNCTION_HOOK(rettype, fname, ...) DEFINE_FUNCTION_HOOK_2(, rettype, fname, __VA_ARGS__)
#define DEFINE_STATIC_FUNCTION_HOOK(rettype, fname, ...) DEFINE_FUNCTION_HOOK_2(static, rettype, fname, __VA_ARGS__)
#define DECLARE_FUNCTION_HOOK(rettype, fname, ...) DEFINE_INTERCEPT_HOOK_2(extern, fname)

#define DEFINE_INTERCEPT_HOOK(fname, ...) DEFINE_INTERCEPT_HOOK_2(, fname)
#define DEFINE_STATIC_INTERCEPT_HOOK(fname, ...) DEFINE_INTERCEPT_HOOK_2(static, fname)
#define DECLARE_INTERCEPT_HOOK(fname, ...) DEFINE_INTERCEPT_HOOK_2(extern, fname)

#define DEFINE_HOOK_GETTER(fname) uintptr_t fname##_ptr_getter (void)

#define HLIST_NAME_ENTRY(fname) { #fname, 0, (void *)fname##_int_hooked, &fname##_hook }
#define HLIST_GETTER_ENTRY(fname) { NULL, fname##_ptr_getter, (void *)fname##_int_hooked, &fname##_hook }

typedef void (*ftrace_fn)(unsigned long, unsigned long, struct ftrace_ops *, struct pt_regs *);

typedef struct fthook
{
	uintptr_t original_function;
	struct ftrace_ops ops;
	int active;
} fthook_t;

typedef struct fthinit
{
	const char *symbol_name;
	uintptr_t (*symbol_getter) (void);
	ftrace_fn hook_function;
	fthook_t *hook;
} fthinit_t;

extern int start_hook(fthook_t *hook, uintptr_t hooked_function, ftrace_fn hook_function);
extern int end_hook(fthook_t *hook);
extern int start_hook_list(const fthinit_t *hook_list, size_t size);
extern int end_hook_list(const fthinit_t *hook_list, size_t size);

#endif
