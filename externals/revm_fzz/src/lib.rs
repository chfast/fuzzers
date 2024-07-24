#[no_mangle]
pub extern "C" fn fzz_revm_validate_eof(data: *const u8, len: usize) -> bool {
    let slice = unsafe { std::slice::from_raw_parts(data, len) };
    let b = revm_interpreter::primitives::Bytes::from(slice);
    let r = revm_interpreter::analysis::validate_raw_eof(b);
    match r {
        Ok(_) => true,
        Err(_) => false
    }
}
