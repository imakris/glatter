license = '''
Copyright 2017 Ioannis Makris

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''


from unittest import case
input_root = r'.\input_headers'
output_root = r'.'

families = {'GL':'gl', 'GLX':'glX', 'EGL':'egl', 'GLU':'glu', 'WGL':'wgl', 'khronos_':'khronos_'}

extension_groups = {key: {} for key in families}

extension_groups['GL'] = {
    '3DFX':0, '3DL':0, 'AMD':0, 'ANDROID':0, 'ANGLE':0, 'APPLE':0, 'ARB':0, 'ARM':0,
    'ATI':0, 'DMP':0, 'EXT':0, 'FJ':0, 'GREMEDY':0, 'HP':0, 'I3D':0, 'IBM':0, 'IGLOO':0,
    'IMG':0, 'INGR':0, 'INTEL':0, 'KHR':0, 'MESA':0, 'MESAX':0, 'NV':0, 'NVX':0, 'OES':0,
    'OML':0, 'OVR':0, 'PGI':0, 'QCOM':0, 'REND':0, 'S3':0, 'SGI':0, 'SGIS':0, 'SGIX':0,
    'SUN':0, 'SUNX':0, 'VIV':0, 'WIN':0, '':0
}

extension_groups['EGL'] = {
    'ANDROID':0, 'ANGLE':0, 'ARM':0, 'EXT':0, 'HI':0, 'IMG':0, 'KHR':0, 'MESA':0, 'NOK':0,
    'NV':0, 'TIZEN':0, '':0
}

all_extgroups = {}
for i in families:
    all_extgroups.update(extension_groups[i])

# containers populated during parsing phase
enum_to_string = {}
function_definitions = {key: set() for key in families}
typedefs = {}

known_fnames = {}

import os
import sys
import re
import operator
import string
import copy
import itertools

ckwords = ['auto', 'break', 'case', 'char', 'const', 'continue', 'default', 'do', 'double', 'else',
    'enum', 'extern', 'float', 'for', 'goto', 'if', 'inline', 'int', 'long', 'register', 'restrict',
    'return', 'short', 'signed', 'sizeof', 'static', 'struct', 'switch', 'typedef', 'union',
    'unsigned', 'void', 'volatile', 'while']

printable_c_types =  {
	'char': '%d',             #
	'signed char': '%d',      # printable character semantics are unlikely, thus printing as int
	'unsigned char': '%u',    #
	'short': '%hi',
	'short int': '%hi',
	'signed short': '%hi',
	'signed short int': '%hi',
	'unsigned short': '%hu',
	'unsigned short int': '%hu',
	'int': '%d',
	'signed': '%d',
	'signed int	': '%d',
	'unsigned': '%u',
	'unsigned int': '%u',
	'long': '%li',
	'long int': '%li',
	'signed long': '%li',
	'signed long int': '%li',
	'unsigned long': '%lu',
	'unsigned long int': '%lu',
	'long long': '%lli',
	'long long int': '%lli',
	'signed long long': '%lli',
	'signed long long int': '%lli',
	'unsigned long long': '%llu',
	'unsigned long long int': '%llu',
	'float': '%f',
	'double': '%f',
	'int8_t': '%"PRId8"',
	'int16_t': '%"PRId16"',
	'int32_t': '%"PRId32"',
	'int64_t': '%"PRId64"',
	'int_fast8_t': '%"PRIdFAST8"',
	'int_fast16_t': '%"PRIdFAST16"',
	'int_fast32_t': '%"PRIdFAST32"',
	'int_fast64_t': '%"PRIdFAST64"',
	'int_least8_t': '%"PRIdLEAST8"',
	'int_least16_t': '%"PRIdLEAST16"',
	'int_least32_t': '%"PRIdLEAST32"',
	'int_least64_t': '%"PRIdLEAST64"',
	'uint8_t': '%"PRIu8"',
	'uint16_t': '%"PRIu16"',
	'uint32_t': '%"PRIu32"',
	'uint64_t': '%"PRIu64"',
	'uint_fast8_t': '%"PRIuFAST8"',
	'uint_fast16_t': '%"PRIuFAST16"',
	'uint_fast32_t': '%"PRIuFAST32"',
	'uint_fast64_t': '%"PRIuFAST64"',
	'uint_least8_t': '%"PRIuLEAST8"',
	'uint_least16_t': '%"PRIuLEAST16"',
	'uint_least32_t': '%"PRIuLEAST32"',
	'uint_least64_t': '%"PRIuLEAST64"',
	'intptr_t': '%"PRIxPTR"',
	'uintptr_t': '%"PRIxPTR"',
	'size_t': '%zu',
	'wchar_t': '%lc',
	'ptrdiff_t': '%td'
}

def get_input_files():
    input_files = []
    for dir_name, subdir_list, file_list in os.walk(input_root):
        for file_name in file_list:
            if (re.match("(.)*\.h$", file_name)):
                input_files.append((dir_name + '/' + file_name).replace('\\', '/'))
    return input_files

comment_pattern = re.compile(
    r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
    re.DOTALL | re.MULTILINE
)

fm_sbp = '(?P<family>'+ '|'.join(families) + ')'
fm_sbl = '(?P<fprefix>'+ '|'.join(families.values()) + ')'
familyenum = re.compile(fm_sbp + 'enum$')
validenum_pattern = re.compile('\w*_BIT[0-9S]?(_[0-9A-Z]+)?$')
condblock_define_pattern = re.compile('^# ?define (?P<dname>'+fm_sbp+'_\w*) 1')
condblock_ifndef_pattern = re.compile('^# ?ifndef (?P<dname>'+fm_sbp+'_\w*)')
headerversion_pattern = re.compile(r'[A-Z0-9]+_VERSION_[0-9]{1,2}_[0-9]{1,2}')
endif_pattern = re.compile('^ ?# ?endif')
condblock_any_ifstar = re.compile('^# ?if')
condblock_any_ifndef = re.compile('^# ?ifndef (?P<dname>\w+)')

enum_pattern = re.compile('^# ?define ('+fm_sbp+'_\w*) ?(\w*)$')
function_coarse_pattern = re.compile(r'(.*?) +('+fm_sbl+'[A-Z]\w+?) ?\( ?(.*?) ?\) ?;')
cconv_p = 'CALLBACK|WINAPIV?|APIENTRY|APIPRIVATE|PASCAL|CDECL|_cdecl|__cdecl|__stdcall|__pascal'
function_fine_pattern = re.compile(r'^((?P<expkw>extern|([A-Z0-9_]*API(CALL)?)?) +)? ?(?P<rt>[\w* ]*?) ?(?P<cconv>\w*('+cconv_p+'))?$')
function_group_pattern = re.compile(r'\w*[a-z]+(?P<group>[A-Z0-9]{2,10})$')
typedef_pattern = re.compile(r'^typedef(?P<type>.+?)(?P<name>'+fm_sbp+'\w+);$');


def comment_remover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " " # note: a space and not an empty string
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)

class Typedef:
    def __init__(self):
        self.name = None
        self.type = None
        self.is_pointer = None
        self.converter = None

class Function_argument:
    def __init__(self):
        self.declaration = None
        self.type = None
        self.name = None
        #self.name_range = None
        self.is_pointer = False
    def __eq__(self, other): 
        return self.type == other.type

    def get_printf_faa(self): ## faa = format and args
        mm = re.match(familyenum, self.type)
        # 1. is api-enum
        if (bool(mm)):
            return ['%s', 'enum_to_string_' + mm.group('family') + '(' + self.name + ')']
        if self.is_pointer:
            return ['%p', '(void*)'+self.name]

        argtype = self.type
        while argtype in typedefs:
            argtype = typedefs[argtype]

        if '*' in argtype:
            return ['%p', '(void*)'+self.name]

        if argtype in printable_c_types:
            fmt = printable_c_types[argtype]
            return [fmt, '('+argtype+')' + self.name]

        return ['%s', 'GET_PRS('+self.name+')']

class Function_declaration:

    def __init__(self):
        self.rtype = None
        self.expkw = None
        self.cconv = None
        self.name = None
        self.extension_group = None
        self.args = None
        self.family = None
        self.proto = None
        self.block = []
        self.occurence = None

    def __hash__(self):
        return hash((self.name, self.block[0]))

    def __repr__(self):
        return "Function_declaration()"
    def __str__(self):
        return self.expkw + ' ' + self.rtype + ' ' + self.cconv + ' ' + self.name + '(' + ');' + ' // ' + self.extension_group
    def __eq__(self, other): 
        return\
        self.name == other.name and\
        self.rtype == other.rtype and\
        self.expkw == other.expkw and\
        self.cconv == other.cconv and\
        self.extension_group == other.extension_group and\
        self.args == other.args and\
        self.family == other.family and\
        self.proto == other.proto
        #self.block == other.block <- not this one

    def __ne__(self, other):
        # Not strictly necessary, but to avoid having both x==y and x!=y
        # True at the same time
        return not(self == other)

def validate_extension(ext_str):
    if ext_str in all_extgroups:
        return ext_str
    else:
        es_trimmed = ext_str[1:]
        while (len(es_trimmed) >= 2):
            if es_trimmed in all_extgroups:
                return es_trimmed
            es_trimmed = es_trimmed[1:]
    return ''

def validate_enum(enum_str):
    return enum_str if not bool(re.match(validenum_pattern, enum_str)) else ''


#def hcr(h):
#    return h if h not in header_conflict_resolution else header_conflict_resolution[h]

def analyze_condition(ppline, former_condition = None):
    #if
    m = re.match('^# ?if (?P<condition>.+)', ppline)
    if (bool(m)):
        return '('+m.group('condition')+')'
    #ifdef
    m = re.match('^# ?ifdef (?P<dname>\w+)$', ppline)
    if (bool(m)):
        return 'defined('+m.group('dname')+')'
    m = re.match('^# ?ifndef (?P<dname>\w+)$', ppline)
    if (bool(m)):
        return '!defined('+m.group('dname')+')'
    #else
    m = re.match('^# ?else', ppline)
    if (bool(m)):
        return '!('+former_condition+')'
    #elif
    m = re.match('^# ?elif (?P<condition>.+)', ppline)
    if (bool(m)):
        return '(('+m.group('condition')+') && !('+former_condition+'))'
    

def copystack(st):
    st = copy.deepcopy(st)
    removers = []

    #flatten
    f_st = []
    for x in st:
        for y in x:
            if (y[0] != '-'):
                if '_PROTOTYPES' not in y and 'WINAPI_FAMILY' not in y:
                    f_st.append(y)
            else:
                removers.append(y)

    for x in removers:
        for i, y in enumerate(f_st):
            if y == '!'+x[1:]:
                f_st[i] = x[1:]

    #remove multiples and _PROTOTYPES
    r_st = []
    for x in f_st:
      if x not in r_st:
        r_st.append(x)

    return r_st


def parse(filename):
    f = open(filename, 'r')
    fstr = f.read()

    #===================#
    #   preprocessing   #
    #===================#

    # remove split lines and comments
    c0 = re.sub(r'\\\n','',fstr, re.S | re.M)
    c1 = comment_remover(c0)

    # collapse multiple spaces and tabs to single spaces, remove trailing and leading spaces
    c2 = list(map(lambda x: x.strip(), re.sub(r'[ \t]+', ' ', c1).splitlines()))

    c3 = []
    tmp = ''
    mode = 0

    # merge multiline statements
    st_buffer = ''
    for v in c2:
        if len(v) > 0:
            if v[0] == '#':
                c3.append(v)
            else:
                st_buffer += v
                if v[-1] in {';', '{', '}'}:
                    c3.append(st_buffer)
                    st_buffer = ''

    c4 = []

    #========================#
    #   conditional blocks   #
    #========================#

    indstack = []
    for i, v in enumerate(c3):

        c4.append([i, copystack(indstack), v])
        
        if (bool(re.match(endif_pattern, v))):
            if (len(indstack) != 0):
                indstack = indstack[:-1]

        elif (bool(re.match(condblock_any_ifstar, v))):
            indstack.append([analyze_condition(v)])

        elif (bool(re.match('^(# ?else)|(# ?elif)', v))):
            indstack[-1] = [analyze_condition(v, indstack[-1][0])]

        elif (bool(re.match('^# ?define ', v))):
            m = re.match('^# ?define (?P<what>\w+)( (?P<as>\w+)$)?', v)
            for k in indstack:
                if k[0] == '!defined('+m.group('what')+')':
                    indstack[-1].append('-defined('+m.group('what')+')')
                    if (m.group('as') != None and m.group('as') != '1'):
                        indstack[-1].append('('+m.group('what')+'=='+m.group('as')+')')

    #===========#
    #   ENUMS   #
    #===========#

    for d in c4:
        m = re.match(enum_pattern, d[2])
        if (bool(m) and validate_enum(m.group(1)) != ''):
            try:
                value = int(m.group(3), 0)
                
                if (value >= 0x100 and value < 0x20000):
                    name = m.group(1)
                    family = m.group(2)
                    eblock = d[1][-1] #if d[1] != None else hcr(header_guard)
                    
                    if (family not in enum_to_string):
                        enum_to_string[family] = {}
                    if (value not in enum_to_string[family]):
                        enum_to_string[family][value] = {}
                    if (eblock not in enum_to_string[family][value]):
                        enum_to_string[family][value][eblock] = set()
                    enum_to_string[family][value][eblock].add(name)

                    continue

            except ValueError: #integer conversion failed
                # not sure if it makes any sense to implement anything here
                pass

            continue

    
    #===============#
    #   TYPEDEFS    #
    #===============#

        m = re.match(typedef_pattern, d[2])
        if (bool(m)):
            typedefs[m.group('name')] = m.group('type').strip()
            continue

    #===============#
    #   FUNCTIONS   #
    #===============#

        # this is a coarse match
        m = re.match(function_coarse_pattern, d[2])

        if (bool(m)):

            # and this is a finer match
            rt_match = re.match(function_fine_pattern, m.group(1))
            
            if (not bool(rt_match)):
                raise

            tmp = Function_declaration()

            tmp.proto = d[2]
            tmp.rtype = rt_match.group('rt')

            tmp.expkw = rt_match.group('expkw')
            if (tmp.expkw == None):
                tmp.expkw = ''
            tmp.cconv = rt_match.group('cconv')
            if (tmp.cconv == None):
                tmp.cconv = ''

            tmp.name = m.group(2)
            name_upper = tmp.name.upper()
            tmp.family = m.group('fprefix').upper()
            family_upper = tmp.family
            tmp.block = d[1] # if d[1] != None else ''

            group_match = re.match(function_group_pattern, tmp.name)
            group = group_match.group('group') if group_match != None else ''

            egroup = validate_extension(group)
            all_extgroups[egroup] += 1
            tmp.extension_group = egroup

            arglist_coarse = m.group(4).split(",")
            arglist_fine = []
            for i, y in enumerate(arglist_coarse):
                arg = Function_argument()
                arg.declaration = y.strip()
                if arg.declaration in ['void', 'VOID']:
                    continue
                arg.is_pointer = '*' in arg.declaration

                # place lindex, rindex at the beginning and at the end of the string accordingly.
                lindex = 0
                rindex = len(arg.declaration)

                # place rindex before the first closing parenthesis
                if ')' in arg.declaration:
                    rindex = arg.declaration.index(')')

                # place rindex before the first opening angle bracket
                if '[' in arg.declaration:
                    rindex = arg.declaration.index('[')

                # place lindex after the last *
                if '*' in arg.declaration:
                    lindex = arg.declaration.rindex('*')

                s1 = arg.declaration[lindex:rindex]
                s1 = s1.rstrip()

                for mm in re.finditer(r'([A-Za-z]+\w*)', s1):
                    pass
                lindex += mm.start()

                arg.name = arg.declaration[lindex:rindex]
                
                arg.type = 'UNKNOWN TYPE'
                if (len(arg.declaration) == rindex):
                    arg.type = arg.declaration[0:lindex].strip()

                #if the argument has no name, we assign a name to it
                if rindex - lindex < 1:
                    arg.name = 'a'+str(i)
                    dfinal = arg.declaration[:lindex] + arg.name + arg.declaration[rindex:]
                    rindex += len(arg.name)
                    arg.declaration = dfinal

                #arg.name_range = (lindex, rindex)
                arglist_fine.append(arg)

            tmp.args = arglist_fine
            
            if tmp.name in known_fnames:
                known_fnames[tmp.name] += 1
            else:
                known_fnames[tmp.name] = 1
            tmp.occurence = known_fnames[tmp.name]

            function_definitions[family_upper].add(copy.deepcopy(tmp))


#================================================#
# MAIN                                           #
#================================================#

print('GLATTER v0.1\n')
print('Working Directory:', os.getcwd())

# COLLECT INPUT FILES
input_files = get_input_files()
print('\n')
if (input_files != []):
    print('The following files will be used as input: \n')
    for s in input_files:
        print(s)
else:
    print('No headers found to be used as input.')
    exit()
print("\n")

# PARSE
for s in input_files:
    parse(s)

# GENERATE OUTPUT FILES
if not os.path.exists(output_root):
    os.makedirs(output_root)

h_file = open(output_root + '/glatter.h_gen', 'w')
c_file = open(output_root + '/glatter.c_gen', 'w')
original_stdout = sys.stdout


def get_args_string(a, mode, trailing_comma=True):
    rv = ''
    if (mode == 1):
        for x in a:
            rv += x.declaration + ', '
        return rv if trailing_comma else rv[:-2]
    elif (mode == 2):
        for x in a:
            rv += x.name + ', '
        return rv if trailing_comma else rv[:-2]
    elif (mode == 3):
        for x in a:
            rv += '(' + x.name + '), '
        return rv if trailing_comma else rv[:-2]
    elif (mode == 4):
        for x in a:
            rv += ' << ' + x.name + ' << ", "'
        return rv if trailing_comma else rv[:-8]
    elif (mode == 6):
        rv = ['', '']
        for x in a:
            ar = x.name
            mm = re.match(r'^(?P<enumfam>GL|EGL|GLX|WGL|GLU)enum\s+\w+$', x.declaration)
            if (bool(mm)):
                rv[0] += '%s, '
                rv[1] += 'enum_to_string_' + mm.group('enumfam') + '(' + x.name + '), '
            else:
                pf = x.get_printf_faa()
                rv[0] += pf[0] + ', '
                rv[1] += pf[1] + ', '
        return rv if trailing_comma else [rv[0][:-2], rv[1][:-2]]

    else:
        raise


#================================================#
# HEADER                                         #
#================================================#

sys.stdout = h_file
print('/*' + license + r'''*/

