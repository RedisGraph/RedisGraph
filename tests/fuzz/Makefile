.PHONY: fuzz

fuzz:
ifneq ($(FUZZ_TIMEOUT), )
	@python3 process.py -t $(FUZZ_TIMEOUT)
else
	@python3 process.py
endif
