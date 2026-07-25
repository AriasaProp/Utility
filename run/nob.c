#define NOBDEF static
#define NOB_NO_ECHO
// #define NOBDEBUG "-ggdb", 
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <string.h>

#define  BIN_DIR           "bin"
#define  OBJ_DIR   BIN_DIR"/obj"
#define TOOL_DIR  BIN_DIR"/tool"
#define DATA_DIR  BIN_DIR"/data"
#define TEST_DIR  BIN_DIR"/test"
#define BENCHMARK_DIR  BIN_DIR"/benchmark"

#include "config.h"
#include "help.h"

typedef enum {
  ActionFlags_None       =      0,
  ActionFlags_ForceBuild = 1 << 0,
  ActionFlags_DebugRun   = 1 << 1,
} ActionFlags;

typedef struct {
  const char *src;
  File_Paths deps;
} File_Src;

#define COMMON_SRC      "main/common.c"
#define TEST_SRCS       COMMON_SRC, "main/array/dstring.c"
#define BENCHMARK_SRCS  TEST_SRCS, "benchmark/util/profiling.c"
typedef enum {
  Exec_Invalid = 0,
  Exec_QTest,
  Exec_Test,
  Exec_Benchmark,
} Exec_Type;

typedef struct {
  const char *name;
  const char **srcs;
} File_Exe;

static const File_Exe QExecs = {
  .name =  "qtest",
  .srcs = (const char *[]) {
    TEST_SRCS,
    "test/qtest.c",
    NULL
  }
};
static const File_Exe Test_Execs[] = {
  {
    .name = "rand",
    .srcs = (const char *[]) {
      TEST_SRCS,
      "test/math/rand_test.c",
      NULL
    }
  },{
    .name = "complex",
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/math/complex.c",
      "test/math/complex_test.c",
      NULL
    }
  },{
    .name = "matrix",
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/math/matrix.c",
      "test/math/matrix_test.c",
      NULL
    }
  },{
    .name = "bigInteger",
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/math/bigInteger.c",
      "test/math/bigInteger_test.c",
      NULL
    }
  },{
    .name = "sort",
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/algorithm/sort.c",
      "test/algorithm/sort_test.c",
      NULL
    }
  },{
    .name = "hash",
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/algorithm/hash.c",
      "test/algorithm/hash_test.c",
      NULL
    }
  },{
    .name = "image",
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/algorithm/hash.c",
      "main/stb/local.c",
      "main/stb/zlib.c",
      "main/stb/image_read.c",
      "main/stb/image_write.c",
      "test/codec/image_test.c",
      NULL
    }
  },
};
static const File_Exe Benchmark_Execs[] = {
  {
    .name = "bigInteger",
    .srcs = (const char *[]) {
      BENCHMARK_SRCS,
      "main/math/bigInteger.c",
      "benchmark/math/bigInteger_benchmark.c",
      NULL
    }
  },{
    .name = "sort",
    .srcs = (const char *[]) {
      BENCHMARK_SRCS,
      "main/algorithm/sort.c",
      "benchmark/algorithm/sort_benchmark.c",
      NULL
    }
  },
};
#undef COMMON_SRC
#undef TEST_SRCS
#undef BENCHMARK_SRCS

static Cmd cmd;
static Procs procs;
static String_Builder stemp;
static int actionFlags;

typedef struct {
  const char **items;
  size_t count;
  size_t capacity;
} Flags;
typedef struct {
  const char **items;
  size_t count, capacity;
} Task;

// group function
static bool clean_group(Task*);
static bool status_group(Task*);
static bool test_group(Task*);
static bool benchmark_group(Task*);
// root function
static bool exec_run(const char *);
static int  obj_compile(const char*, const Flags);
static bool exec_compile(const char*, const char**,const Flags);
static bool walk_dir_cleanup(Walk_Entry);