// This file was generated by glatter.py script.

''')

def get_function_mdnd(family): #macros, declarations and definitions
    #file buffers
    header_part = source_part = notes = ''
    header_d = header_r = source_c = ''

    if (len(function_definitions[family]) == 0):
        return ['', '', '']

    sfd0 = sorted(function_definitions[family], key=lambda x: tuple(x.block) + tuple([x.name]) )

    # known headers with conflicts
    for i, v in enumerate(sfd0):
        vbs = set(v.block)
        if ('defined(__gl_h_)' in vbs and 'defined(__GL_H__)' not in vbs):
            sfd0[i].block = sfd0[i].block[:1] + ['!defined(__GL_H__)'] + sfd0[i].block[1:]
        if ('defined(__glu_h_)' in vbs and 'defined(__GLU_H__)' not in vbs):
            sfd0[i].block = sfd0[i].block[:1] + ['!defined(__GLU_H__)'] + sfd0[i].block[1:]
 
    # an attempt for all the rest
    all_fnames = {}
    ambiguous_fnames = {}
    indices_for_deletion = set()
    for i, v in enumerate(sfd0):
        test_name = v.name
        # test_name = (v.name, v.block[0])
        if test_name not in all_fnames:
            all_fnames[test_name] = i
        else:
            if (test_name not in ambiguous_fnames):
                ambiguous_fnames[test_name] = [all_fnames[test_name]]
            ambiguous_fnames[test_name].append(i)

    for v in ambiguous_fnames:
        perms = itertools.permutations(ambiguous_fnames[v], 2)
        for w in perms:
            s0 = set(sfd0[w[0]].block)
            s1 = set(sfd0[w[1]].block)
            if s0 < s1:
                ds = s1 - s0
                for x in ds:
                    sfd0[w[0]].block.append(('!'+x) if x[0]!='!' else x[1:])
            elif s0==s1:
                indices_for_deletion.add(min(w[0], w[1]))

    sfd = []
    for i, v in enumerate(sfd0):
        if i in indices_for_deletion:
            notes += '''
