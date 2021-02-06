FROM balenalib/armv7hf-debian:buster-build

RUN [ "cross-build-start" ]

ARG GO_URL
RUN curl -s -k $GO_URL -o go.tar.gz && tar -C /usr/local -xzf go.tar.gz && rm go.tar.gz

ARG LIBHEIF_VERSION
ARG LIBVIPS_VERSION
ARG BUILD_HEIF
RUN if [ "$BUILD_HEIF" = "true" ] ; \
  then \
    apt-get update && apt-get -y install libexpat1-dev libfftw3-dev liblcms2-dev libwebp-dev libtiff-dev libexif-dev libpng-dev libturbojpeg0-dev libglib2.0-dev libimagequant-dev libffi-dev libpcre3-dev libde265-dev liborc-0.4-dev; \
    cd /root; curl -s -L -k -o libheif-$LIBHEIF_VERSION.tar.gz https://github.com/strukturag/libheif/releases/download/v$LIBHEIF_VERSION/libheif-$LIBHEIF_VERSION.tar.gz; tar -zxf libheif-$LIBHEIF_VERSION.tar.gz; cd libheif-$LIBHEIF_VERSION; ./autogen.sh; ./configure --prefix=/usr/local/; make; make install; \
  else \
    apt-get update && apt-get -y install libexpat1-dev libfftw3-dev liblcms2-dev libwebp-dev libtiff-dev libexif-dev libpng-dev libturbojpeg0-dev libglib2.0-dev libimagequant-dev libffi-dev libpcre3-dev libde265-dev liborc-0.4-dev libheif-dev; \
  fi

RUN cd /root; curl -s -L -k -o vips-$LIBVIPS_VERSION.tar.gz https://github.com/libvips/libvips/releases/download/v$LIBVIPS_VERSION/vips-$LIBVIPS_VERSION.tar.gz; tar -zxf vips-$LIBVIPS_VERSION.tar.gz; cd vips-$LIBVIPS_VERSION; ./autogen.sh; ./configure --prefix=/usr/local/ --without-gsf  --without-magick  --without-OpenEXR --without-nifti --without-pdfium  --without-poppler --without-rsvg --without-openslide --without-matio  --without-cfitsio --without-pangoft2 --with-imagequant --with-heif --with-libwebp --with-tiff --with-giflib --with-png --with-jpeg; make; make install

ENV PATH /usr/bin:/bin:/usr/local/go/bin
ENV ROOT /usr/local/go
ENV GOPATH /root
ENV LD_LIBRARY_PATH /usr/local/lib

ARG SRC_DIR

RUN mkdir -p $SRC_DIR
COPY go.sum go.mod main.go $SRC_DIR/
COPY vendor $SRC_DIR/vendor
COPY test-images $SRC_DIR/test-images

RUN cd $SRC_DIR; go build

RUN [ "cross-build-end" ]

WORKDIR $SRC_DIR
