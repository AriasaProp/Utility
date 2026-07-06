#define NOBDEF static
#define NOB_NO_ECHO
// #define NOBDEBUG "-ggdb", 
#define NOB_IMPLEMENTATION
#include "nob.h"

#define  BIN_DIR           "bin"
#define  OBJ_DIR   BIN_DIR"/obj"
#define TOOL_DIR  BIN_DIR"/tool"
#define DATA_DIR  BIN_DIR"/data"
#define TEST_DIR  BIN_DIR"/test"
#define BENCHMARK_DIR  BIN_DIR"/benchmark"

#include "config.h"
#include "help.h"

typedef enum {
  ActionFlags_None = 0,
  ActionFlags_ForceBuild = 1,
  ActionFlags_DebugRun   = 2,
} ActionFlags;

typedef struct {
  const char *src;
  File_Paths deps;
} File_Src;
typedef struct {
  File_Src *items;
  size_t count, capacity;
} File_Srcs;

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
  Exec_Type type;
  const char **srcs;
} File_Exe;
static const File_Exe Execs[] = {
  {
    .name = BIN_DIR"/qtest",
    .type = Exec_QTest,
    .srcs = (const char *[]) {
      TEST_SRCS,
      "test/qtest.c",
      NULL
    }
  },{
    .name = TEST_DIR"/rand_test",
    .type = Exec_Test,
    .srcs = (const char *[]) {
      TEST_SRCS,
      "test/math/rand_test.c",
      NULL
    }
  },{
    .name = TEST_DIR"/complex_test",
    .type = Exec_Test,
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/math/complex.c",
      "test/math/complex_test.c",
      NULL
    }
  },{
    .name = TEST_DIR"/matrix_test",
    .type = Exec_Test,
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/math/matrix.c",
      "test/math/matrix_test.c",
      NULL
    }
  },{
    .name = TEST_DIR"/bigInteger_test",
    .type = Exec_Test,
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/math/bigInteger.c",
      "test/math/bigInteger_test.c",
      NULL
    }
  },{
    .name = TEST_DIR"/sort_test",
    .type = Exec_Test,
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/algorithm/sort.c",
      "test/algorithm/sort_test.c",
      NULL
    }
  },{
    .name = TEST_DIR"/hash_test",
    .type = Exec_Test,
    .srcs = (const char *[]) {
      TEST_SRCS,
      "main/algorithm/hash.c",
      "test/algorithm/hash_test.c",
      NULL
    }
  },{
    .name = BENCHMARK_DIR"/bigInteger_benchmark",
    .type = Exec_Benchmark,
    .srcs = (const char *[]) {
      BENCHMARK_SRCS,
      "main/math/bigInteger.c",
      "benchmark/math/bigInteger_benchmark.c",
      NULL
    }
  },{
    .name = BENCHMARK_DIR"/sort_benchmark",
    .type = Exec_Benchmark,
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
static File_Srcs src_dep;
static String_Builder sb;
static int actionFlags;

typedef struct {
  const char **items;
  size_t count;
  size_t capacity;
} Flags;

static bool exec_run(const char *);
static int  obj_compile(const char*, const Flags);
static bool exec_compile(const File_Exe,const Flags);
static bool walk_dir_cleanup(Walk_Entry);

int main(int argc, char **argv) {
  GO_REBUILD_URSELF(argc, argv);
  int ret = EXIT_SUCCESS;
  size_t i, j, k;
  shift(argv, argc);
  int exec_index = -1;
  actionFlags = ActionFlags_None;
#define CASE_ACT(A,B) if (!strcmp(*argv, A) || !strcmp(*argv, B))
#define CASE_FLG(A,B) if (!strcmp(*argv, A) || !strcmp(*argv, B))
  while (argc) {
    CASE_ACT("h","help") {
      printf (help_msg);
      break;
    } else CASE_ACT("c","clean") {
      if (!file_exists(BIN_DIR))
        nob_log(NOB_INFO, "Binary file doesn't exists.");
      else if (walk_dir(BIN_DIR, walk_dir_cleanup, .post_order = true))
        nob_log(NOB_INFO, "Cleanup walk is suceed.");
      else
        nob_log(NOB_ERROR, "Cleanup walk is error.");
      break;
    } else CASE_ACT("s","status") {
      nob_log(NOB_INFO, "Exec status test!");
      for (i = 0; i < ARRAY_LEN(Execs); ++i) {
        nob_log(NOB_INFO, "%s is\t%sPASSED\033[0m", Execs[i].name, file_exists(Execs[i].name) ? "\033[32m" : "\033[31mnot " );
      }
      break;
    } else CASE_FLG("-b","--build") {
      actionFlags |= ActionFlags_ForceBuild;
    } else CASE_FLG("-d","--debug") {
      actionFlags |= ActionFlags_DebugRun;
    } else {
      for (i = 0; i < ARRAY_LEN(Execs); ++i) {
        if (!strcmp(Execs[i].name, *argv)) {
          exec_index = i;
          break;
        }
      }
      if (exec_index < 0) {
        nob_log(NOB_INFO, "unknown command or flags of \"%s\"!", *argv);
        printf (help_msg);
        break;
      }
    }
    shift(argv, argc);
  }
#undef CASE_ACT
#undef CASE_FLG
  if (exec_index >= 0) {
    File_Exe current_exe = Execs[exec_index];
    // load dependencies
    if (!(actionFlags & ActionFlags_ForceBuild)) {
      for (i = 0; current_exe.srcs[i]; ++i) {
        const char *depen_file = temp_sprintf(OBJ_DIR"/%s.d", current_exe.srcs[i]);
        sb.count = 0;
        if (file_exists(depen_file)) {
          if (read_entire_file(depen_file, &sb)) {
            File_Paths fp = {0};
            String_View sv = sb_to_sv(sb);
            String_View src = sv_chop_by_delim(&sv, ':');
            for (j = 0; (j < sv.count); ++j) {
              if (!sv.data[j] || isspace(sv.data[j]) || (sv.data[j] == '\\'))
                continue;
              for (k = j + 1; (k < sv.count) && (!isspace(sv.data[k]) &&
                  (sv.data[k] != '\\') && sv.data[k]
                ); ++k) ;
              da_append(&fp, strndup(sv.data + j, k - j));
              j = k;
            }
            da_append(&src_dep, ((File_Src){.src = strndup(current_exe.srcs[i], strlen(current_exe.srcs[i])), .deps = fp}));
          } else {
            nob_log(NOB_ERROR, "Fail read %s dependency file.", depen_file);
          }
        } else {
          nob_log(NOB_INFO, "%s will compiling anyway.", current_exe.srcs[i]);
        }
      }
    }
    Flags compile_flags = {0};
    switch (current_exe.type) {
      default: break;
      case Exec_QTest:
      case Exec_Test:
        da_append_many(&compile_flags,
#if defined(_MSC_VER) && !defined(__clang__)                   
          ((const char*[]){"/Od", "/Zi", "/I.\test"}), 3
#else                   
          ((const char*[]){"-O0", "-ggdb", "-I./test"}), 3
#endif
        );
        break;
      case Exec_Benchmark:
        da_append_many(&compile_flags,
#if defined(_MSC_VER) && !defined(__clang__)                   
          ((const char*[]){"/O3", "/I.\test", "-I./benchmark"}), 3
#else
          ((const char*[]){"-O3", "-I./test", "-I./benchmark"}), 3
#endif
        );
        break;
    }
    nob_log(NOB_INFO, "Compile/running %s", current_exe.name);
    if (!(exec_compile(current_exe, compile_flags) && exec_run(current_exe.name))) {
      ret = EXIT_FAILURE;
    }
    da_free(compile_flags);
  }
  
  da_free(procs);
  da_free(cmd);
  sb_free(sb);
  da_foreach(File_Src, fs, &src_dep) {
    free((void*)fs->src);
    da_foreach(const char*, fp, &fs->deps)
      free((void*)*fp);
    da_free(fs->deps);
  }
  da_free(src_dep);
  return ret;
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
static bool exec_compile(const File_Exe file_exe, const Flags flags) {
  size_t i;
  // exec need rebuild ?
  {
    bool rebuild = !file_exists(file_exe.name);
    int build;
    for(i = 0; file_exe.srcs[i]; ++i) {
      build = obj_compile(file_exe.srcs[i], flags);
      if (build < 0) {
        nob_log(NOB_ERROR, "make objs %s for executable %s is fail", file_exe.srcs[i], file_exe.name);
        if (!procs_flush(&procs))
          nob_log(NOB_ERROR, "fail procs compile %s", file_exe.name);
        return false;
      }
      rebuild = rebuild || !!build;
    }
    if (!rebuild) return true;
  }
  if (!mkdir_if_not_exists(temp_dir_name(file_exe.name)))
    return false;
  // Wait on all the async processes to finish and reset procs dynamic array to 0
  if (!procs_flush(&procs)) {
    nob_log(NOB_ERROR, "fail procs compile %s", file_exe.name);
    return false;
  }
  nob_cc(&cmd);
  // append objs file
  for(i = 0; file_exe.srcs[i]; ++i)
    da_append(&cmd, temp_sprintf(OBJ_DIR"/%s.o", file_exe.srcs[i]));
  nob_cc_output(&cmd, file_exe.name);
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
  const char *out = temp_sprintf(OBJ_DIR"/%s.o", in);
  if (!(actionFlags & ActionFlags_ForceBuild) && file_exists(out)) {
    da_foreach(File_Src, fs, &src_dep) {
      if (!strcmp(fs->src, out)) {
        int need_build = needs_rebuild(out, fs->deps.items, fs->deps.count);
        if (need_build < 1) return need_build;
        break;
      }
    }
    // compile it anyway
  }
  if (!mkdir_if_not_exists(temp_dir_name(out)))
    return -1;
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
    return -1;
  }
  return cmd_run(&cmd, .async = &procs) ? 1 : -1;
}
static bool walk_dir_cleanup(Walk_Entry entry) {
  return delete_file(entry.path);
}

