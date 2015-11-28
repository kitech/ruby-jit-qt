import sys
import os
import clang
import clang.cindex

clang.cindex.Config.set_library_file('/usr/lib/libclang.so')
compile_args = ['-x', 'c++', '-std=c++11', '-D__CODE_GENERATOR__']

def demo_cmdline():
    index = clang.cindex.Index.create()
    translate_unit = index.parse(sys.argv[1], compile_args)
    tu = translate_unit
    print(tu, tu.get_includes(), tu.cursor)
    for inc in tu.get_includes():
        pass
    # print(inc, inc.source, inc.include, inc.location, inc.depth)

    clses = build_classes(tu.cursor)
    print(clses)

# demo_cmdline()

qtmodules = ['QtCore', 'QtGui', 'QtWidgets', 'QtNetwork', 'QtDBus']


def gen_module_macro_include_path():
    prefix = '/usr/include/qt'
    paths = []
    for m in qtmodules:
        paths.append(prefix + '/' + m + '/' + m)
    return paths


def build_classes(cursor):
    class_names = {}  #

    for c in cursor.get_children():
        # print(c.kind, ',', c.spelling)
        if c.kind == clang.cindex.CursorKind.CLASS_DECL:
            name = c.spelling
            if name.endswith('Private'):
                # print('Omited private internal class definition:' + name)
                continue
            if name.endswith('Ref'):
                # print('Omited private internal class definition:' + name)
                continue

            # print(c.kind, ',', c.spelling)
            method_names = build_methods(c)
            if c.spelling in class_names:
                if len(class_names[c.spelling]) < len(method_names):
                    class_names[c.spelling] = method_names
            else:
                class_names[c.spelling] = method_names

    return class_names


def build_methods(cursor):
    method_names = {}

    for m in cursor.get_children():
        print(m.kind, ',', m.spelling)
        if m.kind == clang.cindex.CursorKind.CXX_METHOD:
            method_names[m.spelling] = m

    return method_names


def build_golang_type_for_class(mod, cls, methods):
    code = "package core\n\n"

    # import
    code += "import \"unsafe\"\n"
    code += "\n"

    # init
    code += "func init(){}\n\n"

    # type
    code += "type " + cls + " struct {\n"
    code += "    o *unsafe.Pointer\n"
    code += "}\n\n"

    # new
    code += "func New" + cls + "() *" + cls + "{\n"
    code += "    var o *unsafe.Pointer = nil\n"
    code += "    return &" + cls + "{o: o}\n"
    code += "}\n\n"

    # method missing
    code += "func (this *" + cls + ") method_missing(mth string, args ...interface{}) interface{} {\n"
    code += "    return 0\n"
    code += "}\n\n"

    def title_method_name(name):
        newname = name[0].upper() + name[1:]
        return newname

    for mth in methods:
        cursor = methods[mth]
        if mth.startswith('operator'):
            # print("Omited operator method: " + mth)
            continue
        ispub = True
        print('pub:' + str(cursor.access_specifier))
        if cursor.access_specifier == clang.cindex.AccessSpecifier.PUBLIC:
            ispub = True
        else: ispub = False

        timth = title_method_name(mth) if ispub else mth
        timth = (timth + '_') if timth in ['type', 'select'] else timth
        # code += "/*\n"
        code += '' if cursor.raw_comment is None else (cursor.raw_comment + "\n")
        # code += "*/\n"
        code += "func (this *" + cls + ") " + timth + "(args ...interface{}) interface{} {\n"
        code += "    return this.method_missing(\"" + mth + "\", args...)\n"
        code += "    // return 0\n"
        code += "}\n"
        code += "\n"

    return code


def write_golang_class_code(mod, cls, code):
    fpath = "src/qt/core/" + cls.lower() + ".go"
    f = os.open(fpath, os.O_CREAT | os.O_TRUNC | os.O_RDWR)
    os.write(f, code)
    return


for minc in gen_module_macro_include_path():
    print('Parsing... ' + minc)
    index = clang.cindex.Index.create()
    tu = index.parse(minc, compile_args)
    print('Parse done. Walking through...')

    class_names = build_classes(tu.cursor)
    print('class count:' + str(len(class_names)))

    cnter = 0
    for cls in class_names:
        method_names = class_names[cls]
        print(cls + '\'s Method count:' + str(len(method_names)) + ", " + str(method_names.keys()))
        code = build_golang_type_for_class('QtCore', cls, method_names)
        print(code)
        print('Writing golang source file...')
        write_golang_class_code('QtCore', cls, code)

        cnter += 1
        if cnter > 10000:
            break  # class loop

    break  # module loop
