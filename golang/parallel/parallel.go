package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"runtime"
	"runtime/pprof"
	"runtime/trace"
	"strconv"
	"strings"
	"sync"
	"time"
)

type Node struct {
	T         float64
	TSum      float64
	X         float64
	Y         float64
	Fixed     bool
	Neighbors float64
}

const PROFILING = false
const NSTEPS = 1000
const Cond = 413
const dX = 0.005
const dY = 0.005
const dT = 0.0005
const TDefault = 300
const EPS = 0.0001

var meshPts int

func SetLen(length int) {
	meshPts = length
}

func InitBar(n int, leftTemp float64, rightTemp float64) []Node {
	nodes := make([]Node, n)

	for k := range nodes {
		nodes[k].T = TDefault
		nodes[k].TSum = 0
		nodes[k].X = float64(k) * dX
		nodes[k].Y = 0
		nodes[k].Fixed = false
		nodes[k].Neighbors = 2.0
	}

	nodes[0].FixTemp(leftTemp)
	nodes[0].Neighbors = 1.0
	nodes[n-1].FixTemp(rightTemp)
	nodes[n-1].Neighbors = 1.0

	return nodes
}

func InitSquare(n int, topLeftTemp float64, bottomRightTemp float64) []Node {
	nodes := make([]Node, n*n)

	for k := range nodes {
		i, j := k%n, k/n
		nodes[k].T = TDefault
		nodes[k].TSum = 0
		nodes[k].X = float64(i) * dX
		nodes[k].Y = float64(j) * dY
		nodes[k].Fixed = false

		neighbors := 0
		if i < n-1 {
			neighbors += 1
		}
		if i > 0 {
			neighbors += 1
		}
		if j < n-1 {
			neighbors += 1
		}
		if j > 0 {
			neighbors += 1
		}
		nodes[k].Neighbors = float64(neighbors)
	}

	nodes[0].FixTemp(topLeftTemp)
	nodes[n*n-1].FixTemp(bottomRightTemp)

	return nodes
}

func (node *Node) FixTemp(temp float64) {
	node.T = temp
	node.Fixed = true
}

var cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file")
var nFlag = flag.Int("n", 1000, "Number of finite elements (or number of finite elements on side if square)")
var squareFlag = flag.Bool("square", false, "True if computing for a square object, false if for a bar")

func main() {
	flag.Parse()
	if PROFILING {
		if *cpuprofile != "" {
			f, err := os.Create(*cpuprofile)
			if err != nil {
				log.Fatal(err)
			}
			pprof.StartCPUProfile(f)
			defer pprof.StopCPUProfile()
		}
		f, err := os.Create("trace.out")
		if err != nil {
			panic(err)
		}
		defer f.Close()

		err = trace.Start(f)
		if err != nil {
			panic(err)
		}
		defer trace.Stop()
	}

	n := *nFlag
	SetLen(n)
	square := *squareFlag
	maxCpu := runtime.NumCPU()
	runtime.GOMAXPROCS(maxCpu)
	fmt.Println("Using", maxCpu, "processors")

	goPoolSize := maxCpu
	fmt.Println("Using", goPoolSize, "go routines")

	var nodes []Node
	var chans []chan float64
	var wg sync.WaitGroup
	sumQ := make(chan int)
	updateQ := make(chan int)

	if square {
		nodes = InitSquare(n, 400, 200)
	} else {
		nodes = InitBar(n, 400, 200)
	}

	chans = make([]chan float64, len(nodes))

	for i := range nodes {
		chans[i] = make(chan float64, len(nodes))
	}

	for i := 0; i < goPoolSize; i += 1 {
		go func() {
			for {
				idx := <-sumQ
				acc := 0.0
				if !square {
					if !nodes[idx].Fixed {
						if idx > 0 {
							acc += nodes[idx-1].T
						}
						if idx < n-1 {
							acc += nodes[idx+1].T
						}
						nodes[idx].TSum = acc
					}
				}
				wg.Done()
			}
		}()
	}

	for i := 0; i < goPoolSize; i += 1 {
		go func() {
			for {
				idx := <-updateQ
				if !nodes[idx].Fixed {
					nodes[idx].T = nodes[idx].TSum / nodes[idx].Neighbors
					nodes[idx].TSum = 0
				}
				wg.Done()
			}
		}()
	}

	startTime := time.Now()

	for step := 0; step < NSTEPS; step++ {
		wg.Add(len(nodes))
		for i := 0; i < len(nodes); i += 1 {
			sumQ <- i
		}
		wg.Wait()
		wg.Add(len(nodes))
		for i := 0; i < len(nodes); i += 1 {
			updateQ <- i
		}
		wg.Wait()
	}

	elapsedTime := time.Now().Sub(startTime)

	temps := make([]string, len(nodes))

	for i, node := range nodes {
		temps[i] = strconv.FormatFloat(node.T, 'f', 3, 64)
	}

	if square {
		fmt.Println("Final Temp:")
		for i := 0; i < n; i++ {
			fmt.Println(strings.Join(temps[i*n:(i+1)*n], " "))
		}
	} else {
		fmt.Println("Final Temp:", strings.Join(temps, " "))
	}
	fmt.Println("Time taken:", elapsedTime)
}
