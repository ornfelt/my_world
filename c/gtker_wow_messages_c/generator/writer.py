import typing


class Writer:
    def __init__(self):
        self.__inner_bytes: bytearray = bytearray()
        self.__indentation: int = 0

    def w(self, s: str):
        for _ in range(0, self.__indentation):
            self.__inner_bytes += b"    "

        self.__inner_bytes += s.encode("utf-8")

    def wln(self, s: str):
        self.w(s)
        self.newline()

    def open_curly(self, s: str):
        self.wln(f"{s} {{")
        self.inc_indent()

    def closing_curly(self, s: str = ""):
        self.dec_indent()
        self.wln(f"}}{s}")

    def write_block(self, s: str):
        def count_spaces(x: str):
            for index, ch in enumerate(x):
                if index == 0 and ch == '\n' and x[1] == ' ':
                    continue
                if ch != ' ':
                    return index
            return len(x)
        prefix_spaces = count_spaces(s)
        prefix_spaces = (prefix_spaces - 1) if (prefix_spaces > 0) else 0

        for i, line in enumerate(s.splitlines()):
            if i == 0:
                continue

            if line.strip() == "":
                self.newline()
                continue

            self.wln(line[prefix_spaces:])

    def column(self) -> int:
        for i, ch in enumerate(reversed(self.__inner_bytes)):
            if ch == b"\n":
                return i

        return len(self.__inner_bytes)

    def w_break_at(self, s: str):
        if self.column() >= 80:
            self.newline()
        self.w_no_indent(s)

    def wln_no_indent(self, s):
        self.w_no_indent(s)
        self.newline()

    def w_no_indent(self, s):
        self.__inner_bytes += s.encode("utf-8")

    def newline(self):
        self.__inner_bytes += b"\n"

    def double_newline(self):
        self.newline()
        self.newline()

    def prepend(self, s: "Writer"):
        s.__inner_bytes.extend(self.__inner_bytes)
        self.__inner_bytes = s.__inner_bytes

    def append(self, s: "Writer"):
        self.__inner_bytes.extend(s.__inner_bytes)

    def inner(self) -> str:
        assert self.__indentation == 0
        return self.__inner_bytes.decode('utf-8')

    def inc_indent(self):
        if self.__indentation == 255:
            raise AssertionError("indentation overflow")

        self.__indentation += 1

    def dec_indent(self):
        if self.__indentation == 0:
            raise AssertionError("indentation underflow")

        self.__indentation -= 1