int main(int argc, char **argv) {
  GO_REBUILD_URSELF(argc, argv);
  shift(argv, argc);
  Task task = {0};
  for (;argc;shift(argv, argc)) {
    if ((*argv)[0] == '-') {
      if ((*argv)[1] == '-') {
        const char *ar = (*argv) + 2;
        // double dash flags
        if (!strcmp("debug", ar)) {
          actionFlags |= ActionFlags_DebugRun;
        } else if (!strcmp("build", ar)) {
          actionFlags |= ActionFlags_ForceBuild;
        }
      } else {
        // dash flags
        for (const char *ar = *argv; *(++ar); ) {
          switch (*ar) {
            case 'd': actionFlags |= ActionFlags_DebugRun; break;
            case 'b': actionFlags |= ActionFlags_ForceBuild; break;
            default: break;
          }
        }
      }
    } else {
      da_append(&task, *argv);
    }
  }
  int ret = EXIT_SUCCESS;
#define CASE_ACT(A,B) if (!strcmp(da_first(&task), A) || !strcmp(da_first(&task), B))
  if (!task.count) {
    nob_log(NOB_ERROR, "at least give a command.\n");
    printf (help_msg);
    ret = EXIT_FAILURE;
  } else {
    CASE_ACT("h","help") {
      printf (help_msg);
    } else CASE_ACT("c","clean") {
      da_remove_first_item(&task);
      if (!clean_group(&task)) ret = EXIT_FAILURE;
    } else CASE_ACT("s","status") {
      da_remove_first_item(&task);
      if (!status_group(&task)) ret = EXIT_FAILURE;
    } else if (!strcmp(da_first(&task), "test")) {
      da_remove_first_item(&task);
      if (!test_group(&task)) ret = EXIT_FAILURE;
    } else if (!strcmp(da_first(&task), "benchmark")) {
      da_remove_first_item(&task);
      if (!benchmark_group(&task)) ret = EXIT_FAILURE;
    } else if (!strcmp(da_first(&task), "qtest")) {
      Flags compile_flags = {0};
      nob_log(NOB_INFO, "Compile & running %s", QExecs.name);
      stemp.count = 0;
      sb_appendf(&stemp, BIN_DIR"/%s", QExecs.name);
      const char *out = strndup(stemp.items, stemp.count);
      bool result = exec_compile(out, QExecs.srcs, compile_flags) && exec_run(out);
      free((void*)out);
      da_free(compile_flags);
      if (!result) ret = EXIT_FAILURE;
    } else {
      nob_log(NOB_ERROR, "%s option doesn't exists.\n", da_first(&task));
      printf (help_msg);
      ret = EXIT_FAILURE;
    }
  }
#undef CASE_ACT
  da_free(task);
  da_free(procs);
  da_free(cmd);
  sb_free(stemp);
  return ret;
}

