// Copyright 2018 Jean Pierre Dudey <jeandudey@hotmail.com>
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

extern crate libc;

use std::mem::transmute;
use std::thread;
use libc::{c_void, c_char, size_t};

mod ffi;

pub const FAST_HASH_LENGTH: usize = 32;
pub const SLOW_HASH_LENGTH: usize = 32;

pub fn fast_hash(data: &[u8]) -> [u8; FAST_HASH_LENGTH] {
    use ffi::cn_fast_hash;

    debug_assert!(FAST_HASH_LENGTH == ffi::HASH_SIZE);

    let output = &mut [0u8; FAST_HASH_LENGTH];    
    unsafe {
        cn_fast_hash(
            data.as_ptr() as *const c_void,
            data.len() as size_t,
            transmute::<*mut u8, *mut c_char>(output.as_mut_ptr()),
        )
    }

    *output
}

pub fn slow_hash(data: &[u8]) -> [u8; SLOW_HASH_LENGTH] {
    use ffi::cn_slow_hash;

    debug_assert!(SLOW_HASH_LENGTH == ffi::HASH_SIZE);

    // FIXME: this is stupid, do it the safe way
    // *const u8 can't be sent between threads.
    let data_ptr = unsafe { transmute::<*const u8, usize>(data.as_ptr()) };
    let data_len = data.len() as size_t;

    let child = thread::Builder::new().stack_size(4194304).spawn(move || {
        let output = &mut [0u8; FAST_HASH_LENGTH];    
        unsafe {
            cn_slow_hash(
                transmute::<usize, *const c_void>(data_ptr),
                data_len,
                transmute::<*mut u8, *mut c_char>(output.as_mut_ptr()),
            )
        }

        *output
    }).unwrap();

    child.join().unwrap()
}