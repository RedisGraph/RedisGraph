.PHONY: fuzz

export PATH:="$(PATH):$(shell python3 -c 'import site; print(site.getuserbase())')/bin"

fuzz:
ifneq ($(FUZZ_TIMEOUT), )
	@./process.py -t $(FUZZ_TIMEOUT)
else
	@./process.py
endif
