# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Compile flags for linux / osx
# Deal with duplicates caused by RediSearch using its own version of triemap
duplicates = -DTrieMapIterator_Next=RG_TrieMapIterator_Next
duplicates += -DNewTrieMap=RG_NewTrieMap
duplicates += -DTrieMap_Delete=RG_TrieMap_Delete
duplicates += -DTrieMap_Iterate=RG_TrieMap_Iterate
duplicates += -DTrieMapIterator_Free=RG_TrieMapIterator_Free
duplicates += -DTrieMap_Find=RG_TrieMap_Find
duplicates += -DTrieMap_Add=RG_TrieMap_Add
duplicates += -DTRIEMAP_NOTFOUND=RG_TRIEMAP_NOTFOUND
duplicates += -DTrieMap_Free=RG_TrieMap_Free

CFLAGS = -W -fno-common -g -ggdb -fPIC -std=gnu99 -O3 
CFLAGS += $(duplicates)

SOURCEDIR = .
CC_SOURCES = $(wildcard $(SOURCEDIR)/*.c)
CC_OBJECTS = $(patsubst $(SOURCEDIR)/%.c, $(SOURCEDIR)/%.o, $(CC_SOURCES))

.SUFFIXES: .c .cc .o

all: libtriemap.a


libtriemap.a: $(CC_OBJECTS)
	ar rcs $@ $^

clean:
	rm -rf *.o *.a
