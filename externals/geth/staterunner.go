package main

import "C"

import (
	"encoding/json"
	"fmt"

	"github.com/ethereum/go-ethereum/core/rawdb"
	"github.com/ethereum/go-ethereum/core/vm"
	"github.com/ethereum/go-ethereum/tests"
)

//export GethRunTest
func GethRunTest(src []byte) {
	var testsByName map[string]tests.StateTest
	if err := json.Unmarshal(src, &testsByName); err != nil {
		panic(fmt.Errorf("unable to read test: %w", err))
	}

	cfg := vm.Config{}

	// Iterate over all the tests, run them and aggregate the results
	for _, test := range testsByName {
		for _, st := range test.Subtests() {
			// Run the test, this checks the post state root hash against the expected
			test.Run(st, cfg, false, rawdb.HashScheme, func(err error, state *tests.StateTestState) {
				if err != nil {
					// Test failed, mark as so.
					panic(err)
				}
			})
		}
	}
}
