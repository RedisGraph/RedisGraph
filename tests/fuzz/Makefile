
ifneq ($(FUZZ_TIMEOUT), )
TIMEOUT_ARG=-t $(FUZZ_TIMEOUT)
endif

fuzz:
	@./process.py -m $(TARGET) $(TIMEOUT_ARG)

.PHONY: fuzz
