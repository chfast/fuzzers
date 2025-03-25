package main

import "C"

import (
	"encoding/json"
	"fmt"
	"os"

	"github.com/ethereum/go-ethereum/common"
	"github.com/ethereum/go-ethereum/core/rawdb"
	"github.com/ethereum/go-ethereum/core/vm"
	"github.com/ethereum/go-ethereum/tests"
)

// testResult contains the execution status after running a state test, any
// error that might have occurred and a dump of the final state if requested.
type testResult struct {
	Name  string       `json:"name"`
	Pass  bool         `json:"pass"`
	Root  *common.Hash `json:"stateRoot,omitempty"`
	Fork  string       `json:"fork"`
	Error string       `json:"error,omitempty"`
}

//export GethRunTest
func GethRunTest(src []byte) {
	var testsByName map[string]tests.StateTest
	if err := json.Unmarshal(src, &testsByName); err != nil {
		panic(fmt.Errorf("unable to read test: %w", err))
	}

	cfg := vm.Config{}

	// Iterate over all the tests, run them and aggregate the results
	result := &testResult{Pass: true}
	for _, test := range testsByName {
		for _, st := range test.Subtests() {
			// Run the test and aggregate the result
			test.Run(st, cfg, false, rawdb.HashScheme, func(err error, state *tests.StateTestState) {
				var root common.Hash
				if state.StateDB != nil {
					root = state.StateDB.IntermediateRoot(false)
					result.Root = &root
					fmt.Fprintf(os.Stderr, "{\"stateRoot\": \"%#x\"}\n", root)
				}
				if err != nil {
					// Test failed, mark as so.
					panic(err)
				}
			})
		}
	}
}
