package main

type TiledTileset struct {
	Name      string `xml:"name,attr"`
	FirstTile string `xml:"firstgid,attr"`
	Width     string `xml:"tilewidth,attr"`
	Height    string `xml:"tileheight,attr"`
	Terrains  struct {
		Types []struct {
			Name string `xml:"name,attr"`
		} `xml:"wangset>wangcolor"`
		Tiles []struct {
			TileId  string `xml:"tileid,attr"`
			TypeIdx string `xml:"wangid,attr"`
		} `xml:"wangset>wangtile"`
	} `xml:"wangsets"`
}

type Property struct {
	Name  string `xml:"name,attr"`
	Value string `xml:"value,attr"`
}

type TiledProperties struct {
	Properties []Property `xml:"properties>property"`
}

type TiledNode struct {
	Name    string `xml:"name,attr"`
	Visible string `xml:"visible,attr"`
}

type TiledLayer struct {
	TiledNode
	TiledProperties
	Width  string `xml:"width,attr"`
	Height string `xml:"height,attr"`
	Data   []byte `xml:"data"`
}

type TiledObject struct {
	Name   string `xml:"name,attr"`
	Type   string `xml:"type,attr"`
	Width  string `xml:"width,attr"`
	Height string `xml:"height,attr"`
	X      string `xml:"x,attr"`
	Y      string `xml:"y,attr"`
	Gid    string `xml:"gid,attr"`
	TiledProperties
}

type TiledObjectLayer struct {
	TiledNode
	Objects []TiledObject `xml:"object"`
}

type TiledSaveFormat struct {
	Tilesets     []TiledTileset     `xml:"tileset"`
	Layers       []TiledLayer       `xml:"layer"`
	ObjectLayers []TiledObjectLayer `xml:"objectgroup"`
}
