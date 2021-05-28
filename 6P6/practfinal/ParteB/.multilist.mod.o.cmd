cmd_/home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.o := gcc -Wp,-MD,/home/kernel/LIN/6P6/practfinal/ParteB/.multilist.mod.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-linux-gnu/6/include -I/home/kernel/oreo-src/kernel/arch/x86/include -I./arch/x86/include/generated/uapi -I./arch/x86/include/generated  -I/home/kernel/oreo-src/kernel/include -I./include -I/home/kernel/oreo-src/kernel/arch/x86/include/uapi -I/home/kernel/oreo-src/kernel/include/uapi -I./include/generated/uapi -include /home/kernel/oreo-src/kernel/include/linux/kconfig.h -I/home/kernel/oreo-src/kernel//home/kernel/LIN/6P6/practfinal/ParteB -I/home/kernel/LIN/6P6/practfinal/ParteB -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fno-pic -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -fno-pic -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -maccumulate-outgoing-args -DCONFIG_X86_X32_ABI -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_SSSE3=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -DCONFIG_AS_AVX512=1 -DCONFIG_AS_SHA1_NI=1 -DCONFIG_AS_SHA256_NI=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -DRETPOLINE -fno-delete-null-pointer-checks -Wno-frame-address -O2 --param=allow-store-data-races=0 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=1024 -fstack-protector-strong -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-var-tracking-assignments -pg -mfentry -DCC_USING_FENTRY -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types  -DKBUILD_BASENAME='"multilist.mod"'  -DKBUILD_MODNAME='"multilist"' -DMODULE  -c -o /home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.o /home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.c

source_/home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.o := /home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.c

