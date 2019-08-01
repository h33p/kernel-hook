#include "hook.h"

static void notrace ftrace_thunk(unsigned long ip, unsigned long parent_ip,
								 struct ftrace_ops *ops, struct pt_regs *regs)
{
	fthook_t *hook = container_of(ops, fthook_t, ops);

	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (uintptr_t)hook->hook_function;
}

int start_hook(fthook_t *hook, uintptr_t hooked_function, void *hook_function)
{
	int ret = 0;

	hook->active = 0;
	hook->original_function = hooked_function;
	hook->hook_function = hook_function;
	hook->ops.func = ftrace_thunk;
	hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
		| FTRACE_OPS_FL_RECURSION_SAFE
		| FTRACE_OPS_FL_IPMODIFY;

	ret = ftrace_set_filter_ip(&hook->ops, hook->original_function, 0, 0);

	if (ret)
		return 1;

	ret = register_ftrace_function(&hook->ops);

	if (ret) {
		ftrace_set_filter_ip(&hook->ops, hook->original_function, 1, 0);
		return 2;
	}

	hook->active = 1;

	return 0;
}

int end_hook(fthook_t *hook)
{
	int ret = unregister_ftrace_function(&hook->ops);

	hook->active = 0;

	if (ret)
		return ret;

	ftrace_set_filter_ip(&hook->ops, hook->original_function, 1, 0);

	return 0;
}

int start_hook_list(const fthinit_t *hook_list, size_t size)
{
	size_t i;
	uintptr_t symaddr;
	int ret = 0, ret2 = 0;

	for (i = 0; i < size; i++) {
		symaddr = kallsyms_lookup_name(hook_list[i].symbol);

		if (!symaddr)
			ret = -1;

		ret2 = start_hook(hook_list[i].hook, symaddr, hook_list[i].hook_function);

		if (ret2)
			ret = ret2;
	}

	return ret;
}

int end_hook_list(const fthinit_t *hook_list, size_t size)
{
	size_t i;
	int ret = 0, ret2 = 0;

	for (i = 0; i < size; i++) {
		if (hook_list[i].hook->active)
			ret2 = end_hook(hook_list[i].hook);

		if (ret2)
			ret = ret2;
	}

	return ret;
}
