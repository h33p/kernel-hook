# Linux kernel hooking library

This project provides a simple method to intercept various kernel methods on the fly by using ftrace.

## Example

main.c includes a complete example hooking 2 kernel functions, implementing a bare bones RDTSC timer based VM detection evasion.

The most important parts are as follows:

A list of functions to hook:

```
static const fthinit_t hook_list[] = {
	HLIST_NAME_ENTRY(kvm_emulate_cpuid),
	HLIST_GETTER_ENTRY(kvm_vcpu_run)
};
```

HLIST\_NAME\_ENTRY retrieves the address of the function using kallsyms, while HLIST\_GETTER\_ENTRY requires the user to define a getter function that returns a uintptr\_t. It is done this way:

```
DEFINE_HOOK_GETTER(kvm_vcpu_run)
{
       return (uintptr_t)kvm_x86_ops->run;
}
```

To initialize the hook list call `start_hook_list` function. Also, `end_hook_list` function removes the hooks.

Every hook has to be defined using DEFINE\_FUNCTION\_HOOK and DEFINE\_INTERCEPT\_HOOK macros. For FUNCTION type, there is DEFINE\_STATIC\_FUNCTION\_HOOK macro if it is possible to define the function in the same file. Otherwise, use DEFINE\_FUNCTION\_HOOK and DECLARE\_FUNCTION\_HOOK. The syntax is all the same, like this:

```
DEFINE_STATIC_FUNCTION_HOOK(int, kvm_emulate_cpuid, struct kvm_vcpu *vcpu)
```

To access the original function use the DEFINE\_ORIGINAL macro inside the function.

DEFINE\_INTERCEPT\_HOOK macros work in a similar way, but you do not need to specify the function parameters, they are always as follows: `unsigned long ip, unsigned long parent_ip, struct ftrace_ops *ops, struct pt_regs *regs`. Example definition:

```
DEFINE_STATIC_INTERCEPT_HOOK(int, vmx_handle_exit)
```

## Troubleshooting

If the hook hangs the system, disable sibling and tail call optimizations by adding this at the top of each file containing the hooks:

```
#pragma GCC optimize("-fno-optimize-sibling-calls")
```
