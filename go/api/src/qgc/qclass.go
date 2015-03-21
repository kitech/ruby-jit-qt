package qgc;

import "unsafe"

type QClass struct {
	X unsafe.Pointer
	R bool // ref or not
	N string // real qt class name
	A int
}

