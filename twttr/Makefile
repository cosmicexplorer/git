# NOTE: Only *Twitter introduced builtins* go into twttr/builtin/

TWTTR_CFLAGS  = -DTWTTR $(TWTTR_INCJANNSON)
TWTTR_LDFLAGS = $(TWTTR_LIBCURL) $(TWTTR_LIBJANNSON)

ifeq ($(uname_S),Darwin)
	TWTTR_LDFLAGS += -Wl,-rpath,@executable_path/../.. -Wl,-rpath,@executable_path/.. -Wl,-rpath,$(prefix)
else
	BASIC_LDFLAGS += -Wl,-R,\$$ORIGIN/../../lib -Wl,-R,\$$ORIGIN/../lib -Wl,R,$(prefix)/lib
endif

TWTTR_LIB_OBJS  = twttr/count-objects.o
TWTTR_LIB_OBJS += twttr/fetch.o
TWTTR_LIB_OBJS += twttr/parent-proc.o
TWTTR_LIB_OBJS += twttr/stats-report.o
TWTTR_LIB_OBJS += twttr/string2int-hashmap.o
TWTTR_LIB_OBJS += twttr/twitter-config.o
TWTTR_LIB_OBJS += twttr/zipkin.o

TWTTR_BUILTIN_OBJS  = twttr/builtin/combine-pack.o
TWTTR_BUILTIN_OBJS += twttr/builtin/upload-stats.o
TWTTR_BUILTIN_OBJS += twttr/builtin/pull-ref.o
TWTTR_BUILTIN_OBJS += twttr/builtin/push-ref.o

TWTTR_SCRIPT_PYTHON  = twttr/builtin/git-manage-config.py
TWTTR_SCRIPT_PYTHON += twttr/builtin/git-workspace.py
