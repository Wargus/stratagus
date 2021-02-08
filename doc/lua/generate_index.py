import os
import re

try:
    import pygments
    from pygments.lexers import CppLexer
    from pygments.formatters import HtmlFormatter
    def highlight(src):
        return pygments.highlight(src, CppLexer(), HtmlFormatter())
    CSS = HtmlFormatter().get_style_defs()
except:
    print("Pygments is not installed, no syntax highlighting")
    def highlight(src):
        return src.replace("\n", "<br>").replace(" ", "&nbsp;").replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;")
    CSS = ""

from os.path import join, dirname, basename, abspath


REGISTER_RE = re.compile(r'lua_register\([a-zA-Z0-9_]+, "([^"]+)", ([^\)]+)\)');
END_OF_DOC_RE = re.compile(r'\*/\s*$')
C_COMMENT_RE = re.compile(r'/\*[^*]*\*+(?:[^/*][^*]*\*+)*/', re.MULTILINE);
BLANK_RE = re.compile(r'^\s*$')
COMMENT_RE = re.compile(r'^\s*/?\*+/?', re.MULTILINE)
PKG_EXPORT_RE = re.compile(r'^(?:extern )?((?:unsigned )?[a-zA-Z0-9_]+ [a-zA-Z0-9_\[\]\*]+(?:\([^)]+\))?);')


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
                    item = m.group(1)
                    item = item.replace("void ", "").replace("tolua_readonly", "readonly")
                    yield "".join(current_comment), item
                current_comment = []


def extract_docstring(filename, c_func):
    """Return the docstring for function, the start of the line for that function, and its source"""
    with open(filename, "r", errors="ignore") as f:
        content = f.read()
        m = get_c_func_re(c_func).search(content)
        if m:
            beginning = m.start()
            all_lines = content.split("\n")
            src = []
            comment = []
            docstring = ""
            cnt = 0
            for idx, line in enumerate(all_lines):
                cnt += len(line) + 1
                if src and line.startswith("}"):
                    src.append(line)
                    return COMMENT_RE.sub("", docstring), idx, "\n".join(src)
                elif src:
                    src.append(line)
                elif cnt > beginning:
                    src = [line]
                elif "*/" in line:
                    docstring = "\n".join(comment)
                    comment = []
                elif comment and line.strip().startswith("*"):
                    comment.append(line)
                elif line.strip().startswith("/*"):
                    comment = [line]
                else:
                    # hit a non-comment line, but not the desired src yet, ignore this one
                    docstring = ""
                    comment = []
    return "", -1, ""


if __name__ == "__main__":
    root = join(dirname(dirname(dirname(abspath(__file__)))), "src")

    with open("index.html", "w") as toc:
        toc.write("""
        <html>
        <head>
        <title>TOC</title>
        <style type=text/css>{0}</style>
        </head>
        <body>
        """.format(CSS))
        for cppfile in walk_files(root, exts=[".cpp"]):
            rule = False
            for lua_register in walk_lua_registers(cppfile):
                if not rule:
                    toc.write("<hr>")
                    toc.write(cppfile.replace(root, ""))
                    toc.write("<hr>")
                    rule = True
                luaname, cname = lua_register
                doc, line, src = extract_docstring(cppfile, cname)
                doc = doc.replace("\n", "<br>")
                src = highlight(src)
                if src:
                    if doc:
                        toc.write("""
                        <details>
                        <summary>{0}</summary>
                        <p>{1}</p>
                          <details>
                          <summary>{2}:{3} {4}</summary>
                          <p>{5}</p>
                          </details>
                        </details>
                        """.format(luaname, doc, cppfile.replace(root, ""), line, cname, src))
                    else:
                        toc.write("""
                        <details>
                        <summary>{0}</summary>
                        <p>{1}:{2} {3}</p>
                        <p>{4}</p>
                        </details>
                        """.format(luaname, cppfile.replace(root, ""), line, cname, src))
                else:
                    toc.write("""
                    <details>
                    <summary>{0}</summary>
                    <p>Lua function implemented as {1}, but didn't find that in {2}</p>
                    </details>
                    """.format(luaname, cname, cppfile.replace(root, "")))

        for pkgfile in walk_files(root, exts=[".pkg"]):
            rule = False
            for lua_register in walk_lua_exports(pkgfile):
                if not rule:
                    toc.write("<hr>")
                    toc.write(pkgfile.replace(root, ""))
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
                    code = code.split("\n")
                    toc.write("""
                    <details>
                    <summary>{0}</summary>
                    <p>{1}</p>
                    <p>{2}</p>
                    </details>
                    """.format(code[0], doc, highlight("\n".join(code))))
        toc.write("""
        </body>
        </html>
        """)
