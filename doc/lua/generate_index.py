import os
import re

from io import StringIO
from html.parser import HTMLParser


try:
    import pygments
    from pygments.lexers import CppLexer
    from pygments.formatters import HtmlFormatter
    def highlight(src):
        return pygments.highlight(src, CppLexer(), HtmlFormatter())
    CSS = HtmlFormatter().get_style_defs()
    code_CSS = "div.example { width: 50%; background-color: lightgrey; border: 1px solid; padding-left: 10px;}"
except:
    print("Pygments is not installed, no syntax highlighting")
    def highlight(src):
        return src.replace("\n", "<br>").replace(" ", "&nbsp;").replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;")
    CSS,code_CSS = "",""

from os.path import join, dirname, basename, abspath


REGISTER_RE = re.compile(r'lua_register\([a-zA-Z0-9_]+, "([^"]+)", ([^\)]+)\)');
END_OF_DOC_RE = re.compile(r'\*/\s*$')
C_COMMENT_RE = re.compile(r'/\*[^*]*\*+(?:[^/*][^*]*\*+)*/', re.MULTILINE);
BLANK_RE = re.compile(r'^\s*$')
COMMENT_RE = re.compile(r'^\s*/?\*+/?', re.MULTILINE)
PKG_EXPORT_RE = re.compile(r'^(?:extern|static|virtual|tolua_property)? *((?:unsigned|const)? *[a-zA-Z0-9_<>]+[ \*]+[a-zA-Z0-9_\[\]\*]+(?:\([^)]*\))?)(?: const)?;')


class MLStripper(HTMLParser):
    def __init__(self):
        super().__init__()
        self.reset()
        self.strict = False
        self.convert_charrefs= True
        self.text = StringIO()
    def handle_data(self, d):
        self.text.write(d)
    def get_data(self):
        return self.text.getvalue()


def strip_tags(html):
    s = MLStripper()
    s.feed(html)
    return s.get_data()


def get_c_func_re(c_func):
    return re.compile(r'int\s+' + c_func + r'[^a-zA-Z0-9_][^;]+$', re.MULTILINE)


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


