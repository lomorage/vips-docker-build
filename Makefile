.PHONY: test-vips-go test-vips-c

SHELL=/bin/bash # Use bash syntax
VIPS_BUILD_HEIF=true
LIBHEIF_VERSION=1.10.0
LIBVIPS_VERSION=8.10.5
GO_VERSION=1.12.17
GO_ARM_V7_URL=https://dl.google.com/go/go${GO_VERSION}.linux-armv6l.tar.gz
SUFFIX_BUILD=build
PREFIX_LOMO_VIPS=lomorage/vips
SRC_DIR=/root/src/github.com/lomorage/vips-docker-build

clean:
	rm -rf artifacts

init:
	mkdir -p artifacts

build-vips:
	docker build \
		--build-arg SRC_DIR=${SRC_DIR} \
		--build-arg GO_URL=${GO_ARM_V7_URL} \
		--build-arg BUILD_HEIF=${VIPS_BUILD_HEIF} \
		--build-arg LIBHEIF_VERSION=${LIBHEIF_VERSION} \
		--build-arg LIBVIPS_VERSION=${LIBVIPS_VERSION} \
		--tag ${PREFIX_LOMO_VIPS} .

test-vips-go:
	docker run --rm --name test-vips \
		-it ${PREFIX_LOMO_VIPS} ./test-vips-go/test-vips

test-vips-c:
	docker run --rm --name test-vips \
		-it ${PREFIX_LOMO_VIPS} ./test-vips-c/test-vips
