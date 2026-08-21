#include "../vendor/lua/lua.h"
#include "../vendor/lua/lualib.h"
#include "../vendor/lua/lauxlib.h"
#include "headers/datatypes.h"
#include <stdio.h>

PipelineData GlobalData = {0};

// (string shader) -> void
int c_load_shader(lua_State* lua_state) {
    size_t shader_path_length;
    const char* shader_path = luaL_tolstring(lua_state, 1, &shader_path_length);
    int res = 1;

    // ditching windows compatibility...
    FILE* file = fopen(shader_path, "rb");
    if (file == NULL) {
        printf("Got error trying to read shader file \"%s\"\n", shader_path);
        return 0;
    };
    if (fseek(file, 0, SEEK_END) < 0) {
        fclose(file);
        printf("Got error trying to read shader file \"%s\"\n", shader_path);
        return 0;
    };

    usize size = 0;
    size = ftell(file);
    if (fseek(file, 0, SEEK_SET) < 0) {
        fclose(file);
        printf("Got error trying to read shader file \"%s\"\n", shader_path);
        return 0;
    };

    char file_content[size];
    if (ferror(file)) {
        fclose(file);
        printf("Got error trying to read shader file \"%s\"\n", shader_path);
        return 0;
    };

    fread(file_content, size, 1, file);

    fclose(file);
    return 0;
}

void execute_lua_file(char* script_path, Program* program) {
    lua_State* lua_state = luaL_newstate();
    luaL_openlibs(lua_state);

    GlobalData.program = program;

    // script will be executed on reload so it is expected it won't be too heavy
    if (luaL_dofile(lua_state, script_path) != LUA_OK) {
        fprintf(stderr, "Error running script: %s\n", lua_tostring(lua_state, -1));
        lua_pop(lua_state, 1); // Clean up the error message from stack
        lua_close(lua_state);
    }
}

