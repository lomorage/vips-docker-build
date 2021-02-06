package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"path/filepath"
	"strings"

	"github.com/leslie-wang/govips/pkg/vips"
	"github.com/pkg/errors"
)

const (
	imgDir   = "test-images"
	metaFile = "meta.json"
)

type dimension struct {
	width  int
	height int
}

var dims = []dimension{{width: 480, height: 320}, {width: 75, height: 75}, {width: 320}}

func main() {
	vips.Startup(&vips.Config{
		ConcurrencyLevel: 1,
		ReportLeaks:      true,
		MaxCacheFiles:    0,
		MaxCacheMem:      0,
		MaxCacheSize:     0,
	})
	defer vips.Shutdown()

	baseDir := "./test-images"
	files, err := ioutil.ReadDir(baseDir)
	if err != nil {
		log.Fatal(err)
	}
	for _, f := range files {
		if f.IsDir() || strings.Contains(filepath.Join(baseDir, f.Name()), "thumbnail") {
			continue
		}
		for _, dim := range dims {
			if err := generateThumbnail(f.Name(), dim.width, dim.height); err != nil {
				log.Fatal(err)
			}
		}
	}
}

func vipsHeightOptions(height int) []*vips.Option {
	options := []*vips.Option{}
	if height != 0 {
		options = append(options,
			vips.InputInt("height", int(height)),
			vips.InputString("size", "force"),
		)
	}
	return options
}

func generateThumbnail(assetpath string, width, height int) error {
	outImage, err := vips.Thumbnail(assetpath, width, vipsHeightOptions(height)...)
	if err != nil {
		if outImage != nil {
			vips.FreeImage(outImage)
		}
		return errors.Wrap(err, "while generating png thumbnail")
	}
	defer vips.FreeImage(outImage)

	vips.RemoveImageMetadata(outImage, "xmp-data")
	vips.RemoveImageMetadata(outImage, "iptc-data")
	vips.RemoveImageMetadata(outImage, "icc-profile-data")

	thumbnailpath := fmt.Sprintf("%s_%d_%d", assetpath, width, height)
	if filepath.Ext(assetpath) == ".png" {
		thumbnailpath += "_thumbnail.png"
		log.Printf("start save png: %s\n", thumbnailpath)
		if err := vips.Pngsave(outImage, thumbnailpath); err != nil {
			return errors.Wrap(err, "while saving png thumbnail")
		}
		log.Printf("finish save jpeg: %s\n", thumbnailpath)
	} else {
		thumbnailpath += "_thumbnail.jpg"
		fmt.Printf("start save jpeg: %s\n", thumbnailpath)
		if err := vips.Jpegsave(outImage, thumbnailpath); err != nil {
			return errors.Wrap(err, "while saving jpeg thumbnail")
		}
		log.Printf("finish save jpeg: %s\n", thumbnailpath)
	}
	return nil
}
