// The build command:
// go build -v -o geth.a -buildmode=c-archive -tags=libfuzzer -gcflags=all=-d=libfuzzer

package main

import "C"
import "fmt"

func main() {
    fmt.Println("geth fzz main()")
}