deps_/home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.o := \
    $(wildcard include/config/module/unload.h) \
  /home/kernel/oreo-src/kernel/include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/modules/tree/lookup.h) \
    $(wildcard include/config/livepatch.h) \
    $(wildcard include/config/cfi/clang.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/module/sig.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/debug/set/module/ronx.h) \
  /home/kernel/oreo-src/kernel/include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  /home/kernel/oreo-src/kernel/include/linux/types.h \
    $(wildcard include/config/have/uid16.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/kernel/oreo-src/kernel/include/uapi/linux/types.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/types.h \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/types.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/int-ll64.h \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/int-ll64.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/bitsperlong.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/bitsperlong.h \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/bitsperlong.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/posix_types.h \
  /home/kernel/oreo-src/kernel/include/linux/stddef.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/stddef.h \
  /home/kernel/oreo-src/kernel/include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/kasan.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  /home/kernel/oreo-src/kernel/include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
    $(wildcard include/config/gcov/kernel.h) \
    $(wildcard include/config/stack/validation.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/posix_types.h \
    $(wildcard include/config/x86/32.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/posix_types_64.h \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/posix_types.h \
  /home/kernel/oreo-src/kernel/include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
    $(wildcard include/config/page/poisoning/zero.h) \
  /home/kernel/oreo-src/kernel/include/uapi/linux/const.h \
  /home/kernel/oreo-src/kernel/include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/panic/timeout.h) \
  /usr/lib/gcc/x86_64-linux-gnu/6/include/stdarg.h \
  /home/kernel/oreo-src/kernel/include/linux/linkage.h \
  /home/kernel/oreo-src/kernel/include/linux/stringify.h \
  /home/kernel/oreo-src/kernel/include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/trim/unused/ksyms.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/linkage.h \
    $(wildcard include/config/x86/64.h) \
    $(wildcard include/config/x86/alignment/16.h) \
  /home/kernel/oreo-src/kernel/include/linux/bitops.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/bitops.h \
    $(wildcard include/config/x86/cmov.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/alternative.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/asm.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/rmwcc.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/barrier.h \
    $(wildcard include/config/x86/ppro/fence.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/nops.h \
    $(wildcard include/config/mk7.h) \
    $(wildcard include/config/x86/p6/nop.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/barrier.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/bitops/find.h \
    $(wildcard include/config/generic/find/first/bit.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/bitops/sched.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/arch_hweight.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/cpufeatures.h \
    $(wildcard include/config/page/table/isolation.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/required-features.h \
    $(wildcard include/config/x86/minimum/cpu/family.h) \
    $(wildcard include/config/math/emulation.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/x86/cmpxchg64.h) \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/matom.h) \
    $(wildcard include/config/paravirt.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/disabled-features.h \
    $(wildcard include/config/x86/intel/mpx.h) \
    $(wildcard include/config/x86/intel/memory/protection/keys.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/bitops/const_hweight.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/bitops/le.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/byteorder.h \
  /home/kernel/oreo-src/kernel/include/linux/byteorder/little_endian.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/byteorder/little_endian.h \
  /home/kernel/oreo-src/kernel/include/linux/swab.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/swab.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/swab.h \
  /home/kernel/oreo-src/kernel/include/linux/byteorder/generic.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/bitops/ext2-atomic-setbit.h \
  /home/kernel/oreo-src/kernel/include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  /home/kernel/oreo-src/kernel/include/linux/typecheck.h \
  /home/kernel/oreo-src/kernel/include/linux/printk.h \
    $(wildcard include/config/message/loglevel/default.h) \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk/nmi.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  /home/kernel/oreo-src/kernel/include/linux/init.h \
    $(wildcard include/config/debug/rodata.h) \
    $(wildcard include/config/lto/clang.h) \
  /home/kernel/oreo-src/kernel/include/linux/kern_levels.h \
  /home/kernel/oreo-src/kernel/include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  /home/kernel/oreo-src/kernel/include/uapi/linux/kernel.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/sysinfo.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
    $(wildcard include/config/x86/internode/cache/shift.h) \
    $(wildcard include/config/x86/vsmp.h) \
  /home/kernel/oreo-src/kernel/include/linux/dynamic_debug.h \
    $(wildcard include/config/jump/label.h) \
  /home/kernel/oreo-src/kernel/include/linux/stat.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/stat.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/stat.h \
  /home/kernel/oreo-src/kernel/include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  /home/kernel/oreo-src/kernel/include/linux/seqlock.h \
    $(wildcard include/config/debug/lock/alloc.h) \
  /home/kernel/oreo-src/kernel/include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
  /home/kernel/oreo-src/kernel/include/linux/preempt.h \
    $(wildcard include/config/preempt/count.h) \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/preempt.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/percpu.h \
    $(wildcard include/config/x86/64/smp.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  /home/kernel/oreo-src/kernel/include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  /home/kernel/oreo-src/kernel/include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /home/kernel/oreo-src/kernel/include/linux/thread_info.h \
    $(wildcard include/config/thread/info/in/task.h) \
    $(wildcard include/config/debug/stack/usage.h) \
    $(wildcard include/config/have/arch/within/stack/frames.h) \
    $(wildcard include/config/hardened/usercopy.h) \
  /home/kernel/oreo-src/kernel/include/linux/bug.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/bug.h \
    $(wildcard include/config/debug/bugverbose.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  /home/kernel/oreo-src/kernel/include/linux/restart_block.h \
    $(wildcard include/config/compat.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/current.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/thread_info.h \
    $(wildcard include/config/vm86.h) \
    $(wildcard include/config/frame/pointer.h) \
    $(wildcard include/config/ia32/emulation.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/page.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/page_types.h \
    $(wildcard include/config/physical/start.h) \
    $(wildcard include/config/physical/align.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/page_64_types.h \
    $(wildcard include/config/randomize/memory.h) \
    $(wildcard include/config/randomize/base.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/kaslr.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/page_64.h \
    $(wildcard include/config/debug/virtual.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/x86/vsyscall/emulation.h) \
  /home/kernel/oreo-src/kernel/include/linux/range.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/memory_model.h \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/sparsemem.h) \
  /home/kernel/oreo-src/kernel/include/linux/pfn.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/getorder.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/cpufeature.h \
    $(wildcard include/config/x86/feature/names.h) \
    $(wildcard include/config/x86/fast/feature/tests.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/processor.h \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/x86/debugctlmsr.h) \
    $(wildcard include/config/xen.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/processor-flags.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/processor-flags.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/math_emu.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/ptrace.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/segment.h \
    $(wildcard include/config/x86/32/lazy/gs.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/ptrace.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/ptrace-abi.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/ptrace.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/sigcontext.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/pgtable_types.h \
    $(wildcard include/config/kmemcheck.h) \
    $(wildcard include/config/mem/soft/dirty.h) \
    $(wildcard include/config/pgtable/levels.h) \
    $(wildcard include/config/proc/fs.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/pgtable_64_types.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/sparsemem.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/msr.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/msr-index.h \
    $(wildcard include/config/tdp/nominal.h) \
    $(wildcard include/config/tdp/level/1.h) \
    $(wildcard include/config/tdp/level/2.h) \
    $(wildcard include/config/tdp/control.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/errno.h \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/errno.h \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/errno-base.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/cpumask.h \
  /home/kernel/oreo-src/kernel/include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  /home/kernel/oreo-src/kernel/include/linux/bitmap.h \
    $(wildcard include/config/s390.h) \
  /home/kernel/oreo-src/kernel/include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/kernel/oreo-src/kernel/include/uapi/linux/string.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/string.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/string_64.h \
    $(wildcard include/config/x86/mce.h) \
  /home/kernel/oreo-src/kernel/include/linux/jump_label.h \
  /home/kernel/oreo-src/kernel/include/linux/atomic.h \
    $(wildcard include/config/generic/atomic64.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/atomic.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/cmpxchg.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/cmpxchg_64.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/atomic64_64.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/atomic-long.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/msr.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/ioctl.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/ioctl.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/ioctl.h \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/ioctl.h \
  /home/kernel/oreo-src/kernel/include/linux/tracepoint-defs.h \
  /home/kernel/oreo-src/kernel/include/linux/static_key.h \
  /home/kernel/oreo-src/kernel/include/linux/errno.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/errno.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/desc_defs.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/special_insns.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/fpu/types.h \
  /home/kernel/oreo-src/kernel/include/linux/personality.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/personality.h \
  /home/kernel/oreo-src/kernel/include/linux/math64.h \
    $(wildcard include/config/arch/supports/int128.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/div64.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/div64.h \
  /home/kernel/oreo-src/kernel/include/linux/err.h \
  /home/kernel/oreo-src/kernel/include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/irqflags.h \
  /home/kernel/oreo-src/kernel/include/linux/bottom_half.h \
  /home/kernel/oreo-src/kernel/include/linux/spinlock_types.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/spinlock_types.h \
    $(wildcard include/config/paravirt/spinlocks.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/qspinlock_types.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/qrwlock_types.h \
  /home/kernel/oreo-src/kernel/include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
  /home/kernel/oreo-src/kernel/include/linux/rwlock_types.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/spinlock.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/paravirt.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/qspinlock.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/qspinlock.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/qrwlock.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/qrwlock.h \
    $(wildcard include/config/cpu/big/endian.h) \
  /home/kernel/oreo-src/kernel/include/linux/rwlock.h \
  /home/kernel/oreo-src/kernel/include/linux/spinlock_api_smp.h \
    $(wildcard include/config/inline/spin/lock.h) \
    $(wildcard include/config/inline/spin/lock/bh.h) \
    $(wildcard include/config/inline/spin/lock/irq.h) \
    $(wildcard include/config/inline/spin/lock/irqsave.h) \
    $(wildcard include/config/inline/spin/trylock.h) \
    $(wildcard include/config/inline/spin/trylock/bh.h) \
    $(wildcard include/config/uninline/spin/unlock.h) \
    $(wildcard include/config/inline/spin/unlock/bh.h) \
    $(wildcard include/config/inline/spin/unlock/irq.h) \
    $(wildcard include/config/inline/spin/unlock/irqrestore.h) \
  /home/kernel/oreo-src/kernel/include/linux/rwlock_api_smp.h \
    $(wildcard include/config/inline/read/lock.h) \
    $(wildcard include/config/inline/write/lock.h) \
    $(wildcard include/config/inline/read/lock/bh.h) \
    $(wildcard include/config/inline/write/lock/bh.h) \
    $(wildcard include/config/inline/read/lock/irq.h) \
    $(wildcard include/config/inline/write/lock/irq.h) \
    $(wildcard include/config/inline/read/lock/irqsave.h) \
    $(wildcard include/config/inline/write/lock/irqsave.h) \
    $(wildcard include/config/inline/read/trylock.h) \
    $(wildcard include/config/inline/write/trylock.h) \
    $(wildcard include/config/inline/read/unlock.h) \
    $(wildcard include/config/inline/write/unlock.h) \
    $(wildcard include/config/inline/read/unlock/bh.h) \
    $(wildcard include/config/inline/write/unlock/bh.h) \
    $(wildcard include/config/inline/read/unlock/irq.h) \
    $(wildcard include/config/inline/write/unlock/irq.h) \
    $(wildcard include/config/inline/read/unlock/irqrestore.h) \
    $(wildcard include/config/inline/write/unlock/irqrestore.h) \
  /home/kernel/oreo-src/kernel/include/linux/time64.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/time.h \
  /home/kernel/oreo-src/kernel/include/linux/uidgid.h \
    $(wildcard include/config/multiuser.h) \
    $(wildcard include/config/user/ns.h) \
  /home/kernel/oreo-src/kernel/include/linux/highuid.h \
  /home/kernel/oreo-src/kernel/include/linux/kmod.h \
  /home/kernel/oreo-src/kernel/include/linux/gfp.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/zone/device.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/memory/isolation.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/cma.h) \
  /home/kernel/oreo-src/kernel/include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/vm/pgflags.h) \
  /home/kernel/oreo-src/kernel/include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/zsmalloc.h) \
    $(wildcard include/config/memcg.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/page/extension.h) \
    $(wildcard include/config/no/bootmem.h) \
    $(wildcard include/config/numa/balancing.h) \
    $(wildcard include/config/deferred/struct/page/init.h) \
    $(wildcard include/config/transparent/hugepage.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/have/memoryless/nodes.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/have/memblock/node/map.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/have/arch/pfn/valid.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  /home/kernel/oreo-src/kernel/include/linux/wait.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/wait.h \
  /home/kernel/oreo-src/kernel/include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  /home/kernel/oreo-src/kernel/include/linux/nodemask.h \
    $(wildcard include/config/movable/node.h) \
  /home/kernel/oreo-src/kernel/include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  /home/kernel/oreo-src/kernel/include/linux/page-flags-layout.h \
  include/generated/bounds.h \
  /home/kernel/oreo-src/kernel/include/linux/memory_hotplug.h \
    $(wildcard include/config/memory/hotremove.h) \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/have/bootmem/info/node.h) \
  /home/kernel/oreo-src/kernel/include/linux/notifier.h \
  /home/kernel/oreo-src/kernel/include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/mutex/spin/on/owner.h) \
  /home/kernel/oreo-src/kernel/include/linux/osq_lock.h \
  /home/kernel/oreo-src/kernel/include/linux/rwsem.h \
    $(wildcard include/config/rwsem/spin/on/owner.h) \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/rwsem.h \
  /home/kernel/oreo-src/kernel/include/linux/srcu.h \
  /home/kernel/oreo-src/kernel/include/linux/rcupdate.h \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/rcu/trace.h) \
    $(wildcard include/config/rcu/stall/common.h) \
    $(wildcard include/config/no/hz/full.h) \
    $(wildcard include/config/rcu/nocb/cpu.h) \
    $(wildcard include/config/tasks/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/prove/rcu.h) \
    $(wildcard include/config/rcu/boost.h) \
    $(wildcard include/config/rcu/nocb/cpu/all.h) \
    $(wildcard include/config/no/hz/full/sysidle.h) \
  /home/kernel/oreo-src/kernel/include/linux/completion.h \
  /home/kernel/oreo-src/kernel/include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/debug/objects/free.h) \
  /home/kernel/oreo-src/kernel/include/linux/ktime.h \
  /home/kernel/oreo-src/kernel/include/linux/jiffies.h \
  /home/kernel/oreo-src/kernel/include/linux/timex.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/timex.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/param.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/param.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  /home/kernel/oreo-src/kernel/include/uapi/asm-generic/param.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/timex.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/tsc.h \
    $(wildcard include/config/x86/tsc.h) \
  include/generated/timeconst.h \
  /home/kernel/oreo-src/kernel/include/linux/timekeeping.h \
  /home/kernel/oreo-src/kernel/include/linux/rcutree.h \
  /home/kernel/oreo-src/kernel/include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
    $(wildcard include/config/wq/watchdog.h) \
  /home/kernel/oreo-src/kernel/include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
    $(wildcard include/config/no/hz/common.h) \
  /home/kernel/oreo-src/kernel/include/linux/sysctl.h \
    $(wildcard include/config/sysctl.h) \
  /home/kernel/oreo-src/kernel/include/linux/rbtree.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/sysctl.h \
  /home/kernel/oreo-src/kernel/include/linux/topology.h \
    $(wildcard include/config/use/percpu/numa/node/id.h) \
    $(wildcard include/config/sched/smt.h) \
  /home/kernel/oreo-src/kernel/include/linux/smp.h \
    $(wildcard include/config/up/late/init.h) \
  /home/kernel/oreo-src/kernel/include/linux/llist.h \
    $(wildcard include/config/arch/have/nmi/safe/cmpxchg.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/smp.h \
    $(wildcard include/config/x86/local/apic.h) \
    $(wildcard include/config/x86/io/apic.h) \
    $(wildcard include/config/x86/32/smp.h) \
    $(wildcard include/config/debug/nmi/selftest.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/mpspec.h \
    $(wildcard include/config/eisa.h) \
    $(wildcard include/config/x86/mpparse.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/mpspec_def.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/x86_init.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/bootparam.h \
  /home/kernel/oreo-src/kernel/include/linux/screen_info.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/screen_info.h \
  /home/kernel/oreo-src/kernel/include/linux/apm_bios.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/apm_bios.h \
  /home/kernel/oreo-src/kernel/include/linux/edd.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/edd.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/e820.h \
    $(wildcard include/config/efi.h) \
    $(wildcard include/config/hibernation.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/e820.h \
    $(wildcard include/config/x86/pmem/legacy.h) \
    $(wildcard include/config/intel/txt.h) \
  /home/kernel/oreo-src/kernel/include/linux/ioport.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/ist.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/ist.h \
  /home/kernel/oreo-src/kernel/include/video/edid.h \
    $(wildcard include/config/x86.h) \
  /home/kernel/oreo-src/kernel/include/uapi/video/edid.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/apicdef.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/apic.h \
    $(wildcard include/config/x86/x2apic.h) \
  /home/kernel/oreo-src/kernel/include/linux/pm.h \
    $(wildcard include/config/vt/console/sleep.h) \
    $(wildcard include/config/pm.h) \
    $(wildcard include/config/pm/clk.h) \
    $(wildcard include/config/pm/generic/domains.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/fixmap.h \
    $(wildcard include/config/provide/ohci1394/dma/init.h) \
    $(wildcard include/config/pci/mmconfig.h) \
    $(wildcard include/config/x86/intel/mid.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/acpi.h \
    $(wildcard include/config/acpi/apei.h) \
    $(wildcard include/config/acpi.h) \
    $(wildcard include/config/acpi/numa.h) \
  /home/kernel/oreo-src/kernel/include/acpi/pdc_intel.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/numa.h \
    $(wildcard include/config/numa/emu.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/topology.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/topology.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/mmu.h \
    $(wildcard include/config/modify/ldt/syscall.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/realmode.h \
    $(wildcard include/config/acpi/sleep.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/io.h \
    $(wildcard include/config/mtrr.h) \
    $(wildcard include/config/x86/pat.h) \
  arch/x86/include/generated/asm/early_ioremap.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/early_ioremap.h \
    $(wildcard include/config/generic/early/ioremap.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/iomap.h \
    $(wildcard include/config/has/ioport/map.h) \
    $(wildcard include/config/pci.h) \
    $(wildcard include/config/generic/iomap.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/pci_iomap.h \
    $(wildcard include/config/no/generic/pci/ioport/map.h) \
    $(wildcard include/config/generic/pci/iomap.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/vsyscall.h \
  /home/kernel/oreo-src/kernel/include/asm-generic/fixmap.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/idle.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/io_apic.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/irq_vectors.h \
    $(wildcard include/config/have/kvm.h) \
    $(wildcard include/config/pci/msi.h) \
  /home/kernel/oreo-src/kernel/include/linux/percpu.h \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
  /home/kernel/oreo-src/kernel/include/linux/elf.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/elf.h \
    $(wildcard include/config/x86/x32/abi.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/user.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/user_64.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/uapi/asm/auxvec.h \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/vdso.h \
    $(wildcard include/config/x86/x32.h) \
  /home/kernel/oreo-src/kernel/include/linux/mm_types.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/arch/enable/split/pmd/ptlock.h) \
    $(wildcard include/config/have/cmpxchg/double.h) \
    $(wildcard include/config/have/aligned/struct/page.h) \
    $(wildcard include/config/userfaultfd.h) \
    $(wildcard include/config/aio.h) \
    $(wildcard include/config/mmu/notifier.h) \
    $(wildcard include/config/arch/want/batched/unmap/tlb/flush.h) \
  /home/kernel/oreo-src/kernel/include/linux/auxvec.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/auxvec.h \
  /home/kernel/oreo-src/kernel/include/linux/uprobes.h \
    $(wildcard include/config/uprobes.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/uprobes.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/elf.h \
  /home/kernel/oreo-src/kernel/include/uapi/linux/elf-em.h \
  /home/kernel/oreo-src/kernel/include/linux/kobject.h \
    $(wildcard include/config/uevent/helper.h) \
    $(wildcard include/config/debug/kobject/release.h) \
  /home/kernel/oreo-src/kernel/include/linux/sysfs.h \
  /home/kernel/oreo-src/kernel/include/linux/kernfs.h \
    $(wildcard include/config/kernfs.h) \
  /home/kernel/oreo-src/kernel/include/linux/idr.h \
  /home/kernel/oreo-src/kernel/include/linux/kobject_ns.h \
  /home/kernel/oreo-src/kernel/include/linux/kref.h \
  /home/kernel/oreo-src/kernel/include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  /home/kernel/oreo-src/kernel/include/linux/extable.h \
  /home/kernel/oreo-src/kernel/include/linux/rbtree_latch.h \
  /home/kernel/oreo-src/kernel/include/linux/cfi.h \
    $(wildcard include/config/cfi/clang/shadow.h) \
  /home/kernel/oreo-src/kernel/arch/x86/include/asm/module.h \
    $(wildcard include/config/m486.h) \
    $(wildcard include/config/m586.h) \
    $(wildcard include/config/m586tsc.h) \
    $(wildcard include/config/m586mmx.h) \
    $(wildcard include/config/mcore2.h) \
    $(wildcard include/config/m686.h) \
    $(wildcard include/config/mpentiumii.h) \
    $(wildcard include/config/mpentiumiii.h) \
    $(wildcard include/config/mpentiumm.h) \
    $(wildcard include/config/mpentium4.h) \
    $(wildcard include/config/mk6.h) \
    $(wildcard include/config/mk8.h) \
    $(wildcard include/config/melan.h) \
    $(wildcard include/config/mcrusoe.h) \
    $(wildcard include/config/mefficeon.h) \
    $(wildcard include/config/mwinchipc6.h) \
    $(wildcard include/config/mwinchip3d.h) \
    $(wildcard include/config/mcyrixiii.h) \
    $(wildcard include/config/mviac3/2.h) \
    $(wildcard include/config/mviac7.h) \
    $(wildcard include/config/mgeodegx1.h) \
    $(wildcard include/config/mgeode/lx.h) \
  /home/kernel/oreo-src/kernel/include/asm-generic/module.h \
    $(wildcard include/config/have/mod/arch/specific.h) \
    $(wildcard include/config/modules/use/elf/rel.h) \
    $(wildcard include/config/modules/use/elf/rela.h) \
  /home/kernel/oreo-src/kernel/include/linux/vermagic.h \
  include/generated/utsrelease.h \

/home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.o: $(deps_/home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.o)

$(deps_/home/kernel/LIN/6P6/practfinal/ParteB/multilist.mod.o):
