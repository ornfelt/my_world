use crate::{WOW_SRP_ERROR_INVALID_PUBLIC_KEY, WOW_SRP_ERROR_NULL_POINTER};
use std::ffi::{c_char, CStr};
use wow_srp::normalized_string::NormalizedString;
use wow_srp::PublicKey;

pub fn char_ptr_to_string(ptr: *const c_char, out_error: *mut c_char) -> Option<NormalizedString> {
    if ptr.is_null() {
        write_error(out_error, crate::WOW_SRP_ERROR_NULL_POINTER);
        return None;
    }

    let str = match unsafe { CStr::from_ptr(ptr).to_str() } {
        Ok(e) => e,
        Err(_) => {
            write_error(out_error, crate::WOW_SRP_ERROR_UTF8);
            return None;
        }
    };

    let str = match NormalizedString::new(str) {
        Ok(e) => e,
        Err(_) => {
            write_error(
                out_error,
                crate::WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME,
            );
            return None;
        }
    };

    Some(str)
}

pub(crate) fn is_null<T>(ptr: *const T, out_error: *mut c_char) -> Option<&'static T> {
    if ptr.is_null() {
        write_error(out_error, crate::WOW_SRP_ERROR_NULL_POINTER);
        return None;
    }

    unsafe { Some(&(*ptr)) }
}

pub(crate) fn is_null_mut<T>(ptr: *mut T, out_error: *mut c_char) -> Option<&'static mut T> {
    if ptr.is_null() {
        write_error(out_error, crate::WOW_SRP_ERROR_NULL_POINTER);
        return None;
    }

    unsafe { Some(&mut (*ptr)) }
}

pub(crate) fn retake_ownership<T>(ptr: *mut T, out_error: *mut c_char) -> Option<Box<T>> {
    if ptr.is_null() {
        write_error(out_error, crate::WOW_SRP_ERROR_NULL_POINTER);
        return None;
    }

    Some(unsafe { Box::from_raw(ptr) })
}

pub(crate) fn free_box_ptr<T>(ptr: *mut T) {
    if ptr.is_null() {
        return;
    }

    let _ = unsafe { Box::from_raw(ptr) };
}

pub(crate) fn write_error(out_error: *mut c_char, error: c_char) {
    if !out_error.is_null() {
        unsafe { std::ptr::write(out_error, error) };
    }
}

pub(crate) fn read_public_key(key: *const u8, out_error: *mut c_char) -> Option<PublicKey> {
    let Some(key) = read_array(key, out_error) else {
        return None;
    };
    let Ok(key) = PublicKey::from_le_bytes(key) else {
        write_error(out_error, WOW_SRP_ERROR_INVALID_PUBLIC_KEY);
        return None;
    };

    Some(key)
}

pub(crate) fn read_array<const N: usize>(
    array: *const u8,
    out_error: *mut c_char,
) -> Option<[u8; N]> {
    if array.is_null() {
        write_error(out_error, WOW_SRP_ERROR_NULL_POINTER);
        return None;
    }

    let mut temp = [0_u8; N];

    for i in 0..N {
        let j = i as isize;
        temp[i] = unsafe { *array.offset(j) };
    }

    Some(temp)
}

pub(crate) fn write_array(array: *mut u8, data: &[u8]) {
    for (i, d) in data.iter().enumerate() {
        unsafe { array.offset(i as isize).write(*d) };
    }
}
