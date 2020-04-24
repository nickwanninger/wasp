/* automatically generated by rust-bindgen */

pub type size_t = ::std::os::raw::c_ulong;
pub type __uint8_t = ::std::os::raw::c_uchar;
pub type __uint16_t = ::std::os::raw::c_ushort;
pub type __uint32_t = ::std::os::raw::c_uint;
pub type __uint64_t = ::std::os::raw::c_ulong;
#[repr(C)]
pub struct wasp_regs_t {
    pub rax: u64,
    pub rbx: u64,
    pub rcx: u64,
    pub rdx: u64,
    pub rsi: u64,
    pub rdi: u64,
    pub rsp: u64,
    pub rbp: u64,
    pub r8: u64,
    pub r9: u64,
    pub r10: u64,
    pub r11: u64,
    pub r12: u64,
    pub r13: u64,
    pub r14: u64,
    pub r15: u64,
    pub rip: u64,
    pub rflags: u64,
}
#[test]
fn bindgen_test_layout_wasp_regs_t() {
    assert_eq!(
        ::std::mem::size_of::<wasp_regs_t>(),
        144usize,
        concat!("Size of: ", stringify!(wasp_regs_t))
    );
    assert_eq!(
        ::std::mem::align_of::<wasp_regs_t>(),
        8usize,
        concat!("Alignment of ", stringify!(wasp_regs_t))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rax as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rax)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rbx as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rbx)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rcx as *const _ as usize },
        16usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rcx)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rdx as *const _ as usize },
        24usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rdx)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rsi as *const _ as usize },
        32usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rsi)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rdi as *const _ as usize },
        40usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rdi)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rsp as *const _ as usize },
        48usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rsp)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rbp as *const _ as usize },
        56usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rbp)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r8 as *const _ as usize },
        64usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r8)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r9 as *const _ as usize },
        72usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r9)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r10 as *const _ as usize },
        80usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r10)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r11 as *const _ as usize },
        88usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r11)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r12 as *const _ as usize },
        96usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r12)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r13 as *const _ as usize },
        104usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r13)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r14 as *const _ as usize },
        112usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r14)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).r15 as *const _ as usize },
        120usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(r15)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rip as *const _ as usize },
        128usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rip)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_t>())).rflags as *const _ as usize },
        136usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_t),
            "::",
            stringify!(rflags)
        )
    );
}
#[repr(C)]
pub struct wasp_segment_t {
    pub base: u64,
    pub limit: u32,
    pub selector: u16,
    pub type_: u8,
    pub present: u8,
    pub dpl: u8,
    pub db: u8,
    pub s: u8,
    pub long_mode: u8,
    pub granularity: u8,
    pub available: u8,
    pub unusable: u8,
}
#[test]
fn bindgen_test_layout_wasp_segment_t() {
    assert_eq!(
        ::std::mem::size_of::<wasp_segment_t>(),
        24usize,
        concat!("Size of: ", stringify!(wasp_segment_t))
    );
    assert_eq!(
        ::std::mem::align_of::<wasp_segment_t>(),
        8usize,
        concat!("Alignment of ", stringify!(wasp_segment_t))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).base as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(base)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).limit as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(limit)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).selector as *const _ as usize },
        12usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(selector)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).type_ as *const _ as usize },
        14usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(type_)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).present as *const _ as usize },
        15usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(present)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).dpl as *const _ as usize },
        16usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(dpl)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).db as *const _ as usize },
        17usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(db)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).s as *const _ as usize },
        18usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(s)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).long_mode as *const _ as usize },
        19usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(long_mode)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).granularity as *const _ as usize },
        20usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(granularity)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).available as *const _ as usize },
        21usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(available)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_segment_t>())).unusable as *const _ as usize },
        22usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_segment_t),
            "::",
            stringify!(unusable)
        )
    );
}
#[repr(C)]
pub struct wasp_dtable_t {
    pub base: u64,
    pub limit: u16,
}
#[test]
fn bindgen_test_layout_wasp_dtable_t() {
    assert_eq!(
        ::std::mem::size_of::<wasp_dtable_t>(),
        16usize,
        concat!("Size of: ", stringify!(wasp_dtable_t))
    );
    assert_eq!(
        ::std::mem::align_of::<wasp_dtable_t>(),
        8usize,
        concat!("Alignment of ", stringify!(wasp_dtable_t))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_dtable_t>())).base as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_dtable_t),
            "::",
            stringify!(base)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_dtable_t>())).limit as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_dtable_t),
            "::",
            stringify!(limit)
        )
    );
}
#[repr(C)]
pub struct wasp_regs_special_t {
    pub cs: wasp_segment_t,
    pub ds: wasp_segment_t,
    pub es: wasp_segment_t,
    pub fs: wasp_segment_t,
    pub gs: wasp_segment_t,
    pub ss: wasp_segment_t,
    pub tr: wasp_segment_t,
    pub ldt: wasp_segment_t,
    pub gdt: wasp_dtable_t,
    pub idt: wasp_dtable_t,
    pub cr0: u64,
    pub cr2: u64,
    pub cr3: u64,
    pub cr4: u64,
    pub cr8: u64,
    pub efer: u64,
    pub apic_base: u64,
    pub interrupt_bitmap: [u64; 4usize],
}
#[test]
fn bindgen_test_layout_wasp_regs_special_t() {
    assert_eq!(
        ::std::mem::size_of::<wasp_regs_special_t>(),
        312usize,
        concat!("Size of: ", stringify!(wasp_regs_special_t))
    );
    assert_eq!(
        ::std::mem::align_of::<wasp_regs_special_t>(),
        8usize,
        concat!("Alignment of ", stringify!(wasp_regs_special_t))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).cs as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(cs)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).ds as *const _ as usize },
        24usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(ds)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).es as *const _ as usize },
        48usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(es)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).fs as *const _ as usize },
        72usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(fs)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).gs as *const _ as usize },
        96usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(gs)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).ss as *const _ as usize },
        120usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(ss)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).tr as *const _ as usize },
        144usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(tr)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).ldt as *const _ as usize },
        168usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(ldt)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).gdt as *const _ as usize },
        192usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(gdt)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).idt as *const _ as usize },
        208usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(idt)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).cr0 as *const _ as usize },
        224usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(cr0)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).cr2 as *const _ as usize },
        232usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(cr2)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).cr3 as *const _ as usize },
        240usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(cr3)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).cr4 as *const _ as usize },
        248usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(cr4)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).cr8 as *const _ as usize },
        256usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(cr8)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).efer as *const _ as usize },
        264usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(efer)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_special_t>())).apic_base as *const _ as usize },
        272usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(apic_base)
        )
    );
    assert_eq!(
        unsafe {
            &(*(::std::ptr::null::<wasp_regs_special_t>())).interrupt_bitmap as *const _ as usize
        },
        280usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_special_t),
            "::",
            stringify!(interrupt_bitmap)
        )
    );
}
#[repr(C)]
pub struct wasp_regs_fpu_t {
    pub fpr: [[u8; 16usize]; 8usize],
    pub fcw: u16,
    pub fsw: u16,
    pub ftwx: u8,
    pub last_opcode: u16,
    pub last_ip: u64,
    pub last_dp: u64,
    pub xmm: [[u8; 16usize]; 16usize],
    pub mxcsr: u32,
}
#[test]
fn bindgen_test_layout_wasp_regs_fpu_t() {
    assert_eq!(
        ::std::mem::size_of::<wasp_regs_fpu_t>(),
        416usize,
        concat!("Size of: ", stringify!(wasp_regs_fpu_t))
    );
    assert_eq!(
        ::std::mem::align_of::<wasp_regs_fpu_t>(),
        8usize,
        concat!("Alignment of ", stringify!(wasp_regs_fpu_t))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).fpr as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(fpr)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).fcw as *const _ as usize },
        128usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(fcw)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).fsw as *const _ as usize },
        130usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(fsw)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).ftwx as *const _ as usize },
        132usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(ftwx)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).last_opcode as *const _ as usize },
        134usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(last_opcode)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).last_ip as *const _ as usize },
        136usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(last_ip)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).last_dp as *const _ as usize },
        144usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(last_dp)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).xmm as *const _ as usize },
        152usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(xmm)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_regs_fpu_t>())).mxcsr as *const _ as usize },
        408usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_regs_fpu_t),
            "::",
            stringify!(mxcsr)
        )
    );
}
pub type wasp_workload_init_fn = ::std::option::Option<
    unsafe extern "C" fn(self_: *mut wasp_workload_t, config: *mut ::std::os::raw::c_void),
