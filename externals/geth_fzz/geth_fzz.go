// The build command:
// go build -v -o geth_fzz.a -buildmode=c-archive -tags=libfuzzer -gcflags=all=-d=libfuzzer

package main

import "C"
import "fmt"
import "unsafe"
import "github.com/ethereum/go-ethereum/core/vm"

var jt = vm.NewPragueEOFInstructionSetForTesting()

//export geth_fzz_eof_validate
func geth_fzz_eof_validate(data *byte, size int) bool {
    b := unsafe.Slice(data, size);
    c := vm.Container{}
    err := c.UnmarshalBinary(b, false)
    if err != nil {
        //fmt.Println(fmt.Errorf("geth err: %v", err))
        return false
    }

    err = c.ValidateCode(&jt, false)
    if err != nil {
        //fmt.Println(fmt.Errorf("geth err: %v", err))
        return false
    }
    return true
}

func main() {
    fmt.Println("geth_fuzz main()")
}