// Note: a set of definitions for function ''' + v.name + ''' from file with
// header guard ''' + v.block[0][8:-1] + ''' was found to be potentially conflicting,
// thus was omitted.'''
        else:
            sfd.append(v)


    current_block  = sfd[0].block

    tmp = '''
#ifdef GLATTER_''' + family + '''
#if ''' + '\n#if '.join(sfd[0].block)

    header_d, header_r, source_c = tmp, tmp, tmp
    tmp = ''

    for x in sfd:
        #test_block = '\n#if '.join(x.block)
        if current_block !=  x.block:

            endifs = []
            for i, v in enumerate(current_block):
                if (len(x.block) <= i or x.block[i] != v):
                    endifs.append(v)

            for c in reversed(endifs):
                tmp += '''
#endif // ''' + c

            for i, v in enumerate(x.block):
                if (len(current_block) <= i or current_block[i] != v):
                    tmp += '''
#if ''' + v
            current_block = x.block

        if tmp != '':
            header_d += tmp
            header_r += tmp
            source_c += tmp
            tmp = ''

        #function block buffers
        dn_mac = '' #debug name macro
        df_dec = '' #debug function declaration
        rn_mac = '' #release name macro
        pt_tdc = '' #pointer type declaration
        pt_typ = '' #pointer type
        pt_nam = '' #pointer name
        ep_dec = '' #extern pointer declaration

        if_dec = '' #init function declaration
        pt_def = '' #pointer definition and initialization
        if_def = '' #init function definition
        df_def = '' #debug function definition
        if_ifm = '' #init function macro call

        #building blocks
        a1e = '(' + get_args_string(x.args, 1) + 'const char* file, int line)'
        a1s = '(' + get_args_string(x.args, 1, False) + ')'
        a2s = '(' + get_args_string(x.args, 2, False) + ')'
        a3e = '(' + get_args_string(x.args, 3) + '__FILE__, __LINE__)'
        a3s = '(' + get_args_string(x.args, 3, False) + ')'
        a6s = get_args_string(x.args, 6, False)

        #fix for clang
        if a1s == '()':
            a1s = '(void)'

        dn_mac = '''
#define ''' + x.name + a2s + ' glatter_' + x.name + '_debug' + a3e

        df_dec = '\n' + x.rtype + ' glatter_' + x.name + '_debug' + a1e + ';'

        pt_nam = 'glatter_' + x.name + '_ptr'

        rn_mac = '''
#define ''' + x.name + a2s + ' ' + pt_nam + a3s

        pt_typ = 'glatter_' + x.name + '_t'

        pt_tdc = '''
typedef ''' + x.rtype + ' (' + x.cconv + ' *' + pt_typ + ')' + a1s + ';'

        ep_dec = '''
extern ''' + pt_typ + ' ' + pt_nam + ';'

        cconv_text = ''
        if (x.cconv != ''):
            cconv_text = ' ' + x.cconv
        if_dec = '\n' + x.rtype + cconv_text + ' glatter_' + x.name + '_init' + a1s + ';'

        pt_def = '''
''' + pt_typ + ' ' + pt_nam + ' = glatter_' + x.name + '_init' + ';'

        if_def = if_dec[:-1] + '''
{
    ''' + pt_nam + ' = (' + pt_typ +') glatter_get_proc_address_' + x.family + '("' + x.name + '''");
    return ''' + pt_nam + a2s + ''';
}'''

        printf_va_args = ', "\\n"'
        if len(x.args) != 0:
            printf_va_args += ', ' + a6s[1]
        df_def = df_dec[:-1] + '''
{
    GLATTER_DBLOCK(file, line, ''' + x.name + ', (' + a6s[0] + ')' + printf_va_args + ')'
        if (x.rtype not in ['void', 'VOID']):
            rarg = Function_argument()
            rarg.name = 'rval'
            rarg.type = x.rtype
            rarg.is_pointer = '*' in x.rtype
            pf = rarg.get_printf_faa()
            df_def += '''
    ''' + x.rtype + ''' rval = ''' + pt_nam + a2s + ''';
    printf("GLATTER: returned ''' + pf[0] + '", ' + pf[1] + ');'

        else:
            df_def += '''
    ''' + pt_nam + a2s + ''';'''
        df_def += '''
    glatter_check_error_'''+ x.family +'''(file, line);'''
        if (x.rtype not in ['void', 'VOID']):
            df_def += '''
    return rval;'''
        df_def += '''
}'''
        return_or_not = ''
        if x.rtype not in ['void', 'VOID']:
            return_or_not = 'return'

        if known_fnames[x.name] > 1:
            if_ifm += '''
#ifndef ''' +  x.name + '_defined'
        if_ifm += '\nGLATTER_FBLOCK(' +return_or_not+ ', '+ x.family + ', ' + x.expkw + ', ' + x.rtype + ', ' + x.cconv + ', ' + x.name + ', ' + a2s + ', '+ a1s + ')'
        if_ifm += df_def
        if_ifm += '''
#define ''' +  x.name + '_defined'
        if known_fnames[x.name] > 1:
            if_ifm += '''
#endif'''
        ublock = '\nGLATTER_UBLOCK(' + x.rtype + ', ' + x.cconv + ', ' + x.name + ', '+ a1s + ')'

        header_d += dn_mac + df_dec
        header_r += rn_mac + ublock
        source_c += if_ifm

    for v in current_block:
        tmp += '''
#endif // ''' + v

    tmp += '''
#endif // GLATTER_''' + family + '\n'
    header_d += tmp
    header_r += tmp
    source_c += tmp

    header_part += '''
#ifdef NDEBUG
''' + header_r + '''
#else // NDEBUG
''' + header_d + '''
#endif // NDEBUG
'''

    source_part += '''
''' + source_c + '''
'''

    return [header_part, source_part, notes]

