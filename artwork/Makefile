SOUND=../sound

.PHONY:
all: nothing

.PHONY:
nothing:
	@>/dev/null echo void

interactive:
	PYTHONPATH=$(SOUND)/lib/python:$(SOUND)/py python workspace.py

.PHONY:
sweep clean:
	@rm -f *.pyc
