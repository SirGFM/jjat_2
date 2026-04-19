package main

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"strconv"
	"strings"
)

func parseInt(value string) int {
	i, err := strconv.ParseInt(value, 0, 64)
	unwrap("failed to parse the value", err)

	return int(i)
}

type Terrain struct {
	Name  string
	Tiles []int
}

func (t *TiledTileset) ToTerrains() []Terrain {
	tileSets := make([][]int, len(t.Terrains.Types))

	for _, tile := range t.Terrains.Tiles {
		idx := parseInt(strings.Split(tile.TypeIdx, ",")[1]) - 1

		tileSets[idx] = append(tileSets[idx], parseInt(tile.TileId))
	}

	terrains := make([]Terrain, len(tileSets))
	for i, tiles := range tileSets {
		terrains[i] = Terrain{
			Name:  t.Terrains.Types[i].Name,
			Tiles: tiles,
		}
	}

	return terrains
}

type Tileset struct {
	FirstTile int
	Width     int
	Height    int
}

func (t *TiledTileset) ToTileset() Tileset {
	return Tileset{
		FirstTile: parseInt(t.FirstTile) - 1,
		Width:     parseInt(t.Width),
		Height:    parseInt(t.Height),
	}
}

func (t *TiledProperties) Get(name string) (Property, error) {
	for _, p := range t.Properties {
		if p.Name == name {
			return p, nil
		}
	}
	return Property{}, errors.New("property not found")
}

func (p *Property) AsInt() int {
	return parseInt(p.Value)
}

type Layer struct {
	Name   string
	Width  int
	Height int
	Data   []int
	HasAny bool
}

func (t *TiledLayer) ToLayer() Layer {
	fullW := parseInt(t.Width)

	tmp, err := t.TiledProperties.Get("width")
	unwrap("failed to get layer width", err)
	w := tmp.AsInt() + 1

	tmp, err = t.TiledProperties.Get("height")
	unwrap("failed to get layer height", err)
	h := tmp.AsInt() + 1

	data := make([]int, w*h)

	splitFn := func(data []byte, atEOF bool) (advance int, token []byte, err error) {
		// Based on bufio.ScanLines
		if atEOF && len(data) == 0 {
			return 0, nil, nil
		}
		if i := bytes.IndexByte(data, ','); i >= 0 {
			// We have a full newline-terminated line.
			return i + 1, data[0:i], nil
		}
		// If we're at EOF, we have a final, non-terminated line. Return it.
		if atEOF {
			return len(data), data, nil
		}
		// Request more data.
		return 0, nil, nil
	}

	hasAny := false
	scanner := bufio.NewScanner(bytes.NewBuffer(t.Data))
	scanner.Split(splitFn)
	for i := range w * h {
		ok := scanner.Scan()
		if !ok {
			panic(fmt.Sprintf("incomplete data at '%s'", t.Name))
		}
		unwrap("failed to parse layer tiles", scanner.Err())

		data[i] = parseInt(strings.TrimSpace(scanner.Text())) - 1
		hasAny = hasAny || data[i] >= 0

		if (i+1)%w == 0 {
			// Skip remaining entries at the end of a line.
			for _ = range fullW - w {
				ok := scanner.Scan()
				if !ok {
					panic(fmt.Sprintf("incomplete data at '%s'", t.Name))
				}
				unwrap("failed to parse layer tiles", scanner.Err())
			}
		}
	}

	return Layer{
		Name:   t.Name,
		Width:  w,
		Height: h,
		Data:   data,
		HasAny: hasAny,
	}
}

type Object struct {
	Name       string
	Type       string
	Width      int
	Height     int
	X          int
	Y          int
	Tile       int
	Properties []Property
}

func (t *TiledObject) ToObject() Object {
	tile := -1
	if t.Gid != "" {
		tile = parseInt(t.Gid) - 1
	}

	return Object{
		Name:       t.Name,
		Type:       t.Type,
		Width:      parseInt(t.Width),
		Height:     parseInt(t.Height),
		X:          parseInt(t.X),
		Y:          parseInt(t.Y),
		Tile:       tile,
		Properties: t.Properties,
	}
}
