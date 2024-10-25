/*
 * C array-like wrapper
 */
#[repr(C)]
pub struct ConstVec<T> {
   pub ptr : *const T,
   pub len : usize,
   pub cap: usize,
}

pub fn vec_to_const_vec<T>(vec: &Vec<T>) -> *mut ConstVec<T>
{
    return Box::into_raw(
            Box::new(
                ConstVec::<T>{
                    ptr : vec.as_ptr(),
                    len : vec.len(),
                    cap : vec.capacity(),
                }
            )
        );
}


pub fn const_vec_destroy<T>(vec: *mut ConstVec<T>)
{
    unsafe {
        let _ = Box::from_raw(vec);
    }
}
