GOPATH=$(PWD):$(HOME)/golib
all:
	# go install -v qt
	go install -v qt/core
	go install -v qt/dynamic
	go build -v eg
	go build -v qgc

test:
	go build -v tests/t000.go
	go build -v tests/t060.go
