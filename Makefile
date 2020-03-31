CC := g++
CFLAGS := -Wall -O3

SRCDIR := src
OBJDIR := obj
BINDIR := bin
$(shell [ ! -d $(OBJDIR) ] && mkdir $(OBJDIR))
$(shell [ ! -d $(BINDIR) ] && mkdir $(BINDIR))

TARGET := $(BINDIR)/cache_analyze
SRCS = $(sort $(wildcard $(SRCDIR)/*.cpp))
HEADS = $(sort $(wildcard $(SRCDIR)/*.h))
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(sort $(wildcard $(SRCDIR)/*.cpp)))
DLIBS = 

$(TARGET):$(OBJS) $(HEADS)
	$(CC) $(CFLAGS) -o $@ $^ $(DLIBS)

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

TAG := cscope.files cscope.in.out cscope.out cscope.po.out tags
.PHONY: clean tags
clean:
	rm -rf $(BINDIR) $(OBJDIR) $(TAG)

tags: 
	rm -f $(TAG)
	find . -type f -regex ".*\.\(cpp\|h\)" >cscope.files
	cscope -bq
	ctags -L cscope.files --fields=+iaS --extra=+q
