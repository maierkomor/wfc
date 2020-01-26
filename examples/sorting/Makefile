WFC	= ../../bin/wfc

all: sort_id sort_name sort_size sort_type sort_none

sort_id:
	$(WFC) -o $@ -fSortMembers=id sorting.wfc

sort_name:
	$(WFC) -o $@ -fSortMembers=name sorting.wfc

sort_size:
	$(WFC) -o $@ -fSortMembers=size sorting.wfc

sort_type:
	$(WFC) -o $@ -fSortMembers=type sorting.wfc

sort_none:
	$(WFC) -o $@ -fSortMembers=none sorting.wfc
