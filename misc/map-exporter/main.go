package main

import (
	"bufio"
	"encoding/xml"
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

func unwrap(cause string, err error) {
	if err != nil {
		panic(fmt.Sprintf("%s: %+v", cause, err))
	}
}

func loadTiledFile(inFile string) TiledSaveFormat {
	var tmx TiledSaveFormat

	input, err := os.Open(inFile)
	unwrap("failed to open the input file", err)

	dec := xml.NewDecoder(input)
	err = dec.Decode(&tmx)
	unwrap("failed to decode tiled data", err)

	return tmx
}

func main() {
	var inFile string
	var outDir string

	flag.StringVar(&inFile, "i", "", "the input tiled save file (.tmx)")
	flag.StringVar(&outDir, "o", "", "the output directory")
	flag.Parse()

	if inFile == "" {
		panic("missing input file")
	} else if outDir == "" {
		panic("missing output directory")
	}

	tmx := loadTiledFile(inFile)

	var terrains []Terrain
	var tilesets []Tileset
	for i, _ := range tmx.Tilesets {
		terrains = append(terrains, tmx.Tilesets[i].ToTerrains()...)
		tilesets = append(tilesets, tmx.Tilesets[len(tmx.Tilesets)-i-1].ToTileset())
	}

	for _, tmp := range tmx.Layers {
		if tmp.Visible == "0" {
			continue
		}

		layer := tmp.ToLayer()
		fileName := filepath.Join(outDir, layer.Name+".gfm")

		func() {
			outFile, err := os.OpenFile(fileName, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
			unwrap("failed to open output file", err)
			defer func() {
				err := outFile.Close()
				unwrap("failed to close the file", err)
			}()

			output := bufio.NewWriter(outFile)
			defer func() {
				err := output.Flush()
				unwrap("failed to flush the file", err)
			}()

			if layer.HasAny {
				for _, terrain := range terrains {
					for _, tile := range terrain.Tiles {
						_, err := output.WriteString(fmt.Sprintf("type %s %d\n", terrain.Name, tile))
						unwrap("failed to write data", err)
					}
				}
			}

			_, err = output.WriteString(fmt.Sprintf("map %d %d\n", layer.Width, layer.Height))
			unwrap("failed to write data", err)
			for i, tile := range layer.Data {
				x := i % layer.Width

				if x == 0 {
					_, err = output.WriteString("  ")
					unwrap("failed to write data", err)
				}

				_, err = output.WriteString(fmt.Sprintf("%d", tile))
				unwrap("failed to write data", err)

				if x < layer.Width-1 {
					_, err = output.WriteString(" ")
					unwrap("failed to write data", err)
				}

				if i > 0 && x == layer.Width-1 {
					_, err = output.WriteString(" \n")
					unwrap("failed to write data", err)
				}
			}
		}()
	}

	for _, objectLayer := range tmx.ObjectLayers {
		if objectLayer.Visible == "0" {
			continue
		}

		fileName := filepath.Join(outDir, objectLayer.Name+".gfm")

		func() {
			outFile, err := os.OpenFile(fileName, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
			unwrap("failed to open output file", err)
			defer func() {
				err := outFile.Close()
				unwrap("failed to close the file", err)
			}()

			output := bufio.NewWriter(outFile)
			defer func() {
				err := output.Flush()
				unwrap("failed to flush the file", err)
			}()

			for _, tmp := range objectLayer.Objects {
				obj := tmp.ToObject()
				baseType := "obj"

				if obj.Tile < 0 && len(obj.Properties) == 0 {
					baseType = "area"
				}

				_, err := output.WriteString(
					fmt.Sprintf(
						"%s %s %d %d %d %d",
						baseType,
						obj.Type,
						obj.X,
						obj.Y,
						obj.Width,
						obj.Height,
					),
				)
				unwrap("failed to write data", err)

				for _, property := range obj.Properties {
					_, err := output.WriteString(fmt.Sprintf(" [ %s , %s ]", property.Name, property.Value))
					unwrap("failed to write data", err)
				}

				_, err = output.WriteString("\n")
				unwrap("failed to write data", err)
			}
		}()
	}
}