static bool clean_group(Task *task) {
  if (!task->count) {
    if (!file_exists(BIN_DIR))
      nob_log(NOB_INFO, "Binary file wasn't exists.");
    else if (walk_dir(BIN_DIR, walk_dir_cleanup, .post_order = true))
      nob_log(NOB_INFO, "Cleanup walk is suceed.");
    else
      nob_log(NOB_ERROR, "Cleanup walk is error.");
  } else if(!strcmp(da_first(task), "test")) {
    if (!file_exists(TEST_DIR))
      nob_log(NOB_INFO, "Binary test file wasn't exists.");
    else if (walk_dir(TEST_DIR, walk_dir_cleanup, .post_order = true))
      nob_log(NOB_INFO, "Cleanup binary test walk is suceed.");
    else
      nob_log(NOB_ERROR, "Cleanup binary test walk is error.");
  } else if(!strcmp(da_first(task), "benchmark")) {
    if (!file_exists(BENCHMARK_DIR))
      nob_log(NOB_INFO, "Binary benchmark file wasn't exists.");
    else if (walk_dir(BENCHMARK_DIR, walk_dir_cleanup, .post_order = true))
      nob_log(NOB_INFO, "Cleanup binary benchmark walk is suceed.");
    else
      nob_log(NOB_ERROR, "Cleanup binary benchmark walk is error.");
  } else {
    nob_log(NOB_ERROR, "Cleanup of %s is not exists.", da_first(task));
    return false;
  }
  return true;
}
static bool status_group(Task *task) {
  if (!task->count) {
    nob_log(NOB_INFO, "Execution status!");
    for (size_t i = 0; i < ARRAY_LEN(Test_Execs); ++i) {
      nob_log(NOB_INFO, "%s is\t%sPASSED\033[0m", Test_Execs[i].name, file_exists(Test_Execs[i].name) ? "\033[32m" : "\033[31mnot ");
    }
    for (size_t i = 0; i < ARRAY_LEN(Benchmark_Execs); ++i) {
      nob_log(NOB_INFO, "%s is\t%sPASSED\033[0m", Benchmark_Execs[i].name, file_exists(Benchmark_Execs[i].name) ? "\033[32m" : "\033[31mnot ");
    }
  } else if (!strcmp(da_first(task), "test")) {
    nob_log(NOB_INFO, "Execution status tests!");
    for (size_t i = 0; i < ARRAY_LEN(Test_Execs); ++i) {
      nob_log(NOB_INFO, "%s is\t%sPASSED\033[0m", Test_Execs[i].name, file_exists(Test_Execs[i].name) ? "\033[32m" : "\033[31mnot ");
    }
  } else if (!strcmp(da_first(task), "benchmark")) {
    nob_log(NOB_INFO, "Execution status benchmark!");
    for (size_t i = 0; i < ARRAY_LEN(Benchmark_Execs); ++i) {
      nob_log(NOB_INFO, "%s is\t%sPASSED\033[0m", Benchmark_Execs[i].name, file_exists(Benchmark_Execs[i].name) ? "\033[32m" : "\033[31mnot ");
    }
  } else {
    nob_log(NOB_ERROR, "There is no status for %s group.", da_first(task));
    return false;
  }
  return true;
}
static bool test_group(Task *task) {
  Flags compile_flags = {0};
  size_t i;
  bool result;
  da_append_many(&compile_flags,
#if defined(_MSC_VER) && !defined(__clang__)                   
    ((const char*[]){"/Od", "/Zi", "/I.\test"}), 3
#else                   
    ((const char*[]){"-O0", "-ggdb", "-I./test"}), 3
#endif
  );
  if (!task->count) {
    nob_log(NOB_INFO, "Compile & running All Tests");
    result = true;
    for (i = 0; result && (i < ARRAY_LEN(Test_Execs)); ++i) {
      nob_log(NOB_INFO, "Compile & running %s", Test_Execs[i].name);
      stemp.count = 0;
      sb_appendf(&stemp, TEST_DIR"/%s_test", Test_Execs[i].name);
      const char *out = strndup(stemp.items, stemp.count);
      result &= exec_compile(out, Test_Execs[i].srcs, compile_flags) && exec_run(out);
      free((void*)out);
    }
  } else {
    for (i = 0; i < ARRAY_LEN(Test_Execs); ++i) {
      if (!strcmp(da_first(task), Test_Execs[i].name)) {
        break;
      }
    }
    if (i < ARRAY_LEN(Test_Execs)) {
      nob_log(NOB_INFO, "Compile & running %s", Test_Execs[i].name);
      stemp.count = 0;
      sb_appendf(&stemp, TEST_DIR"/%s_test", Test_Execs[i].name);
      const char *out = strndup(stemp.items, stemp.count);
      result = exec_compile(out, Test_Execs[i].srcs, compile_flags) && exec_run(out);
      free((void*)out);
    } else {
      nob_log(NOB_ERROR, "Unknown test of %s", da_first(task));
    }
  }
  da_free(compile_flags);
  return result;
}
static bool benchmark_group(Task *task) {
  Flags compile_flags = {0};
  size_t i;
  bool result;
  da_append_many(&compile_flags,
#if defined(_MSC_VER) && !defined(__clang__)                   
    ((const char*[]){"/O3", "/I.\test", "-I./benchmark"}), 3
#else
    ((const char*[]){"-O3", "-I./test", "-I./benchmark"}), 3
#endif
  );
  if (!task->count) {
    nob_log(NOB_INFO, "Compile & running All Tests");
    result = true;
    for (i = 0; result && (i < ARRAY_LEN(Benchmark_Execs)); ++i) {
      nob_log(NOB_INFO, "Compile & running %s", Benchmark_Execs[i].name);
      stemp.count = 0;
      sb_appendf(&stemp, BENCHMARK_DIR"/%s_benchmark", Benchmark_Execs[i].name);
      const char *out = strndup(stemp.items, stemp.count);
      result &= exec_compile(out, Benchmark_Execs[i].srcs, compile_flags) && exec_run(out);
      free((void*)out);
    }
  } else {
    for (i = 0; i < ARRAY_LEN(Benchmark_Execs); ++i) {
      if (!strcmp(da_first(task), Benchmark_Execs[i].name)) {
        break;
      }
    }
    if (i < ARRAY_LEN(Benchmark_Execs)) {
      nob_log(NOB_INFO, "Compile & running %s", Benchmark_Execs[i].name);
      stemp.count = 0;
      sb_appendf(&stemp, BENCHMARK_DIR"/%s_benchmark", Benchmark_Execs[i].name);
      const char *out = strndup(stemp.items, stemp.count);
      result = exec_compile(out, Benchmark_Execs[i].srcs, compile_flags) && exec_run(out);
      free((void*)out);
    } else {
      nob_log(NOB_ERROR, "Unknown test of %s", da_first(task));
    }
  }
  da_free(compile_flags);
  return result;
}

