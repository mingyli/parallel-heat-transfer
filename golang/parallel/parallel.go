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
	Neighbors int
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
		nodes[k].Neighbors = 2
	}

	nodes[0].FixTemp(leftTemp)
	nodes[0].Neighbors = 1
	nodes[n-1].FixTemp(rightTemp)
	nodes[n-1].Neighbors = 1

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
		nodes[k].Neighbors = neighbors
	}

	nodes[0].FixTemp(topLeftTemp)
	nodes[n*n-1].FixTemp(bottomRightTemp)

	return nodes
}

func (node *Node) FixTemp(temp float64) {
	node.T = temp
	node.Fixed = true
}

func (node *Node) Apply(ch *chan float64) {
	if node.Fixed {
		return
	}

	for i := 0; i < node.Neighbors; i++ {
		node.TSum += <-(*ch)
	}

	node.T = node.TSum / float64(node.Neighbors)
	node.TSum = 0
}

var cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file")

func main() {
	if PROFILING {
		flag.Parse()
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

	n := 100
	SetLen(n)
	square := false

	var nodes []Node
	var chans []chan float64
	var wg sync.WaitGroup

	if square {
		nodes = InitSquare(n, 400, 200)
	} else {
		nodes = InitBar(n, 400, 200)
	}

	chans = make([]chan float64, len(nodes))

	for i := range nodes {
		chans[i] = make(chan float64)
	}

	fmt.Println(runtime.NumCPU())
	runtime.GOMAXPROCS(8)

	startTime := time.Now()

	for step := 0; step < NSTEPS; step++ {
		// Send temperature to all other nodes
		/*
			for i := 0; i < len(nodes); i++ {
				for j := 0; j < len(nodes); j++ {
					if i == j || nodes[j].Fixed {
						continue
					}
					xDist := math.Abs(nodes[i].X - nodes[j].X)
					yDist := math.Abs(nodes[i].Y - nodes[j].Y)
					if (xDist < dX+EPS && yDist == 0.0) ||
						(yDist < dX+EPS && xDist == 0.0) {
						go func(temp float64, ch *chan float64) {
							*ch <- temp
						}(nodes[i].T, &chans[j])
					}
				}
			}
		*/

		if !square {
			go func(temp float64, ch *chan float64) {
				*ch <- temp
			}(nodes[1].T, &chans[0])

			for i := 1; i < n-1; i += 1 {
				go func(temp float64, ch *chan float64) {
					*ch <- temp
				}(nodes[i+1].T, &chans[i])
				go func(temp float64, ch *chan float64) {
					*ch <- temp
				}(nodes[i-1].T, &chans[i])
			}

			go func(temp float64, ch *chan float64) {
				*ch <- temp
			}(nodes[n-2].T, &chans[n-1])

		} else {
			for i := 0; i < n; i += 1 {
				for j := 0; j < n; j += 1 {
					for dx := -1; dx < 2; dx += 1 {
						for dy := -1; dy < 2; dy += 1 {
							if (dx == 0 && dy == 0) || (dx != 0 && dy != 0) {
								continue
							}
							if (i+dx < 0) || (i+dx >= n) || (j+dy < 0) || (j+dy >= n) {
								continue
							}
							idxA := j*n + i
							idxB := (j+dy)*n + (i + dx)
							go func(temp float64, ch *chan float64) {
								*ch <- temp
							}(nodes[idxB].T, &chans[idxA])
						}
					}
				}
			}
		}

		for i := 0; i < len(nodes); i++ {
			wg.Add(1)
			go func(node *Node, ch *chan float64) {
				defer wg.Done()
				node.Apply(ch)
			}(&nodes[i], &chans[i])
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
