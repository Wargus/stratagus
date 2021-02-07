import os
import re

from os.path import join, dirname, basename, abspath


REGISTER_RE = re.compile(r'lua_register\([a-zA-Z0-9_]+, "([^"]+)", ([^\)]+)\)');
END_OF_DOC_RE = re.compile(r'\*/\s*$')
C_COMMENT_RE = re.compile(r'/\*[^*]*\*+(?:[^/*][^*]*\*+)*/', re.MULTILINE);
BLANK_RE = re.compile(r'^\s*$')
COMMENT_RE = re.compile(r'^\s*/?\*+/?', re.MULTILINE)
PKG_EXPORT_RE = re.compile(r'(?:(?:extern )?[a-zA-Z0-9_]+ ([a-zA-Z0-9_]+\([^)]+\))|(?:extern )?([a-zA-Z0-9_]+ [a-zA-Z0-9_\[\]\*]+));')


def get_c_func_re(c_func):
    return re.compile(r'int\s+' + c_func + '[^;]+$', re.MULTILINE)


def walk_files(directory, exts):
    """Yield filenames recursively under directory that end with exts"""
    for root, dirs, files in os.walk(directory):
        for name in files:
            for ext in exts:
                if name.endswith(ext):
                    yield join(root, name)
                    break


def walk_lua_registers(filename):
    """Yield (lua name, c function name) pairs"""
    with open(filename, "r", errors="ignore") as f:
        for line in f.readlines():
            m = REGISTER_RE.search(line)
            if m:
                yield m.group(1), m.group(2)


def walk_lua_exports(filename):
    with open(filename, "r", errors="ignore") as f:
        block = None
        current_comment = []
        for line in f.readlines():
            if block:
                block.append(line)
                if line.strip() == "};":
                    yield "".join(current_comment), "".join(block)
                    current_comment = []
                    block = None
            elif line.startswith("//"):
                current_comment.append(line)
            elif line.startswith("class") or line.startswith("enum"):
                block = [line]
            else:
                m = PKG_EXPORT_RE.search(line)
                if m:
                    yield "".join(current_comment), m.group(1) or m.group(2)
                current_comment = []


def extract_docstring(filename, c_func):
    """Return the docstring for function and the start of the line for that function"""
    with open(filename, "r", errors="ignore") as f:
        content = f.read()
        m = get_c_func_re(c_func).search(content)
        if m:
            beginning = m.start
            preamble = content[:m.start()]
            lines = preamble.split("\n")
            if len(lines) > 2:
                if END_OF_DOC_RE.search(lines[-2]):
                    # line prior to declaration looks like end of comment
                    idx = 2
                    while len(lines) > idx:
                        if BLANK_RE.match(lines[-idx]):
                            break
                        idx += 1
                    m = C_COMMENT_RE.search("\n".join(lines[-idx:-1]))
                    docstring = m.group(0)
                    return COMMENT_RE.sub("", docstring), len(lines)
    return "&lt;documentation not found&gt;", -1


if __name__ == "__main__":
    root = join(dirname(dirname(dirname(abspath(__file__)))), "src")

    with open("index.html", "w") as toc:
        toc.write("""
        <html>
        <head><title>TOC</title></head>
        <body>
        """)
        for cppfile in walk_files(root, exts=[".cpp"]):
            rule = False
            for lua_register in walk_lua_registers(cppfile):
                if not rule:
                    toc.write("<hr>")
                    rule = True
                luaname, cname = lua_register
                doc, line = extract_docstring(cppfile, cname)
                doc = doc.replace("\n", "<br>")
                toc.write("""
                <details>
                <summary>{0}</summary>
                <p>{1}</p>
                <p>{2}:{3} {4}</p>
                </details>
                """.format(luaname, doc, cppfile.replace(root, ""), line, cname))
        for pkgfile in walk_files(root, exts=[".pkg"]):
            rule = False
            for lua_register in walk_lua_exports(pkgfile):
                if not rule:
                    toc.write("<hr>")
                    rule = True
                doc, code = lua_register
                doc = doc.replace("\n", "<br>")
                if "\n" not in code:
                    toc.write("""
                    <details>
                    <summary>{0}</summary>
                    <p>{1}</p>
                    </details>
                    """.format(code, doc))
                else:
                    code = code.replace(" ", "&nbsp;").replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;")
                    code = code.split("\n")
                    toc.write("""
                    <details>
                    <summary>{0}</summary>
                    <p>{1}</p>
                    <p>{2}</p>
                    </details>
                    """.format(code[0], doc, "<br>".join(code)))
        toc.write("""
        </body>
        </html>
        """)
