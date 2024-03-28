#![allow(non_camel_case_types)]

use libc::{c_void, c_char, size_t};

pub const HASH_SIZE: usize = 32;

extern "C" {
    pub fn cn_fast_hash(data: *const c_void, length: size_t, hash: *mut c_char);
    pub fn cn_slow_hash(data: *const c_void, length: size_t, hash: *mut c_char);
}