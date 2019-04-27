package main

import (
	"fmt"
	"math"
	"strconv"
	"strings"
)

type Node struct {
	T     float64
	TSum  float64
	X     float64
	Fixed bool
}

const NSTEPS = 300
const Cond = 413
const dX = 0.005
const dT = 0.0005
const TDefault = 300

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
		nodes[k].Fixed = false
	}

	// Left end
	nodes[0].T = leftTemp
	nodes[0].Fixed = true

	// Right end
	nodes[meshPts-1].T = rightTemp
	nodes[meshPts-1].Fixed = true

	return nodes
}

func (node *Node) ApplyTsum(neighbor Node) {
	if node.Fixed {
		return
	}

	dist := math.Abs(node.X - neighbor.X)

	if dist <= dX+0.001 && dist != 0 {
		node.TSum += neighbor.T
	}
}

func (node *Node) TUpdate() {
	if node.Fixed {
		return
	}
	node.T = node.TSum / 2.0
	node.TSum = 0
}

func main() {
	n := 1000
	SetLen(n)
	nodes := InitBar(n, 400, 200)

	for step := 0; step < NSTEPS; step++ {
		for k := range nodes {
			for _, nodeB := range nodes {
				nodes[k].ApplyTsum(nodeB)
			}
		}

		for k := range nodes {
			nodes[k].TUpdate()
		}

	}

	temps := make([]string, n)

	for i, node := range nodes {
		temps[i] = strconv.FormatFloat(node.T, 'f', 3, 64)
	}
	fmt.Println("Final Temp:", strings.Join(temps, " "))
}
