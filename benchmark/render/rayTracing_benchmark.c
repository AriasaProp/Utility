#include "render/rayTracing.h"
#include "stb/local.h"
#include "stb/image_write.h"
#include "math/vector.h"
#include "math/mat4.h"
#include "util/console_out.h"
#include "common.h"
#include <sys/stat.h>
#include <errno.h>

static void add_box(Objects *);

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
	const struct {
	  const char *name;
	  uvec2 dim;
	} dims[] = {
	  { .name = "QQVGA", .dim = UVECTOR2(160, 120)},
	  // { .name =  "QVGA", .dim = UVECTOR2(320, 240)},
	  // { .name =  "HVGA", .dim = UVECTOR2(480, 320)},
	  // { .name =  "WVGA", .dim = UVECTOR2(800, 480)},
	  // {"FWVGA", UVECTOR2(854, 480)},
	  // {"HD_720p", UVECTOR2(1280, 720)},
	  // {"WXGA", UVECTOR2(1280, 800)},
	  // {"FHD_1080p", UVECTOR2(1920, 1080)},
	  // {"2K", UVECTOR2(2048, 1080)},
	  // {"QHD_WQHD", UVECTOR2(2560, 1440)},
	  // {"UHD_4K", UVECTOR2(3840, 2160)},
	  // {"5K", UVECTOR2(5120, 2880)},
	  // {"8K", UVECTOR2(7680, 4320)},
	};
	
	
	PRINT_INF("rayTracing benchmark ");
	int res = EXIT_FAILURE;
	iter i;
	char temp[257];
	ubyte bin[800*480*3];
	image_bitmap ib = {0};
	ib.data = bin;
	const char *RenderFolder = "bin/render";
  int result =
#ifdef _WIN32
  	_mkdir(RenderFolder);
#else
		mkdir(RenderFolder, 0755);
#endif
  if (result < 0) {
    switch (errno) {
    case EEXIST: break;
    case ENOTDIR:
    case ENOENT:
    	printf(RED"bin doesn't exists!\n"RESET);
      goto end;
    default:
      printf(RED"could not create directory %s\n"RESET, strerror(errno));
      goto end;
    }
  }

	Objects obj = {0};
	add_box(&obj);
	for (i = 0; i < STACK_ARR_LEN(dims); ++i) {
		ib.w = dims[i].dim.x;
		ib.h = dims[i].dim.y;
		ib.bpc = 8;
		ib.chnl = 3;
		if (!rayTracing_render(bin, obj, dims[i].dim)) {
			printf("rayTracing failure!\n");
			goto end;
		}
		snprintf(temp, 256, "%s/%s_black.png", RenderFolder, dims[i].name);
		if (!image_write(temp, ib, ImageFile_PNG)) {
			printf("image write failure! %s \n", stb_get_error());
			goto end;
		}
	}
	res = EXIT_SUCCESS;
end:
	darray_foreach(Object, io, &obj) {
		darray_free(&(io->vert));
		darray_free(&(io->idx));
	}
	darray_free(&obj);
	return res;
}

static void add_box(Objects *objs) {
	Object o = {0};
	o.trans = MAT4_IDT;
	darray_appends(&(o.vert), CLIT(vec3[]){
		VECTOR3( 10.0f, 20.0f, 10.0f),
		VECTOR3(-10.0f, 20.0f, 10.0f),
		VECTOR3( 10.0f, 20.0f,-10.0f),
		VECTOR3(-10.0f, 20.0f,-10.0f), // 3
		VECTOR3( 10.0f, 00.0f, 10.0f),
		VECTOR3(-10.0f, 00.0f, 10.0f),
		VECTOR3( 10.0f, 00.0f,-10.0f), // 6
		VECTOR3(-10.0f, 00.0f,-10.0f),
	}, 8);
	darray_appends(&(o.idx), CLIT(ushrt[]){
		0, 2, 1,     1, 3, 0, // top
		0, 1, 4,     1, 5, 4, // back
		2, 6, 7,     7, 3, 2, // front
		4, 5, 7,     7, 6, 4, //bottom
		2, 0, 4,     4, 6, 2, // right
		1, 3, 7,     7, 5, 1, // left
	}, 12);
	darray_append(objs, o);
}