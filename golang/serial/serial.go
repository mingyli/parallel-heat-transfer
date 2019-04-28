package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"runtime/pprof"
	"runtime/trace"
	"strconv"
	"strings"
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

const PROFILING = true
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

func (node *Node) ApplyTsum(neighbor *Node) {
	if node.Fixed {
		return
	}

	xDist := node.X - neighbor.X
	if xDist < 0 {
		xDist = -xDist
	}
	yDist := 0.0

	//yDist := node.Y - neighbor.Y
	//if yDist < 0 {
	//yDist = -yDist
	//}

	if (xDist < dX+EPS && yDist == 0.0) || (yDist < dX+EPS && xDist == 0.0) {
		node.TSum += neighbor.T
	}
}

func (node *Node) TUpdate() {
	if node.Fixed {
		return
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
	n := 10000
	SetLen(n)
	square := false
	var nodes []Node

	if square {
		nodes = InitSquare(n, 400, 200)
	} else {
		nodes = InitBar(n, 400, 200)
	}

	startTime := time.Now()

	for step := 0; step < NSTEPS; step++ {
		/*
			for i := 0; i < n; i += 1 {
				for j := 0; j < n; j += 1 {
					if i == j {
						continue
					}
					nodes[i].ApplyTsum(&nodes[j])
				}
			}
		*/
		if !square {
			nodes[0].ApplyTsum(&nodes[1])
			for i := 1; i < n-1; i += 1 {
				nodes[i].ApplyTsum(&nodes[i+1])
				nodes[i].ApplyTsum(&nodes[i-1])
			}
			nodes[n-1].ApplyTsum(&nodes[n-2])
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
							nodes[idxA].TSum += nodes[idxB].T
						}
					}
				}
			}
		}

		for i := 0; i < len(nodes); i += 1 {
			nodes[i].TUpdate()
		}
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
