use revm_interpreter::{analysis::validate_raw_eof_inner, primitives::Bytes, analysis::CodeType};

#[no_mangle]
pub extern "C" fn fzz_revm_validate_eof(data: *const u8, len: usize) -> i32 {
    let slice = unsafe { std::slice::from_raw_parts(data, len) };
    let ok = match validate_raw_eof_inner(Bytes::from(slice), Some(CodeType::ReturnOrStop)) {
        Ok(_) => 1,
        Err(_) => 0,
    };
    let ik = match validate_raw_eof_inner(Bytes::from(slice), Some(CodeType::ReturnContract)) {
        Ok(_) => 2,
        Err(_) => 0,
    };
    ok | ik
}