PLUGIN_PREAMBLE = r"""
-- Start copy from spec/lua.lua 
local unpack = table.unpack or unpack
local funcdef = "([A-Za-z_][A-Za-z0-9_%.%:]*)%s*"
local decindent = {
  ['else'] = true, ['elseif'] = true, ['until'] = true, ['end'] = true}
local incindent = {
  ['else'] = true, ['elseif'] = true, ['for'] = true, ['do'] = true,
  ['if'] = true, ['repeat'] = true, ['while'] = true}
local function isfndef(str)
  local l
  local s,e,cap,par = string.find(str, "function%s+" .. funcdef .. "(%(.-%))")
  -- try to match without brackets now, but only at the beginning of the line
  if (not s) then
    s,e,cap = string.find(str, "^%s*function%s+" .. funcdef)
  end
  -- try to match "foo = function()"
  if (not s) then
    s,e,cap,par = string.find(str, funcdef .. "=%s*function%s*(%(.-%))")
  end
  if (s) then
    l = string.find(string.sub(str,1,s-1),"local%s+$")
    cap = cap .. " " .. (par or "(?)")
  end
  return s,e,cap,l
end
local q = EscapeMagic

local PARSE = require 'lua_parser_loose'
local LEX = require 'lua_lexer_loose'

local function ldoc(tx, typepatt)
  local varname = "([%w_]+)"
  -- <type> == ?string, ?|T1|T2
  -- anything that follows optional "|..." is ignored
  local typename = "%??"..typepatt
  -- @tparam[...] <type> <paramname>
  -- @param[type=<type>] <paramname>
  -- @string[...] <paramname>; not handled
  local t, v = tx:match("--%s*@tparam%b[]%s+"..typename.."%s+"..varname)
  if not t then -- try without optional [...] part
    t, v = tx:match("--%s*@tparam%s+"..typename.."%s+"..varname)
  end
  if not t then
    t, v = tx:match("--%s*@param%s*%[type="..typename.."%]%s+"..varname)
  end
  return t, v
end

local mapspec = {
  exts = {"sms", "smp"},
  lexer = wxstc.wxSTC_LEX_LUA,
  apitype = "lua",
  linecomment = "--",
  sep = ".:",
  isdecindent = function(str)
    str = (str
      :gsub('%[=*%[.-%]=*%]','') -- remove long strings
      :gsub("%b[]","") -- remove all table indexes
      :gsub('%[=*%[.*',''):gsub('.*%]=*%]','') -- remove partial long strings
      :gsub('%-%-.*','') -- strip comments after strings are processed
      :gsub("%b()","()") -- remove all function calls
    )
    -- this handles three different cases:
    local term = (str:match("^%s*([%w_]+)%s*$")
      or str:match("^%s*(elseif)[%s%(]")
      or str:match("^%s*(until)[%s%(]")
      or str:match("^%s*(else)%f[^%w_]")
    )
    -- (1) 'end', 'elseif', 'else', 'until'
    local match = term and decindent[term]
    -- (2) 'end)', 'end}', 'end,', and 'end;'
    if not term then term, match = str:match("^%s*(end)%s*([%)%}]*)%s*[,;]?") end
    -- endFoo could be captured as well; filter it out
    if term and str:match("^%s*(end)[%w_]") then term = nil end
    -- (3) '},', '};', '),' and ');'
    if not term then match = str:match("^%s*[%)%}]+%s*[,;]?%s*$") end

    return match and 1 or 0, match and term and 1 or 0
  end,
  isincindent = function(str)
    -- remove "long" comments and escaped slashes (to process \' and \" below)
    str = str:gsub('%-%-%[=*%[.-%]=*%]',' '):gsub('\\[\\\'"]','')
    while true do
      local num, sep = nil, str:match("['\"]")
      if not sep then break end
      str, num = str:gsub(sep..".-\\"..sep,sep):gsub(sep..".-"..sep," ")
      if num == 0 then break end
    end
    str = (str
      :gsub('%[=*%[.-%]=*%]',' ') -- remove long strings
      :gsub('%b[]',' ') -- remove all table indexes
      :gsub('%[=*%[.*',''):gsub('.*%]=*%]','') -- remove partial long strings
      :gsub('%-%-.*','') -- strip comments after strings are processed
      :gsub("%b()","()") -- remove all function calls
    )

    local func = (isfndef(str) or str:match("%f[%w_]function%s*%(")) and 1 or 0
    local term = str:match("^%s*([%w_]+)%W*")
    local terminc = term and incindent[term] and 1 or 0
    -- fix 'if' not terminated with 'then'
    -- or 'then' not started with 'if'
    if (term == 'if' or term == 'elseif') and not str:match("%f[%w_]then%f[^%w_]")
    or (term == 'for') and not str:match("%f[%w_]do%f[^%w_]")
    or (term == 'while') and not str:match("%f[%w_]do%f[^%w_]")
    -- or `repeat ... until` are on the same line
    or (term == 'repeat') and str:match("%f[%w_]until%f[^%w_]")
    -- if this is a function definition, then don't increment the level
    or func == 1 then
      terminc = 0
    elseif not (term == 'if' or term == 'elseif') and str:match("%f[%w_]then%f[^%w_]")
    or not (term == 'for') and str:match("%f[%w_]do%f[^%w_]")
    or not (term == 'while') and str:match("%f[%w_]do%f[^%w_]") then
      terminc = 1
    end
    local _, opened = str:gsub("([%{%(])", "%1")
    local _, closed = str:gsub("([%}%)])", "%1")
    -- ended should only be used to negate term and func effects
    local anon = str:match("%f[%w_]function%s*%(.+[^%w_]end%f[^%w_]")
    local ended = (terminc + func > 0) and (str:match("[^%w_]+end%s*$") or anon) and 1 or 0

    return opened - closed + func + terminc - ended
  end,
  marksymbols = function(code, pos, vars)
    local lx = LEX.lexc(code, nil, pos)
    return coroutine.wrap(function()
      local varnext = {}
      PARSE.parse_scope_resolve(lx, function(op, name, lineinfo, vars, nobreak)
        if not(op == 'Id' or op == 'Statement' or op == 'Var'
            or op == 'Function' or op == 'String'
            or op == 'VarNext' or op == 'VarInside' or op == 'VarSelf'
            or op == 'FunctionCall' or op == 'Scope' or op == 'EndScope') then
          return end -- "normal" return; not interested in other events

        -- level needs to be adjusted for VarInside as it comes into scope
        -- only after next block statement
        local at = vars[0] and (vars[0] + (op == 'VarInside' and 1 or 0))
        if op == 'Statement' then
          for _, token in pairs(varnext) do coroutine.yield(unpack(token)) end
          varnext = {}
        elseif op == 'VarNext' or op == 'VarInside' then
          table.insert(varnext, {'Var', name, lineinfo, vars, at, nobreak})
        end

        coroutine.yield(op, name, lineinfo, vars, op == 'Function' and at-1 or at, nobreak)
      end, vars)
    end)
  end,

  typeassigns = function(editor)
    local maxlines = 48 -- scan up to this many lines back
    local iscomment = editor.spec.iscomment
    local assigns = {}
    local endline = editor:GetCurrentLine()-1
    local line = math.max(endline-maxlines, 0)

    while (line <= endline) do
      local ls = editor:PositionFromLine(line)
      local tx = editor:GetLine(line) --= string
      local s = bit.band(editor:GetStyleAt(ls + #tx:match("^%s*") + 2),31)

      -- check for assignments
      local sep = editor.spec.sep
      local varname = "([%w_][%w_"..q(sep:sub(1,1)).."]*)"
      local identifier = "([%w_][%w_"..q(sep).."%s]*)"

      -- special hint
      local typ, var = tx:match("%s*%-%-=%s*"..varname.."%s+"..identifier)
      local ldoctype, ldocvar = ldoc(tx, varname)
      if var and typ then
        assigns[var] = typ:gsub("%s","")
      elseif ldoctype and ldocvar then
        assigns[ldocvar] = ldoctype
      elseif not iscomment[s] then
        -- real assignments
        local var,typ = tx:match("%s*"..identifier.."%s*=%s*([^;]+)")

        var = var and var:gsub("local",""):gsub("%s","")
        -- remove assert() calls as they don't affect their parameter types
        typ = typ and typ:gsub("assert%s*(%b())", function(s) return s:gsub("^%(",""):gsub("%)$","") end)
        -- handle `require` as a special case that returns a type that matches its parameter
        -- (this is without deeper analysis on loaded files and should work most of the time)
        local req = typ and typ:match("^require%s*%(?%s*['\"](.+)['\"]%s*%)?")
        typ = req or typ
        typ = (typ and typ
          :gsub("%b()","")
          :gsub("%b{}","")
          :gsub("%b[]",".0")
          -- replace concatenation with addition to avoid misidentifying types
          :gsub("%.%.+","+")
          -- remove comments; they may be in strings, but that's okay here
          :gsub("%-%-.*",""))
        if (typ and (typ:match(",") or typ:match("%sor%s") or typ:match("%sand%s"))) then
          typ = nil
        end
        typ = typ and typ:gsub("%s","")
        typ = typ and typ:gsub(".+", function(s)
            return (s:find("^'[^']*'$")
              or s:find('^"[^"]*"$')
              or s:find('^%[=*%[.*%]=*%]$')) and 'string' or s
          end)

        -- filter out everything that is not needed
        if typ and typ ~= 'string' -- special value for all strings
        and (not typ:match('^'..identifier..'$') -- not an identifier
          or typ:match('^%d') -- or a number
          or editor.api.tip.keys[typ] -- or a keyword
          ) then
          typ = nil
        end

        if (var and typ) then
          if (assigns[typ] and not req) then
            assigns[var] = assigns[typ]
          else
            if req then assigns[req] = nil end
            assigns[var] = typ
          end
        end
      end
      line = line+1
    end

    return assigns
  end,

  lexerstyleconvert = {
    text = {wxstc.wxSTC_LUA_IDENTIFIER,},

    lexerdef = {wxstc.wxSTC_LUA_DEFAULT,},
    comment = {wxstc.wxSTC_LUA_COMMENT,
      wxstc.wxSTC_LUA_COMMENTLINE,
      wxstc.wxSTC_LUA_COMMENTDOC,},
    stringtxt = {wxstc.wxSTC_LUA_STRING,
      wxstc.wxSTC_LUA_CHARACTER,
      wxstc.wxSTC_LUA_LITERALSTRING,},
    stringeol = {wxstc.wxSTC_LUA_STRINGEOL,},
    preprocessor= {wxstc.wxSTC_LUA_PREPROCESSOR,},
    operator = {wxstc.wxSTC_LUA_OPERATOR,},
    number = {wxstc.wxSTC_LUA_NUMBER,},

    keywords0 = {wxstc.wxSTC_LUA_WORD,},
    keywords1 = {wxstc.wxSTC_LUA_WORD2,},
    keywords2 = {wxstc.wxSTC_LUA_WORD3,},
    keywords3 = {wxstc.wxSTC_LUA_WORD4,},
    keywords4 = {wxstc.wxSTC_LUA_WORD5,},
    keywords5 = {wxstc.wxSTC_LUA_WORD6,},
    keywords6 = {wxstc.wxSTC_LUA_WORD7,},
    keywords7 = {wxstc.wxSTC_LUA_WORD8,},
  },

  keywords = {
    -- keywords
    [[and break do else elseif end for function goto if in local not or repeat return then until while]],

    -- constants/variables
    [[_G _VERSION _ENV false io.stderr io.stdin io.stdout nil math.huge math.pi self true package.cpath package.path]],

    -- core/global functions
    [[assert collectgarbage dofile error getfenv getmetatable ipairs load loadfile loadstring
      module next pairs pcall print rawequal rawget rawlen rawset require
      select setfenv setmetatable tonumber tostring type unpack xpcall]],

    -- library functions
    [[bit32.arshift bit32.band bit32.bnot bit32.bor bit32.btest bit32.bxor bit32.extract
      bit32.lrotate bit32.lshift bit32.replace bit32.rrotate bit32.rshift
      coroutine.create coroutine.resume coroutine.running coroutine.status coroutine.wrap coroutine.yield
      coroutine.isyieldable
      debug.debug debug.getfenv debug.gethook debug.getinfo debug.getlocal
      debug.getmetatable debug.getregistry debug.getupvalue debug.getuservalue debug.setfenv
      debug.sethook debug.setlocal debug.setmetatable debug.setupvalue debug.setuservalue
      debug.traceback debug.upvalueid debug.upvaluejoin
      io.close io.flush io.input io.lines io.open io.output io.popen io.read io.tmpfile io.type io.write
      close flush lines read seek setvbuf write
      math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp
      math.floor math.fmod math.frexp math.ldexp math.log math.log10 math.max math.min math.modf
      math.pow math.rad math.random math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh
      math.type math.tointeger math.maxinteger math.mininteger math.ult
      os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale os.time os.tmpname
      package.loadlib package.searchpath package.seeall package.config
      package.loaded package.loaders package.preload package.searchers
      string.byte string.char string.dump string.find string.format string.gmatch string.gsub string.len
      string.lower string.match string.rep string.reverse string.sub string.upper
      byte find format gmatch gsub len lower match rep reverse sub upper
      table.move, string.pack, string.unpack, string.packsize
      table.concat table.insert table.maxn table.pack table.remove table.sort table.unpack]]
  },
}
-- end copy from spec/lua.lua

local api = {
"""