static bool exec_run(const char *exec) {
  if (actionFlags & ActionFlags_DebugRun) {
#if defined(_MSC_VER) && !defined(__clang__)
    #error("how to debug on msvc?")
#else                   
    cmd_append(&cmd, "gdb");
#endif
  }
  cmd_append(&cmd, exec);
  if (!cmd_run(&cmd)) {
    delete_file(exec);
    return false;
  }
  if (actionFlags & ActionFlags_DebugRun) {
    delete_file(exec);
  }
  return true;
}
static bool exec_compile(const char *out, const char **srcs, const Flags flags) {
  size_t i;
  // exec need rebuild ?
  {
    bool rebuild = !file_exists(out);
    int build;
    for(i = 0; srcs[i]; ++i) {
      build = obj_compile(srcs[i], flags);
      if (build < 0) {
        nob_log(NOB_ERROR, "make objs %s for executable %s is fail", srcs[i], out);
        if (!procs_flush(&procs))
          nob_log(NOB_ERROR, "fail procs compile %s", out);
        return false;
      }
      rebuild = rebuild || !!build;
    }
    if (!rebuild) return true;
  }
  if (!mkdir_if_not_exists(temp_dir_name(out)))
    return false;
  // Wait on all the async processes to finish and reset procs dynamic array to 0
  if (!procs_flush(&procs)) {
    nob_log(NOB_ERROR, "fail procs compile %s", out);
    return false;
  }
  nob_cc(&cmd);
  // append objs file
  for(i = 0; srcs[i]; ++i)
    da_append(&cmd, temp_sprintf(OBJ_DIR"/%s.o", srcs[i]));
  nob_cc_output(&cmd, out);
#ifndef _MSC_VER
  cmd_append(&cmd, 
   "-lc",
# ifndef NO_STDMATH
   "-lm",
# endif
  );
#endif
  return cmd_run(&cmd);
}
static int obj_compile(const char *in, const Flags flags) {
  String_Builder sb = {0};
  sb_appendf(&sb, OBJ_DIR"/%s.o", in);
  const char *out = strndup(sb.items, sb.count);
  int res = (actionFlags & ActionFlags_ForceBuild) || !file_exists(out);
  if (res < 1) {
    // load dependencies
    sb.count = 0;
    sb_appendf(&sb, OBJ_DIR"/%s.d", in);
    const char *depen_file = strndup(sb.items, sb.count);
    sb.count = 0;
    if (file_exists(depen_file) && read_entire_file(depen_file, &sb)) {
      File_Paths fp = {0};
      String_View sv = sb_to_sv(sb);
      String_View src = sv_chop_by_delim(&sv, ':');
      for (size_t j = 0, k; (j < sv.count); ++j) {
        if (!sv.data[j] || isspace(sv.data[j]) || (sv.data[j] == '\\'))
          continue;
        for (k = j + 1; (k < sv.count) && (!isspace(sv.data[k]) &&
            (sv.data[k] != '\\') && sv.data[k]
          ); ++k) ;
        da_append(&fp, strndup(sv.data + j, k - j));
        j = k;
      }
      res = needs_rebuild(out, fp.items, fp.count);
      da_foreach(const char*, fpi, &fp) {
        free((void*)*fpi);
      }
      da_free(fp);
    }
    free((void*)depen_file);
    // compile it anyway
  }
  if (res > 0) {
    if (mkdir_if_not_exists(temp_dir_name(out))) {
      // res == 1, let's build
      // create obj file
      char *ext = temp_file_ext(in);
      if (!strcmp(ext, ".c")) {
        nob_cc(&cmd);
    #if defined(_MSC_VER) && !defined(__clang__)
    # error("object input cl.exe")
    #else
        cmd_append(&cmd, "-c", in);
    #endif
        nob_cc_output(&cmd, out);
        cmd_append(&cmd,
    #if defined(_MSC_VER) && !defined(__clang__)
          "/MMD", "/std:c11", "/WX", "/W4", "/nologo", "/D_CRT_SECURE_NO_WARNINGS", "/I.\main",
    #  ifdef NO_STDMATH
          "/DNO_STDMATH",
    #  endif // NO_STDMATH
    #  ifdef FAST_MATH
          "/fp:fast", "/DFASTER_MATH",
    #  endif // FAST_MATH
    #else
          "-MMD", "-std=c11", "-Werror", "-Wall", "-I./main",
    #  ifdef NO_STDMATH
          "-DNO_STDMATH",
    #  endif // NO_STDMATH
    #  ifdef FAST_MATH
          "-ffast-math", "-DFASTER_MATH",
    #  endif // FAST_MATH
    #endif
        );
        da_append_many(&cmd, flags.items, flags.count);
      } else {
        nob_log(NOB_ERROR, "not ready to compile %s file", ext);
        res = -1;
      }
      if (res > 0) res = cmd_run(&cmd, .async = &procs) ? 1 : -1;
    } else {
      res = -1;
    }
  }
  free((void*)out);
  sb_free(sb);
  return res;
}
static bool walk_dir_cleanup(Walk_Entry entry) {
  return delete_file(entry.path);
}

