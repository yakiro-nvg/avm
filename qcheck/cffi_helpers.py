import os, importlib, cffi
from pycparser import c_generator, c_ast, parse_file

class DefGeneratorVisitor(c_ast.NodeVisitor):
    def __init__(self, typedefs):
        self.expected_typedefs = typedefs
        self.typedefs = []
        self.decls = []
        self.gen = c_generator.CGenerator()

    def visit_Typedef(self, node):
        if node.name in self.expected_typedefs:
            self.typedefs.append(self.gen.visit(node) + ';')

    def visit_FuncDef(self, node):
        node.body = None
        node.decl.funcspec = []
        node.decl.storage = []
        self.decls.append(self.gen.visit(node)[:-2] + ';')

PATH = os.path.dirname(__file__)

def to_cdef(filename, typedefs):
    typedefs = [
        'u8', 'u16', 'u32', 'u64', 's8', 's16', 's32', 's64',
        'f32', 'f64', 'abool', 'aresult_t', 'aalloc_t'
    ] + (typedefs or [])
    ast = parse_file(
        filename,
        cpp_path='clang' if os.name == 'nt' else 'gcc',
        cpp_args=[
            '-E', '-DATEST',
            '-I' + os.path.join(PATH, '../externals/pycparser/utils/fake_libc_include'),
            '-I' + os.path.join(PATH, '../src/inc'),
            '-I' + os.path.join(PATH, '../src/private'),
        ],
        use_cpp=True)
    v = DefGeneratorVisitor(typedefs)
    v.visit(ast)
    return '\n'.join(v.typedefs) + '\n' + '\n'.join(v.decls)

def load_ffi(filename, typedefs):
    coverage = os.getenv('AVM_COVERAGE', '0') == '1'
    ffi = cffi.FFI()
    ffi.cdef(to_cdef(filename, typedefs))
    ffi.cdef('void flush_coverage(void);')
    with open(filename, 'r') as f:
        src = '#define AINLINE\n#define ASTATIC\n' + f.read()
        cargs = []; largs = []
        if coverage:
            cargs = ['-fprofile-arcs', '-ftest-coverage']
            largs = ['-fprofile-arcs']
            src += '\nvoid __gcov_flush(void); void flush_coverage(void) { __gcov_flush(); }'
        else:
            src += '\nvoid flush_coverage(void) { }'
        name = '_ffi_' + os.path.splitext(os.path.basename(filename))[0]
        ffi.set_source(name, src,
            include_dirs=[
                os.path.join(PATH, '../src/inc'),
                os.path.join(PATH, '../src/private')],
            extra_compile_args=cargs, extra_link_args=largs)
        ffi.compile()
        m = importlib.import_module(name)
        return m
