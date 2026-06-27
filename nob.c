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
} ActionFlags;

static const char *Common_Files[] = {
  "main/common.c",
};
static const char *Tests_Files[] = {
  "test/util/profiling.c",
};

static const struct {
  const char *name;
  const char **srcs;
} Tests_Exc[] = {
  {
    .name = "rand",
    .srcs = (const char*[]){
	    "test/math/rand_test.c",
      NULL
    }
  },{
    .name = "complex",
    .srcs = (const char*[]){
      "main/math/complex.c",
	    "test/math/complex_test.c",
      NULL
    }
  },{
    .name = "matrix",
    .srcs = (const char*[]){
      "main/math/matrix.c",
	    "test/math/matrix_test.c",
      NULL
    }
  },{
    .name = "bigInteger",
    .srcs = (const char*[]){
      "main/math/bigInteger.c",
	    "test/math/bigInteger_test.c",
      NULL
    }
  },{
    .name = "sort",
    .srcs = (const char*[]){
      "main/algorithm/sort.c",
	    "test/algorithm/sort_test.c",
      NULL
    }
  },{
    .name = "hash",
    .srcs = (const char*[]){
      "main/algorithm/hash.c",
	    "test/algorithm/hash_test.c",
      NULL
    }
  },
};
static Cmd cmd;
static Procs procs;
static int actionFlags;
typedef struct {
  const char **items;
  size_t count;
  size_t capacity;
} Flags;

static bool test_run(size_t);
static bool exec_run(const char *);
static bool obj_compile(const char*, Flags*);
static bool exec_compile(const char*,File_Paths*,Flags*);
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
        int exists = file_exists(temp_sprintf("%s/%s_test", TEST_DIR, Tests_Exc[i].name));
        nob_log(NOB_INFO, "%s is\t%sPASSED\033[0m",Tests_Exc[i].name, exists ? "\033[32m" : "\033[31mnot ");
      }
      break;
    case Action_Test:
      if (test_index < 0) {
        nob_log(NOB_INFO, "Running All Tests");
        for (i = 0; i < ARRAY_LEN(Tests_Exc); ++i)
          if (!test_run(i)) ret = EXIT_FAILURE;
      } else {
        nob_log(NOB_INFO, "Running %s test", Tests_Exc[test_index].name);
        if (!test_run(test_index)) ret = EXIT_FAILURE;
      }
      break;
  }
  nob_log(NOB_INFO, "Done!");
  da_free(procs);
  da_free(cmd);
  return ret;
}

// test run format
static const char * const Test_Flags[] = {
#if defined(_MSC_VER) && !defined(__clang__)                   
  "/Od", "/Zi", "/I.\test"
#else                   
  "-O0", "-ggdb", "-I./test"
#endif
};
static bool test_run(size_t i) {
  bool ret = false;
  const char *out = temp_sprintf(TEST_DIR"/%s_test", Tests_Exc[i].name);
  // need rebuild
  File_Paths srcs = {0};
  da_append_many(&srcs, Common_Files,      ARRAY_LEN(Common_Files));
  da_append_many(&srcs, Tests_Files,       ARRAY_LEN(Tests_Files));
  for (size_t j = 0; Tests_Exc[i].srcs[j]; ++j)
    da_append(&srcs, Tests_Exc[i].srcs[j]);
  int need_build = needs_rebuild(out, srcs.items, srcs.count);
  if(need_build < 0) {
    nob_log(NOB_ERROR, "Failure check test of %s_test", Tests_Exc[i].name);
  } else if (!need_build) { // no rebuild is needed
    ret = exec_run(out);
  } else {
    // rename old test exists
    const char *out_old = temp_sprintf(TEST_DIR"/%s_test.old", Tests_Exc[i].name);
    if (file_exists(out) && !nob_rename(out, out_old)) {
      nob_log(NOB_ERROR, "Failure rename file of %s", out);
    } else {
      // start build
      Flags flags = {0};
      for (size_t j = 0; j < ARRAY_LEN(Test_Flags); ++j)
        da_append(&flags, Test_Flags[j]);
      if (exec_compile(out, &srcs, &flags) && exec_run(out)) {
        if (file_exists(out_old)) delete_file(out_old);
        ret = true;
      } else if (file_exists(out_old) && !nob_rename(out_old, out)) // rename old test back
        nob_log(NOB_ERROR, "Failure rename file test of %s, it stay old", out_old);
      da_free(flags);
    }
  }
  da_free(srcs);
  return ret;
}
static bool exec_run(const char *exec) {
  cmd_append(&cmd, exec);
  if (!cmd_run(&cmd)) {
    nob_log(NOB_ERROR, "Failure run %s", exec);
    delete_file(exec);
    return false;
  }
  return true;
}
static bool exec_compile(const char *out, File_Paths *in, Flags *flags) {
  if (!(actionFlags & ActionFlags_ForceBuild)) {
    // exec need rebuild ?
    int need_build = needs_rebuild(out, in->items, in->count);
    if (need_build < 0) {
      nob_log(NOB_ERROR, "check executable %s is fail", out);
      return false;
    } else if (!need_build)
      return true;
  }
  // src need rebuild ?
  da_foreach(const char* ,i ,in) {
    if (!obj_compile(*i, flags)) {
      nob_log(NOB_ERROR, "make objs %s for executable %s is fail", *i, out);
      return false;
    }
  }
  // Wait on all the async processes to finish and reset procs dynamic array to 0
  if (!procs_flush(&procs)) {
    nob_log(NOB_ERROR, "fail compile %s", out);
    return false;
  }
  if (!mkdir_if_not_exists(temp_dir_name(out))) return false;
  nob_cc(&cmd);
  // append objs file
  da_foreach(const char*,i,in)
    cmd_append(&cmd, temp_sprintf(OBJ_DIR"/%s.o", *i));
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
static bool obj_compile(const char *in, Flags *flags) {
  const char *out = temp_sprintf(OBJ_DIR"/%s.o",in);
  if (!(actionFlags & ActionFlags_ForceBuild)) {
    int need_build = needs_rebuild1(out,in);
    if (need_build < 0) {
      nob_log(NOB_ERROR, "check rebuild of %s is fail", out);
      return false;
    } else if (!need_build)
      return true;
  }
  if (!mkdir_if_not_exists(temp_dir_name(out))) return false;
  // create obj file
  nob_cc(&cmd);
#if defined(_MSC_VER) && !defined(__clang__)
# error("object input cl.exe")
#else
  cmd_append(&cmd, "-c", in);
#endif
  nob_cc_output(&cmd, out);
  cmd_append(&cmd,
#if defined(_MSC_VER) && !defined(__clang__)
    "/MMD", "/MP", "/std:c11", "/WX", "/W4", "/nologo", "/D_CRT_SECURE_NO_WARNINGS", "/I.\main",
#  ifdef NO_STDMATH
    "/DNO_STDMATH",
#  endif // NO_STDMATH
#  ifdef FAST_MATH
    "/fp:fast", "/DFASTER_MATH"
#  endif // FAST_MATH
#else
    "-MMD", "-MP", "-std=c11", "-Werror", "-I./main"
#  ifdef NO_STDMATH
    "-DNO_STDMATH",
#  endif // NO_STDMATH
#  ifdef FAST_MATH
    "-ffast-math", "-DFASTER_MATH"
#  endif // FAST_MATH
#endif
  );
  da_foreach(const char*, fi, flags)
    cmd_append(&cmd, *fi);
  return cmd_run(&cmd, .async = &procs);
}
static bool walk_dir_cleanup(Walk_Entry entry) {
  return delete_file(entry.path);
}
