use revm_interpreter::{analysis::validate_raw_eof_inner, primitives::Bytes, analysis::CodeType};

#[no_mangle]
pub extern "C" fn fzz_revm_validate_eof(data: *const u8, len: usize) -> bool {
    let slice = unsafe { std::slice::from_raw_parts(data, len) };
    let r = validate_raw_eof_inner(Bytes::from(slice), Some(CodeType::ReturnOrStop));
    match r {
        Ok(_) => true,
        Err(_) => false,
    }
}