PLUGIN_END = r"""
}

local interpreter = {
  name = "Stratagus",
  description = "Stratagus engine",
  api = {"baselib", "stratagus"},
  frun = function(self,wfilename,rundebug)
    ide:Print("Run stratagus normally")
  end,
  hasdebugger = true,
}

-- the actual plugin
return {
  name = "Stratagus plugin",
  description = "Stratagus completion and filetypes",
  author = "Tim Felgentreff",
  version = 0.1,

  onRegister = function(self)
    ide:AddAPI("lua", "stratagus", api)
    ide:AddSpec("stratagus-maps", mapspec)
    ide:AddInterpreter("Stratagus", interpreter)
    ide:Print("Stratagus plugin registered")
  end,

  onUnRegister = function(self)
    ide:RemoveInterpreter("Stratagus")
    ide:RemoveSpec("stratagus-maps")
    ide:RemoveAPI("lua", "stratagus")
    ide:Print("Stratagus plugin unloaded")
  end,
}
"""


def c_register_to_api(luaname, doc):
    luadoc = strip_tags(doc.strip().replace('\n', '\\n').replace('"', r'\"'))
    return f'  {luaname} = {{ type = "function", description = "{luadoc}" }},\n'


def pkg_function_to_api(code, doc):
    luaname = code.split("(")[0].strip()
    luaargs = code[len(luaname):]
    if luaargs == "(void)":
        luaargs = "()"
    luaargs = luaargs.replace("const ", "").replace("unsigned ", "").replace(" &", " ").replace(" *", " ").replace('"', r'\"')
    while (m := re.search(r"char ([a-zA-Z_0-9]+)", luaargs)):
        luaargs = luaargs[:m.span(0)[0]] + m.group(1) + ": string" + luaargs[m.span(1)[1]:]
    while (m := re.search(r"int ([a-zA-Z_0-9]+)", luaargs)):
        luaargs = luaargs[:m.span(0)[0]] + m.group(1) + ": number" + luaargs[m.span(1)[1]:]
    while (m := re.search(r"bool ([a-zA-Z_0-9]+)", luaargs)):
        luaargs = luaargs[:m.span(0)[0]] + m.group(1) + ": boolean" + luaargs[m.span(1)[1]:]
    while (m := re.search(r"(?:std::)?string ([a-zA-Z_0-9]+)", luaargs)):
        luaargs = luaargs[:m.span(0)[0]] + m.group(1) + ": string" + luaargs[m.span(1)[1]:]
    while (m := re.search(r"([a-zA-Z_0-9]+) ([a-zA-Z_0-9]+)", luaargs)):
        luaargs = luaargs[:m.span(0)[0]] + m.group(2) + ": " + m.group(1) + luaargs[m.span(2)[1]:]
    luaretval = ""
    if " " in luaname:
        if "*" in luaname:
            luaname = luaname.replace(" *", " * ")
            luaname = luaname.replace(" *", "*")
        luaretval = cpp_type_to_lua_type(luaname.split()[-2])
        luaretval = f', valuetype = "{luaretval}", returns = "({luaretval})"'
        luaname = luaname.split()[-1]
    luaname = luaname.replace("*", "").replace("&", "")
    luadoc = strip_tags(doc.strip().replace('\n', '\\n').replace('"', r'\"'))
    return f'  {luaname} = {{ type = "function", description = "{luadoc}", args = "{luaargs}"{luaretval} }},\n'


