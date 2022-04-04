.PHONY: fuzz

fuzz:
ifneq ($(FUZZ_TIMEOUT), )
	@./process.py -t $(FUZZ_TIMEOUT)
else
	@./process.py
endif
