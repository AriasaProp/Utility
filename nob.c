#define NOBDEF static
#define NOB_NO_ECHO
#define NOB_IMPLEMENTATION
#include "nob.h"

#define  BIN_DIR           "bin"
#define  OBJ_DIR   BIN_DIR"/obj"
#define TOOL_DIR  BIN_DIR"/tool"
#define DATA_DIR  BIN_DIR"/data"
#define TEST_DIR  BIN_DIR"/test"

#include "config.h"
#include "help.h"

typedef enum {
  Action_None = 0,
  Action_Help,
  Action_Status,
  Action_Clean,
  Action_Test,
} Actions;

typedef enum {
  ActionFlags_None = 0,
  ActionFlags_ForceBuild = 1,
  ActionFlags_DebugRun   = 2,
} ActionFlags;

typedef struct {
  const char *src;
  const char **deps;
} File_Src;
typedef struct {
  File_Src *items;
  size_t count, capacity;
} File_Srcs;

#define FILE_SRC_DEPS(SRC, ...) (File_Src){.src = (SRC), .deps = (const char*[]){"main/common.h", __VA_ARGS__}}
#define FILE_SRC_TEST_DEPS(SRC, ...) (File_Src){.src = (SRC), .deps = (const char*[]){"main/common.h", "test/util/profiling.h", __VA_ARGS__}}
#define COMMON_SRC FILE_SRC_DEPS("main/common.c", NULL)
#define TEST_SRCS COMMON_SRC, FILE_SRC_TEST_DEPS("test/util/profiling.c", NULL)
static const struct {
  const char *name;
  File_Src *srcs;
} Tests_Exc[] = {
  {
    .name = "rand",
    .srcs = (File_Src[]) {
      TEST_SRCS,
      FILE_SRC_TEST_DEPS("test/math/rand_test.c", "main/common.h"),
      NULL
    }
  },{
    .name = "complex",
    .srcs = (File_Src[]) {
      TEST_SRCS,
      FILE_SRC_DEPS("main/math/complex.c","main/math/complex.h"),
      FILE_SRC_TEST_DEPS("test/math/complex_test.c", NULL),
      NULL
    }
  },{
    .name = "matrix",
    .srcs = (File_Src[]) {
      TEST_SRCS,
      FILE_SRC_DEPS("main/math/matrix.c","main/math/matrix.h"),
      FILE_SRC_TEST_DEPS("test/math/matrix_test.c", NULL),
      NULL
    }
  },{
    .name = "bigInteger",
    .srcs = (File_Src[]) {
      TEST_SRCS,
      FILE_SRC_DEPS("main/math/bigInteger.c","main/math/bigInteger.h"),
      FILE_SRC_TEST_DEPS("test/math/bigInteger_test.c", NULL),
      NULL
    }
  },{
    .name = "sort",
    .srcs = (File_Src[]) {
      TEST_SRCS,
      FILE_SRC_DEPS("main/algorithm/sort.c","main/algorithm/sort.h"),
      FILE_SRC_TEST_DEPS("test/algorithm/sort_test.c", NULL),
      NULL
    }
  },{
    .name = "hash",
    .srcs = (File_Src[]) {
      TEST_SRCS,
      FILE_SRC_DEPS("main/algorithm/hash.c","main/math/algorithm/hash.h"),
      FILE_SRC_TEST_DEPS("test/algorithm/hash_test.c", NULL),
      NULL
    }
  },
  NULL
};
#undef COMMON_SRC
#undef TEST_SRCS
#undef FILE_SRC_DEPS
#undef FILE_SRC_TEST_DEPS

static Cmd cmd;
static Procs procs;
static int actionFlags;

typedef struct {
  const char **items;
  size_t count;
  size_t capacity;
} Flags;

static bool test_run(const char*, const File_Src*);
static bool exec_run(const char *);
static int  obj_compile(const File_Src, const Flags);
static bool exec_compile(const char*,const File_Src*,const Flags);
static bool walk_dir_cleanup(Walk_Entry);

