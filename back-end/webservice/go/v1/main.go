package main

import (
	"fmt"
	"net/http"
)

func main() {
	fmt.Println("Sensoriamento - API")

	http.ListenAndServe(":8081", routes())
}
