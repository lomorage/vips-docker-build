#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>
#include <vips/foreign.h>

void
gobject_set_property(VipsObject * object, const char *name,
		     const GValue * value)
{
	VipsObjectClass *object_class = VIPS_OBJECT_GET_CLASS(object);
	GType type = G_VALUE_TYPE(value);

	GParamSpec *pspec;
	VipsArgumentClass *argument_class;
	VipsArgumentInstance *argument_instance;

	if (vips_object_get_argument(object, name,
				     &pspec, &argument_class,
				     &argument_instance)) {
		g_warning("gobject warning: %s", vips_error_buffer());
		vips_error_clear();
		return;
	}

	if (G_IS_PARAM_SPEC_ENUM(pspec) && type == G_TYPE_STRING) {
		GType pspec_type = G_PARAM_SPEC_VALUE_TYPE(pspec);

		int enum_value;
		GValue value2 = { 0 };

		if ((enum_value = vips_enum_from_nick(object_class->nickname,
						      pspec_type,
						      g_value_get_string
						      (value))) < 0) {
			g_warning("gobject warning: %s", vips_error_buffer());
			vips_error_clear();
			return;
		}

		g_value_init(&value2, pspec_type);
		g_value_set_enum(&value2, enum_value);
		g_object_set_property(G_OBJECT(object), name, &value2);
		g_value_unset(&value2);
	} else {
		g_object_set_property(G_OBJECT(object), name, value);
	}
}

int
thumbnail(const char *inFilename, const char *outFilename, const char *opt,
	  int width, int height)
{
	printf("generate thumbnail: %s %dx%d\n", inFilename, width, height);
	GValue iFile = { 0, };
	GValue oFile = { 0, };
	GValue gw = { 0, };
	GValue gh = { 0, };
	GValue tn = { 0, };
	GValue out = { 0, };

	g_value_init(&iFile, G_TYPE_STRING);
	g_value_set_string(&iFile, inFilename);

	g_value_init(&oFile, G_TYPE_STRING);
	g_value_set_string(&oFile, outFilename);

	g_value_init(&tn, vips_image_get_type());

	g_value_init(&gw, G_TYPE_INT);
	g_value_set_int(&gw, width);

	g_value_init(&gh, G_TYPE_INT);
	g_value_set_int(&gh, height);

	VipsOperation *optThumbnail = vips_operation_new("thumbnail");
	gobject_set_property((VipsObject *) optThumbnail, "filename", &iFile);
	gobject_set_property((VipsObject *) optThumbnail, "width", &gw);
	gobject_set_property((VipsObject *) optThumbnail, "height", &gh);

	printf("vips_cache_operation_buildp %s thumbnail: ", inFilename);
	int ret = vips_cache_operation_buildp(&optThumbnail);
	if (ret != 0) {
		vips_error_buffer();
		return ret;
	}
	printf("success\n");
	g_object_get_property((GObject *) optThumbnail, "out", &tn);

	g_object_unref((gpointer) optThumbnail);

	VipsImage *imgOut = (VipsImage *) g_value_get_object(&tn);
	g_value_unset(&iFile);
	g_value_unset(&gw);
	g_value_unset(&gh);
	g_value_unset(&tn);

#ifdef RM_METADATA
	if (vips_image_remove(imgOut, "jpeg-thumbnail-data") != 1) {
		printf("remove jpeg-thumbnail-data from %s not success\n",
		       inFilename);
	}
	if (vips_image_remove(imgOut, "xmp-data") != 1) {
		printf("remove xmp-data from %s not success\n", inFilename);
	}
	if (vips_image_remove(imgOut, "iptc-data") != 1) {
		printf("remove iptc-data from %s not success\n", inFilename);
	}
	if (vips_image_remove(imgOut, "icc-profile-data") != 1) {
		printf("remove icc-profile-data from %s not success\n",
		       inFilename);
	}
#endif

	g_value_init(&out, vips_image_get_type());
	g_value_set_object(&out, (gpointer) imgOut);

	VipsOperation *optSave = vips_operation_new(opt);
	gobject_set_property((VipsObject *) optSave, "filename", &oFile);
	gobject_set_property((VipsObject *) optSave, "in", &out);

	printf("vips_cache_operation_buildp %s %s: ", inFilename, opt);
	ret = vips_cache_operation_buildp(&optSave);
	if (ret != 0) {
		vips_error_buffer();
		return ret;
	}
	printf("success\n");
	g_object_unref((gpointer) optSave);
	g_value_unset(&oFile);
	g_value_unset(&out);

	g_object_unref(imgOut);

	return 0;
}

typedef struct dim {
	int width;
	int height;
} dim;

int main(int argc, char *argv[])
{
	int i = 0;
	char *imageType[] =
	    { "jpegload", "pngload", "heifload", "webpload", "tiffload" };
	char *images[] =
	    { "1.webp", "20180330_3.heic", "img_4479.heic", "sample1.dng",
		"true_2003_01_17.jpg", "true_2003_11_01_1.jpg",
		"true_2003_11_01_2.jpg",
		"true_2003_11_23.jpg", "true_2004_1_21.jpg"
	};
	dim dims[3] = {
		{.width = 480,.height = 320},
		{.width = 75,.height = 75},
		{.width = 320,.height = 0}
	};
	int ret = vips_init("govips");
	if (ret != 0) {
		printf("vips_init failure\n");
		return ret;
	}

	vips_concurrency_set(1);
	vips_cache_set_max(0);
	vips_cache_set_max_mem(0);
	vips_leak_set(1);

	for (i = 0; i < 5; i++) {
		if (vips_type_find("VipsOperation", imageType[i]) != 0) {
			continue;
		}
		printf("find %s failure\n", imageType[i]);
		return ret;
	}
	for (i = 0; i < 9; i++) {
		for (int j = 0; j < 3; j++) {
			char baseDir[100] = "./test-images/";
			ret =
			    thumbnail(strcat(baseDir, images[i]), "1.jpg",
				      "jpegsave", dims[j].width,
				      dims[j].height);
			if (ret != 0) {
				return ret;
			}
		}
	}
	for (int j = 0; j < 3; j++) {
		ret =
		    thumbnail("./test-images/true_2018_07_28.png",
			      "1.png", "pngsave", dims[j].width,
			      dims[j].height);
		if (ret != 0) {
			return ret;
		}
	}
	return 0;
}