>;
pub type wasp_workload_handle_hcall_fn = ::std::option::Option<
    unsafe extern "C" fn(
        self_: *mut wasp_workload_t,
        regs: *mut wasp_regs_t,
        ramsize: size_t,
        ram: *mut ::std::os::raw::c_void,
    ) -> ::std::os::raw::c_int,
>;
pub type wasp_workload_handle_exit_fn =
    ::std::option::Option<unsafe extern "C" fn(self_: *mut wasp_workload_t)>;
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct wasp_workload_t {
    pub ctx: *mut ::std::os::raw::c_void,
    pub init: wasp_workload_init_fn,
    pub handle_hcall: wasp_workload_handle_hcall_fn,
    pub handle_exit: wasp_workload_handle_exit_fn,
}
#[test]
fn bindgen_test_layout_wasp_workload_t() {
    assert_eq!(
        ::std::mem::size_of::<wasp_workload_t>(),
        32usize,
        concat!("Size of: ", stringify!(wasp_workload_t))
    );
    assert_eq!(
        ::std::mem::align_of::<wasp_workload_t>(),
        8usize,
        concat!("Alignment of ", stringify!(wasp_workload_t))
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_workload_t>())).ctx as *const _ as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_workload_t),
            "::",
            stringify!(ctx)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_workload_t>())).init as *const _ as usize },
        8usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_workload_t),
            "::",
            stringify!(init)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_workload_t>())).handle_hcall as *const _ as usize },
        16usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_workload_t),
            "::",
            stringify!(handle_hcall)
        )
    );
    assert_eq!(
        unsafe { &(*(::std::ptr::null::<wasp_workload_t>())).handle_exit as *const _ as usize },
        24usize,
        concat!(
            "Offset of field: ",
            stringify!(wasp_workload_t),
            "::",
            stringify!(handle_exit)
        )
    );
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct wasp_machine_t {
    _unused: [u8; 0],
}
extern "C" {
    pub fn wasp_machine_create(memsize: size_t) -> *mut wasp_machine_t;
}
extern "C" {
    pub fn wasp_machine_free(self_: *mut wasp_machine_t);
}
extern "C" {
    pub fn wasp_machine_run(self_: *mut wasp_machine_t, workload: *mut wasp_workload_t);
}
extern "C" {
    pub fn wasp_machine_reset(self_: *mut wasp_machine_t);
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct wasp_loader_t {
    _unused: [u8; 0],
}
extern "C" {
    pub fn wasp_loader_inject(self_: *mut wasp_loader_t, vm: *mut wasp_machine_t) -> bool;
}
extern "C" {
    pub fn wasp_loader_free(self_: *mut wasp_loader_t);
}
pub type wasp_loader_create_fn_t = ::std::option::Option<
    unsafe extern "C" fn(path: *const ::std::os::raw::c_char) -> *mut wasp_loader_t,
>;
extern "C" {
    pub fn wasp_elf_loader_create(path: *const ::std::os::raw::c_char) -> *mut wasp_loader_t;
}
extern "C" {
    pub fn wasp_flatbin_loader_create(path: *const ::std::os::raw::c_char) -> *mut wasp_loader_t;
}