int main(int argc, char **argv) {
  GO_REBUILD_URSELF(argc, argv);
  int ret = EXIT_SUCCESS;
  size_t i, j;
  shift(argv, argc);
  int action = Action_None;
  int test_index = -1;
  actionFlags = ActionFlags_None;
#define CASE_ACT1(A)  if (!strcmp(*argv, A) && (action == Action_None))
#define CASE_ACT(A,B) if ((!strcmp(*argv, A) || !strcmp(*argv, B)) && (action == Action_None))
#define CASE_FLG(A,B) if (!strcmp(*argv, A) || !strcmp(*argv, B))
  while (argc) {
    CASE_ACT("h","help") {
      action = Action_Help;
    } else CASE_ACT("c","clean") {
      action = Action_Clean;
    } else CASE_ACT("s","status") {
      action = Action_Status;
    } else CASE_ACT("t","tests") {
      action = Action_Test;
    } else CASE_ACT1("rand_test") {
      action = Action_Test;
      test_index = 0;
    } else CASE_ACT1("complex_test") {
      action = Action_Test;
      test_index = 1;
    } else CASE_ACT1("matrix_test") {
      action = Action_Test;
      test_index = 2;
    } else CASE_ACT1("bigInteger_test") {
      action = Action_Test;
      test_index = 3;
    } else CASE_ACT1("sort_test") {
      action = Action_Test;
      test_index = 4;
    } else CASE_ACT1("hash_test") {
      action = Action_Test;
      test_index = 5;
    } else if (!strcmp(*argv, "d") || !strcmp(*argv, "debug")) {
      break;
    } else CASE_FLG("-b","--build") {
      actionFlags |= ActionFlags_ForceBuild;
    } else CASE_FLG("-d","--debug") {
      actionFlags |= ActionFlags_DebugRun;
    } else {
      action = Action_Help;
      nob_log(NOB_INFO, "unknown command or flags of \"%s\"!", *argv);
    }
    shift(argv, argc);
  }
#undef CASE_ACT1
#undef CASE_ACT
#undef CASE_FLG
  switch (action) {
    default:
    case Action_Help:
    	printf (help_msg);
      break;
    case Action_Clean:
      if (!file_exists(BIN_DIR))
        nob_log(NOB_INFO, "Binary file doesn't exists.");
      else if (walk_dir(BIN_DIR, walk_dir_cleanup, .post_order = true))
        nob_log(NOB_INFO, "Cleanup walk is suceed.");
      else
        nob_log(NOB_ERROR, "Cleanup walk is error.");
      break;
    case Action_Status:
      nob_log(NOB_INFO, "Lists test!");
      for (i = 0; i < ARRAY_LEN(Tests_Exc); ++i) {
        nob_log(NOB_INFO, "%s is\t%sPASSED\033[0m",
          Tests_Exc[i].name,
          file_exists(temp_sprintf("%s/%s_test", TEST_DIR, Tests_Exc[i].name)) ?
            "\033[32m" : "\033[31mnot "
        );
      }
      break;
    case Action_Test:
      if (test_index < 0) {
        nob_log(NOB_INFO, "Running All Tests");
        for (i = 0; (Tests_Exc[i].name); ++i) {
          if (!test_run(Tests_Exc[i].name, Tests_Exc[i].srcs)) ret = EXIT_FAILURE;
        }
      } else {
        nob_log(NOB_INFO, "Running %s test", Tests_Exc[test_index].name);
        if (!test_run(Tests_Exc[test_index].name, Tests_Exc[test_index].srcs)) ret = EXIT_FAILURE;
      }
      break;
  }
  nob_log(NOB_INFO, "Done!");
  da_free(procs);
  da_free(cmd);
  return ret;
}

// test run format
static bool test_run(const char *name, const File_Src *fs) {
  // test flags
  const char *out = temp_sprintf(TEST_DIR"/%s_test", name);
  Flags flags = {0};
  da_append_many(&flags,
#if defined(_MSC_VER) && !defined(__clang__)                   
    ((const char*[]){"/Od", "/Zi", "/I.\test"}), 3
#else                   
    ((const char*[]){"-O0", "-ggdb", "-I./test"}), 3
#endif
  );
  bool ret = exec_compile(out, fs, flags) && exec_run(out);
  da_free(flags);
  return ret;
}
static bool exec_run(const char *exec) {
  if (actionFlags & ActionFlags_DebugRun) {
#if defined(_MSC_VER) && !defined(__clang__)
    #error("debug on msvc")
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
static bool exec_compile(const char *out, const File_Src *in, const Flags flags) {
  size_t i;
  // exec need rebuild ?
  int build;
  bool out_exists = file_exists(out);
  bool rebuild = !out_exists;
  for(i = 0; in[i].src; ++i) {
    build = obj_compile(in[i], flags);
    if (build < 0) {
      nob_log(NOB_ERROR, "make objs %s for executable %s is fail", in[i].src, out);
      if (!procs_flush(&procs))
        nob_log(NOB_ERROR, "fail procs compile %s", out);
      return false;
    }
    rebuild = rebuild || !!build;
  }
  if (rebuild && out_exists) delete_file(out);
  else if (!rebuild) return true;
  
  if (!mkdir_if_not_exists(temp_dir_name(out)))
    return false;
  // Wait on all the async processes to finish and reset procs dynamic array to 0
  if (!procs_flush(&procs)) {
    nob_log(NOB_ERROR, "fail procs compile %s", out);
    return false;
  }
  nob_cc(&cmd);
  // append objs file
  for(i = 0; in[i].src; ++i)
    da_append(&cmd, temp_sprintf(OBJ_DIR"/%s.o", in[i].src));
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
static int obj_compile(const File_Src in, const Flags flags) {
  const char *out = temp_sprintf(OBJ_DIR"/%s.o", in.src);
  if (!(actionFlags & ActionFlags_ForceBuild) && file_exists(out)) {
    File_Paths indep = {0};
    da_append(&indep, in.src);
    for (size_t i = 0; in.deps[i]; ++i) {
      da_append(&indep, in.deps[i]);
    }
    int need_build = needs_rebuild(out, indep.items, indep.count);
    da_free(indep);
    if (need_build < 1) return need_build;
  }
  if (!mkdir_if_not_exists(temp_dir_name(out))) return -1;
  // create obj file
  char *ext = temp_file_ext(in.src);
  if (!strcmp(ext, ".c")) {
    nob_cc(&cmd);
#if defined(_MSC_VER) && !defined(__clang__)
# error("object input cl.exe")
#else
    cmd_append(&cmd, "-c", in.src);
#endif
    nob_cc_output(&cmd, out);
    cmd_append(&cmd,
#if defined(_MSC_VER) && !defined(__clang__)
      "/std:c11", "/WX", "/W4", "/nologo", "/D_CRT_SECURE_NO_WARNINGS", "/I.\main",
#  ifdef NO_STDMATH
      "/DNO_STDMATH",
#  endif // NO_STDMATH
#  ifdef FAST_MATH
      "/fp:fast", "/DFASTER_MATH",
#  endif // FAST_MATH
#else
      "-std=c11", "-Werror", "-Wall", "-I./main",
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