mndn_gl  = get_function_mdnd('GL' )
mndn_glu = get_function_mdnd('GLU')
mndn_glx = get_function_mdnd('GLX')
mndn_wgl = get_function_mdnd('WGL')
mndn_egl = get_function_mdnd('EGL')

print(mndn_gl[2] + mndn_glu[2] + mndn_glx[2] + mndn_wgl[2] + mndn_egl[2])

if bool(mndn_gl ): print(mndn_gl [0])
if bool(mndn_glu): print(mndn_glu[0])
if bool(mndn_glx): print(mndn_glx[0])
if bool(mndn_wgl): print(mndn_wgl[0])
if bool(mndn_egl): print(mndn_egl[0])

h_file.close()


#================================================#
# SOURCE                                         #
#================================================#

def print_enum_to_string(family, fallback):
    print('''
const char* enum_to_string_''', family  ,'''(GLenum e)
{
    switch (e) {''', sep='')
    sorted_ets = sorted(enum_to_string[family].items(), key=lambda x: x[0] )

    last_ifb = ''
    block_is_open = False

    for x in sorted_ets:
        inv_d = {}
        for y in x[1]:
            if y == '':
                # TODO: print a warning message, that the enum was not classified, thus omitted
                print('this is a test - remove this print')
                continue
            for z in x[1][y]:
                if (not z in inv_d):
                    inv_d[z] = []
                inv_d[z].append(y)
        if len(inv_d) == 1:
            ifb = '''\
#if ''' + ' || '.join(map(str, next(iter(inv_d.values()))))
            if ifb != last_ifb:
                if block_is_open:
                    print('#endif', sep='')
                print(ifb, sep='')
                last_ifb = ifb
            block_is_open = True
            print('''\
        case ''', hex(x[0]),''': return "''', next(iter(inv_d)), '''";''', sep='')

        else:
            last_ifb = ''
            if block_is_open:
                print('''\
#endif''')
                block_is_open = False
            print('''\
        case ''', hex(x[0]),':', sep='')

            for z in sorted(inv_d.items()):
                print('''\
#if ''' + ' || '.join(map(str, z[1])) + '''
                    return "''', z[0], '''";
#endif''', sep = '')

            # in case there is nothing under the case, break, to go to fallback
            print('''\
            break;''', sep='')
    if block_is_open:
        print('''\
#endif''')

    print('''\
    }
    return ''', fallback, ''';
}
''', sep='')


sys.stdout = c_file
print('/*' + license + r'''*/

// This file was generated by glatter.py script.

''')

print_enum_to_string('GL',  '"<UNKNOWN ENUM>"')
print_enum_to_string('GLU', 'enum_to_string_GL(e)')
print_enum_to_string('WGL', 'enum_to_string_GL(e)')
print_enum_to_string('GLX', 'enum_to_string_GL(e)')
print_enum_to_string('EGL', 'enum_to_string_GL(e)')

if bool(mndn_gl ): print(mndn_gl [1])
if bool(mndn_glu): print(mndn_glu[1])
if bool(mndn_wgl): print(mndn_wgl[1])
if bool(mndn_glx): print(mndn_glx[1])
if bool(mndn_egl): print(mndn_egl[1])

c_file.close()

sys.stdout = original_stdout
