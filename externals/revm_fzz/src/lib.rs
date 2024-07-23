pub fn add(left: usize, right: usize) -> usize {
    left + right
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }
}

#[no_mangle]
pub extern "C" fn fzz_add(a:u32, b:u32) -> u32 {
    return add(a as usize, b as usize) as u32
}