def cpp_type_to_lua_type(luatype):
    luatype = re.sub(r"[\*&]", "", luatype)
    luatype = re.sub(r"<[^>]+>", "", luatype)
    if "int" in luatype:
        luatype = "number"
    elif "string" in luatype:
        luatype = "string"
    elif "char*" in luatype.replace(" ", ""):
        luatype = "string"
    elif "char" in luatype:
        luatype = "number"
    elif "bool" in luatype:
        luatype = "boolean"
    elif "short" in luatype:
        luatype = "number"
    elif "long" in luatype:
        luatype = "number"
    elif "double" in luatype:
        luatype = "number"
    elif "float" in luatype:
        luatype = "number"
    return luatype


def pkg_global_to_api(code, doc):
    luatype, luaname = code.split()[-2:]
    luaname = re.sub(r"[\*&]", "", luaname)
    luaname = re.sub(r"\[[A-Za-z0-9_ ]+\]", "", luaname)
    luatype = cpp_type_to_lua_type(luatype)
    return f'  {luaname} = {{ type = "value", valuetype = "{luatype}" }},\n'


def pkg_class_to_api(code, doc, apifile):
    if (m := re.match(r" *class *([A-Za-z0-9_]+)(?: *: *(?:public)? *([A-Za-z0-9_]+))?", code)):
        luaname = m.group(1)
        luadoc = strip_tags(doc.strip().replace('\n', '\\n').replace('"', r'\"'))
        apifile.write(f'  {luaname} = {{ type = "class", description = "{luadoc}",')
        if m.group(2):
            apifile.write(f' inherits = "{m.group(2)}",')
        apifile.write('  childs = {\n')
        for line in code.split("\n")[1:]:
            line = line.strip().replace("static ", "")
            if (m := PKG_EXPORT_RE.search(line)):
                item = m.group(1)
                item = item.replace("void ", "").replace("tolua_readonly", "readonly")
                if "(" in item and item.endswith(")"):
                    apifile.write(f'  {pkg_function_to_api(item, "")}')
                else:
                    apifile.write(f'  {pkg_global_to_api(item, "")}')
            elif (m := re.search(f'^{luaname}\([^\)]*\);', line)):
                # constructor
                item = m.group(0)
                apifile.write(f'  {pkg_function_to_api(f"{luaname} " + item.replace(luaname, "new"), "")}')
                apifile.write(f'  {pkg_function_to_api(f"{luaname} " + item.replace(luaname, "local_new"), "")}')
        apifile.write('  }},\n')
    elif "enum" in code:
        for line in code.split("\n")[1:]:
            if (m := re.search(r"([a-zA-Z0-9_]+)(?: *@ *([a-zA-Z0-9_]+))?", line)):
                apifile.write(f'  {m.group(2) or m.group(1)} = {{ type = "value", valuetype = "number" }},\n')


if __name__ == "__main__":
    root = join(dirname(dirname(dirname(abspath(__file__)))), "src")

    with open("index.html", "w") as toc, open("zerobrane-stratagus-plugin.lua", "w") as api:
        toc.write("""
        <!DOCTYPE html>
        <html lang="en">
        <head>
        <title>TOC</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style type=text/css>{0}
        {1}
        </style>
        </head>
        <body>
        """.format(CSS, code_CSS))
        api.write(PLUGIN_PREAMBLE)
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
                api.write(c_register_to_api(luaname, doc))
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

                    if "(" in code and code.endswith(")"):
                        api.write(pkg_function_to_api(code, doc))
                    else:
                        api.write(pkg_global_to_api(code, doc))
                else:
                    pkg_class_to_api(code, doc, api)
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
        api.write(PLUGIN_END)
