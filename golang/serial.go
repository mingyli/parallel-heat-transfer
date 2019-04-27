package main

import (
	"fmt"
	"math"
	"strconv"
	"strings"
)

type Node struct {
	T         float64
	TSum      float64
	X         float64
	Y         float64
	Fixed     bool
	Neighbors int
}

const NSTEPS = 20
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
	}

	nodes[0].FixTemp(leftTemp)
	nodes[meshPts-1].FixTemp(rightTemp)

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

func (node *Node) ApplyTsum(neighbor Node) {
	if node.Fixed {
		return
	}

	xDist := math.Abs(node.X - neighbor.X)
	yDist := math.Abs(node.Y - neighbor.Y)

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

func main() {
	n := 10
	SetLen(n)
	square := true
	var nodes []Node

	if square {
		nodes = InitSquare(n, 400, 200)
	} else {
		nodes = InitBar(n, 400, 200)
	}

	for step := 0; step < NSTEPS; step++ {
		for i := range nodes {
			for j, nodeB := range nodes {
				if i == j {
					continue
				}
				nodes[i].ApplyTsum(nodeB)
			}
		}

		for k := range nodes {
			nodes[k].TUpdate()
		}
	}

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
}
