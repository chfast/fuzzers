// The build command:
// go build -v -o geth_fzz.a -buildmode=c-archive -tags=libfuzzer -gcflags=all=-d=libfuzzer

package main

import "C"
import "fmt"
import "unsafe"
import "github.com/ethereum/go-ethereum/core/vm"

var jt = vm.NewPragueEOFInstructionSetForTesting()

func validateEOF(b []byte, initcode bool) int32 {
    c := vm.Container{}
    err := c.UnmarshalBinary(b, initcode)
    if err != nil {
        return 0
    }
    err = c.ValidateCode(&jt, initcode)
    if err != nil {
        return 0
    }
    return 1
}

//export geth_fzz_eof_validate
func geth_fzz_eof_validate(data *byte, size int) int32 {
    b := unsafe.Slice(data, size);
    rt := validateEOF(b, false)
    ic := validateEOF(b, true)
    return (ic << 1) | rt
}

func main() {
    fmt.Println("geth_fuzz main()")
}
