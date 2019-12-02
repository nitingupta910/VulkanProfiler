import sys
import os.path
import subprocess

sys.path.append( sys.argv[1] )

# Generated by CMake
from generated import *

if __name__ == '__main__':
    compiled_shaders_header_filename = os.path.relpath( COMPILED_SHADERS_HEADER )
    glslang_validator = os.path.normpath( VULKAN_SPIRV_COMPILER )
    glslang_validator_options = \
        ['-D'] + \
        ['-V'] + \
        ['-e'] + ['main'] + \
        ['--variable-name'] + [COMPILED_SHADERS_HEADER_NAME]
    with open( compiled_shaders_header_filename, 'w' ) as compiled_shaders_file:
        compiled_shaders_file.write(
            '#pragma once\n' +
            '#include <stdint.h>\n' +
            '\n' +
            'namespace Profiler\n' +
            '{\n' +
            '    struct ProfilerShaders\n' +
            '    {' )
        for shader in SHADERS:
            shader_name = os.path.basename( shader[:-5] ).replace( '.', '_' )
            shader_output_filename = os.path.basename( shader + '.compiled.h' )
            glslang_validator_shader_options = \
                glslang_validator_options + \
                ['-o'] + [shader_output_filename]
            subprocess.check_call(
                [glslang_validator] + \
                   glslang_validator_shader_options + \
                   [shader],
                stdout=sys.stdout,
                stderr=sys.stderr )
            with open( shader_output_filename ) as shader_output_file:
                shader_bytecode_found = False
                for line in shader_output_file:
                    line = line.strip()
                    if line.startswith( 'const uint32_t ' + COMPILED_SHADERS_HEADER_NAME ):
                        shader_bytecode_found = True
                        compiled_shaders_file.write( ('\n' +
                            '        inline static const uint32_t {0}[] =\n' +
                            '        {{\n').format( shader_name ) )
                        continue # Assume there is no bytecode in the first line
                    if '}' in line:
                        line = line.split( '}' )[0]
                        if line == '':
                            break # nothing left to write
                    if shader_bytecode_found:
                        compiled_shaders_file.write(
                            '            {0}\n'.format( line ) )
                if shader_bytecode_found:
                    compiled_shaders_file.write( '        };\n' )
        compiled_shaders_file.write(
            '    };\n' +
            '}\n' )