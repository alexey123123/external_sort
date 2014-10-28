TARGET1 := bin/esort
TARGET2 := bin/test_merge

LDFLAGS := -lrt -lpthread
CFLAGS := -Wno-unused-parameter -O3 -std=c++0x

Q := @

CPP := g++

OBJDIR := .obj

SRC_ESORT := esort/main.cpp \
		esort/program_options.cpp \
		esort/thread.cpp \
		esort/sort_thread.cpp \
		esort/merge_thread.cpp \
		esort/clock_ms.cpp

SRC_TEST := tests/test_merge/test_merge.cpp \
		esort/thread.cpp \
		esort/merge_thread.cpp \
		esort/clock_ms.cpp

OBJS := $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SRC_ESORT))) 
OBJS_T := $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SRC_TEST)))

Q := @

all: $(TARGET1) $(TARGET2)

clean:
	rm -rf $(OBJDIR)
	rm -rf $(TARGET1)
	rm -rf $(TARGET2)

.PHONY: all clean

$(TARGET1): $(OBJS)
	@echo "LD $(TARGET1)"
	$(Q)$(CPP) -o $(TARGET1) $(OBJS) $(LDFLAGS)

$(TARGET2): $(OBJS_T)
	@echo "LD $(TARGET2)"
	$(Q)$(CPP) -o $(TARGET2) $(OBJS_T) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $(OBJDIR)/$*.o`
	@echo "CC $*.cpp"
	$(Q)$(CPP) -c $*.cpp -o $(OBJDIR)/$*.o $(CFLAGS)
	
	@$(CPP) -MM $(CFLAGS) $*.cpp | sed -e 's|.*:|$(OBJDIR)/$*.o:|' > $(OBJDIR)/$*.d

