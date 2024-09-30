use std::fmt::Write;

pub(crate) struct Writer {
    pub inner: String,
}

impl Writer {
    pub fn new() -> Self {
        Self {
            inner: String::with_capacity(8000),
        }
    }

    pub fn w(&mut self, s: impl AsRef<str>) {
        self.inner.write_str(s.as_ref()).unwrap();
    }

    pub fn newline(&mut self) {
        self.w("\n");
    }

    pub fn wln(&mut self, s: impl AsRef<str>) {
        self.w(s);
        self.newline();
    }
}
