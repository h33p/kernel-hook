#include "hook.h"
#include <linux/kvm_types.h>
#include <linux/kvm_host.h>
#include <asm/kvm_host.h>

MODULE_DESCRIPTION("KVM page fault hook.");
MODULE_AUTHOR("Heep");
MODULE_LICENSE("GPL");

static int kvmhook_init(void);
static void kvmhook_fini(void);

module_init(kvmhook_init);
module_exit(kvmhook_fini);

#define printkvm(format, ...) printk(KBUILD_MODNAME": "format, ##__VA_ARGS__)

#pragma GCC optimize("-fno-optimize-sibling-calls")

static int called_count = 0;
static struct kvm_vcpu *called_vcpu = NULL;
static unsigned long prev_cr3 = 0;

DEFINE_STATIC_HOOK(int, kvm_mmu_page_fault, struct kvm_vcpu *vcpu, gva_t cr2, u64 error_code,
							  void *insn, int insn_len)
{
	DEFINE_ORIGINAL(kvm_mmu_page_fault);
	unsigned long cr3 = vcpu->arch.cr3;

	if (called_count < 20 && called_vcpu != vcpu && cr3 != prev_cr3) {
		printkvm("page fault on vcpu %p cr2 %lx cr3 %lx err %llx insn %p len %d\n", vcpu, cr2, cr3, error_code, insn, insn_len);
		called_count++;
		called_vcpu = vcpu;
		prev_cr3 = cr3;
	}

	return orig_fn(vcpu, cr2, error_code, insn, insn_len);
}

static int error_quit(const char *msg)
{
	printkvm("%s\n", msg);
	return -EFAULT;
}

static const fthinit_t hook_list[] = {
	HLIST_ENTRY(kvm_mmu_page_fault)
};

static int kvmhook_init(void)
{
	int ret;

	printkvm("initializing...\n");

	ret = start_hook_list(hook_list, ARRAY_SIZE(hook_list));

	if (ret == -1)
		return error_quit("Last error: Failed to lookup kvm_mmu_page_fault!");
	else if (ret == 1)
		return error_quit("Last error: Failed to call ftrace_set_filter_ip!");
	else if (ret == 2)
		return error_quit("Last error: Failed to call ftrace_set_filter_ip!");

	printkvm("KVMHook initialized!\n");

	return 0;
}

static void kvmhook_fini(void)
{
	int ret;
	printkvm("unloading...\n");

	ret = end_hook_list(hook_list, ARRAY_SIZE(hook_list));

	if (ret) {
		error_quit("Failed to unregister the ftrace function");
		return;
	}

	printkvm("KVMHook unloaded!\n");
}
