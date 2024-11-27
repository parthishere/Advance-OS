/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */

/* THIS FILE IS AUTOGENERATED BY BPFTOOL! */
#ifndef __MINIMAL_BPF_SKEL_H__
#define __MINIMAL_BPF_SKEL_H__

#include <errno.h>
#include <stdlib.h>
#include <bpf/libbpf.h>

struct minimal_bpf {
	struct bpf_object_skeleton *skeleton;
	struct bpf_object *obj;
	struct {
		struct bpf_map *rb;
		struct bpf_map *bss;
		struct bpf_map *rodata;
	} maps;
	struct {
		struct bpf_program *handle_xdp;
	} progs;
	struct {
		struct bpf_link *handle_xdp;
	} links;

#ifdef __cplusplus
	static inline struct minimal_bpf *open(const struct bpf_object_open_opts *opts = nullptr);
	static inline struct minimal_bpf *open_and_load();
	static inline int load(struct minimal_bpf *skel);
	static inline int attach(struct minimal_bpf *skel);
	static inline void detach(struct minimal_bpf *skel);
	static inline void destroy(struct minimal_bpf *skel);
	static inline const void *elf_bytes(size_t *sz);
#endif /* __cplusplus */
};

static void
minimal_bpf__destroy(struct minimal_bpf *obj)
{
	if (!obj)
		return;
	if (obj->skeleton)
		bpf_object__destroy_skeleton(obj->skeleton);
	free(obj);
}

static inline int
minimal_bpf__create_skeleton(struct minimal_bpf *obj);

static inline struct minimal_bpf *
minimal_bpf__open_opts(const struct bpf_object_open_opts *opts)
{
	struct minimal_bpf *obj;
	int err;

	obj = (struct minimal_bpf *)calloc(1, sizeof(*obj));
	if (!obj) {
		errno = ENOMEM;
		return NULL;
	}

	err = minimal_bpf__create_skeleton(obj);
	if (err)
		goto err_out;

	err = bpf_object__open_skeleton(obj->skeleton, opts);
	if (err)
		goto err_out;

	return obj;
err_out:
	minimal_bpf__destroy(obj);
	errno = -err;
	return NULL;
}

static inline struct minimal_bpf *
minimal_bpf__open(void)
{
	return minimal_bpf__open_opts(NULL);
}

static inline int
minimal_bpf__load(struct minimal_bpf *obj)
{
	return bpf_object__load_skeleton(obj->skeleton);
}

static inline struct minimal_bpf *
minimal_bpf__open_and_load(void)
{
	struct minimal_bpf *obj;
	int err;

	obj = minimal_bpf__open();
	if (!obj)
		return NULL;
	err = minimal_bpf__load(obj);
	if (err) {
		minimal_bpf__destroy(obj);
		errno = -err;
		return NULL;
	}
	return obj;
}

static inline int
minimal_bpf__attach(struct minimal_bpf *obj)
{
	return bpf_object__attach_skeleton(obj->skeleton);
}

static inline void
minimal_bpf__detach(struct minimal_bpf *obj)
{
	bpf_object__detach_skeleton(obj->skeleton);
}

static inline const void *minimal_bpf__elf_bytes(size_t *sz);

static inline int
minimal_bpf__create_skeleton(struct minimal_bpf *obj)
{
	struct bpf_object_skeleton *s;
	int err;

	s = (struct bpf_object_skeleton *)calloc(1, sizeof(*s));
	if (!s)	{
		err = -ENOMEM;
		goto err;
	}

	s->sz = sizeof(*s);
	s->name = "minimal_bpf";
	s->obj = &obj->obj;

	/* maps */
	s->map_cnt = 3;
	s->map_skel_sz = sizeof(*s->maps);
	s->maps = (struct bpf_map_skeleton *)calloc(s->map_cnt, s->map_skel_sz);
	if (!s->maps) {
		err = -ENOMEM;
		goto err;
	}

	s->maps[0].name = "rb";
	s->maps[0].map = &obj->maps.rb;

	s->maps[1].name = "minimal_.bss";
	s->maps[1].map = &obj->maps.bss;

	s->maps[2].name = "minimal_.rodata";
	s->maps[2].map = &obj->maps.rodata;

	/* programs */
	s->prog_cnt = 1;
	s->prog_skel_sz = sizeof(*s->progs);
	s->progs = (struct bpf_prog_skeleton *)calloc(s->prog_cnt, s->prog_skel_sz);
	if (!s->progs) {
		err = -ENOMEM;
		goto err;
	}

	s->progs[0].name = "handle_xdp";
	s->progs[0].prog = &obj->progs.handle_xdp;
	s->progs[0].link = &obj->links.handle_xdp;

	s->data = minimal_bpf__elf_bytes(&s->data_sz);

	obj->skeleton = s;
	return 0;
err:
	bpf_object__destroy_skeleton(s);
	return err;
}

static inline const void *minimal_bpf__elf_bytes(size_t *sz)
{
	static const char data[] __attribute__((__aligned__(8))) = "\
\x7f\x45\x4c\x46\x02\x01\x01\0\0\0\0\0\0\0\0\0\x01\0\xf7\0\x01\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\x88\x1f\0\0\0\0\0\0\0\0\0\0\x40\0\0\0\0\0\x40\0\x0d\0\
\x01\0\0\x2e\x73\x74\x72\x74\x61\x62\0\x2e\x73\x79\x6d\x74\x61\x62\0\x2e\x74\
\x65\x78\x74\0\x78\x64\x70\0\x6c\x69\x63\x65\x6e\x73\x65\0\x2e\x62\x73\x73\0\
\x2e\x72\x6f\x64\x61\x74\x61\0\x2e\x6d\x61\x70\x73\0\x6d\x69\x6e\x69\x6d\x61\
\x6c\x2e\x62\x70\x66\x2e\x63\0\x4c\x42\x42\x30\x5f\x34\0\x73\x79\x6e\x5f\x63\
\x6f\x75\x6e\x74\0\x6c\x61\x73\x74\x5f\x73\x79\x6e\x5f\x74\x69\x6d\x65\0\x68\
\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\0\x4c\x42\
\x42\x31\x5f\x34\0\x4c\x42\x42\x31\x5f\x35\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\
\x64\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x31\0\x4c\x42\x42\x31\x5f\x32\x30\
\0\x4c\x42\x42\x31\x5f\x32\x31\0\x4c\x42\x42\x31\x5f\x37\0\x68\x61\x6e\x64\x6c\
\x65\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x32\0\x4c\x42\x42\x31\
\x5f\x39\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\
\x74\x2e\x33\0\x4c\x42\x42\x31\x5f\x31\x31\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\
\x64\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x34\0\x4c\x42\x42\x31\x5f\x31\x35\
\0\x4c\x42\x42\x31\x5f\x31\x37\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\
\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x35\0\x4c\x42\x42\x31\x5f\x31\x39\0\x4c\x42\
\x42\x31\x5f\x31\x38\0\x4c\x42\x42\x31\x5f\x32\x32\0\x68\x61\x6e\x64\x6c\x65\
\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x37\0\x68\x61\x6e\x64\x6c\
\x65\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x36\0\x69\x73\x5f\x73\
\x79\x6e\x5f\x66\x6c\x6f\x6f\x64\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\0\
\x72\x62\0\x4c\x49\x43\x45\x4e\x53\x45\0\x2e\x72\x65\x6c\x2e\x74\x65\x78\x74\0\
\x2e\x72\x65\x6c\x78\x64\x70\0\x2e\x42\x54\x46\0\x2e\x42\x54\x46\x2e\x65\x78\
\x74\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x36\0\0\0\x04\0\xf1\xff\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\0\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\0\x44\0\0\0\0\0\x03\0\xb0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x4b\0\0\0\x01\0\
\x06\0\0\0\0\0\0\0\0\0\x04\0\0\0\0\0\0\0\x55\0\0\0\x01\0\x06\0\x08\0\0\0\0\0\0\
\0\x08\0\0\0\0\0\0\0\0\0\0\0\x03\0\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x63\0\
\0\0\x01\0\x07\0\0\0\0\0\0\0\0\0\x12\0\0\0\0\0\0\0\x76\0\0\0\0\0\x04\0\x88\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\x7d\0\0\0\0\0\x04\0\xb8\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\x84\0\0\0\x01\0\x07\0\x12\0\0\0\0\0\0\0\x10\0\0\0\0\0\0\0\x99\0\0\0\0\0\x04\
\0\xa0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xa1\0\0\0\0\0\x04\0\xa8\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\xa9\0\0\0\0\0\x04\0\xf8\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xb0\0\0\0\
\x01\0\x07\0\x22\0\0\0\0\0\0\0\x2d\0\0\0\0\0\0\0\xc5\0\0\0\0\0\x04\0\x28\x01\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\xcc\0\0\0\x01\0\x07\0\x4f\0\0\0\0\0\0\0\x37\0\0\0\0\
\0\0\0\xe1\0\0\0\0\0\x04\0\x60\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xe9\0\0\0\x01\0\
\x07\0\x86\0\0\0\0\0\0\0\x25\0\0\0\0\0\0\0\xfe\0\0\0\0\0\x04\0\x10\x02\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\x06\x01\0\0\0\0\x04\0\x48\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\x0e\x01\0\0\x01\0\x07\0\xab\0\0\0\0\0\0\0\x42\0\0\0\0\0\0\0\x23\x01\0\0\0\0\
\x04\0\x10\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x2b\x01\0\0\0\0\x04\0\x80\x02\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\x33\x01\0\0\0\0\x04\0\xc8\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\x3b\x01\0\0\x01\0\x07\0\x0f\x01\0\0\0\0\0\0\x1f\0\0\0\0\0\0\0\x50\x01\0\0\
\x01\0\x07\0\xed\0\0\0\0\0\0\0\x22\0\0\0\0\0\0\0\0\0\0\0\x03\0\x06\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\0\x07\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x65\
\x01\0\0\x12\0\x03\0\0\0\0\0\0\0\0\0\xc0\0\0\0\0\0\0\0\x72\x01\0\0\x12\0\x04\0\
\0\0\0\0\0\0\0\0\x30\x03\0\0\0\0\0\0\x7d\x01\0\0\x11\0\x08\0\0\0\0\0\0\0\0\0\
\x10\0\0\0\0\0\0\0\x80\x01\0\0\x11\0\x05\0\0\0\0\0\0\0\0\0\x0d\0\0\0\0\0\0\0\
\x69\x12\x0c\0\0\0\0\0\x57\x02\0\0\0\x02\0\0\x15\x02\x13\0\0\0\0\0\x69\x11\x0c\
\0\0\0\0\0\x57\x01\0\0\0\x10\0\0\x55\x01\x10\0\0\0\0\0\x18\x06\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\x61\x61\0\0\0\0\0\0\x07\x01\0\0\x01\0\0\0\x63\x16\0\0\0\0\0\0\x85\
\0\0\0\x05\0\0\0\x18\x01\0\0\x08\0\0\0\0\0\0\0\0\0\0\0\x79\x12\0\0\0\0\0\0\xbf\
\x03\0\0\0\0\0\0\x1f\x23\0\0\0\0\0\0\xb7\x02\0\0\x01\xca\x9a\x3b\x2d\x32\x03\0\
\0\0\0\0\x7b\x01\0\0\0\0\0\0\xb7\x01\0\0\0\0\0\0\x63\x16\0\0\0\0\0\0\xb7\0\0\0\
\x01\0\0\0\x95\0\0\0\0\0\0\0\x61\x17\x04\0\0\0\0\0\x61\x18\0\0\0\0\0\0\xbf\x73\
\0\0\0\0\0\0\x1f\x83\0\0\0\0\0\0\x18\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xb7\x02\0\
\0\x12\0\0\0\x85\0\0\0\x06\0\0\0\xbf\x86\0\0\0\0\0\0\x07\x06\0\0\x0e\0\0\0\x2d\
\x76\x06\0\0\0\0\0\x69\x81\x0c\0\0\0\0\0\x55\x01\x04\0\x08\0\0\0\x07\x08\0\0\
\x22\0\0\0\x2d\x78\x02\0\0\0\0\0\x71\x61\x09\0\0\0\0\0\x15\x01\x06\0\x06\0\0\0\
\x18\x01\0\0\x12\0\0\0\0\0\0\0\0\0\0\0\xb7\x02\0\0\x10\0\0\0\x85\0\0\0\x06\0\0\
\0\xb7\0\0\0\x02\0\0\0\x95\0\0\0\0\0\0\0\x71\x61\0\0\0\0\0\0\x67\x01\0\0\x02\0\
\0\0\x57\x01\0\0\x3c\0\0\0\x25\x01\x04\0\x13\0\0\0\x18\x01\0\0\x22\0\0\0\0\0\0\
\0\0\0\0\0\xb7\x02\0\0\x2d\0\0\0\x05\0\xf5\xff\0\0\0\0\x0f\x16\0\0\0\0\0\0\x3d\
\x67\x04\0\0\0\0\0\x18\x01\0\0\x4f\0\0\0\0\0\0\0\0\0\0\0\xb7\x02\0\0\x37\0\0\0\
\x05\0\xef\xff\0\0\0\0\xbf\x61\0\0\0\0\0\0\x07\x01\0\0\x14\0\0\0\x3d\x17\x04\0\
\0\0\0\0\x18\x01\0\0\x86\0\0\0\0\0\0\0\0\0\0\0\xb7\x02\0\0\x25\0\0\0\x05\0\xe8\
\xff\0\0\0\0\x69\x61\x0c\0\0\0\0\0\x57\x01\0\0\0\x02\0\0\x15\x01\x13\0\0\0\0\0\
\x69\x61\x0c\0\0\0\0\0\x57\x01\0\0\0\x10\0\0\x55\x01\x10\0\0\0\0\0\x18\x08\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\x61\x81\0\0\0\0\0\0\x07\x01\0\0\x01\0\0\0\x63\x18\0\0\
\0\0\0\0\x85\0\0\0\x05\0\0\0\x18\x01\0\0\x08\0\0\0\0\0\0\0\0\0\0\0\x79\x12\0\0\
\0\0\0\0\xbf\x03\0\0\0\0\0\0\x1f\x23\0\0\0\0\0\0\xb7\x02\0\0\x01\xca\x9a\x3b\
\x2d\x32\x03\0\0\0\0\0\x7b\x01\0\0\0\0\0\0\xb7\x01\0\0\0\0\0\0\x63\x18\0\0\0\0\
\0\0\xbf\x61\0\0\0\0\0\0\x07\x01\0\0\x20\0\0\0\x3d\x17\x04\0\0\0\0\0\x18\x01\0\
\0\xab\0\0\0\0\0\0\0\0\0\0\0\xb7\x02\0\0\x42\0\0\0\x05\0\xcb\xff\0\0\0\0\xb7\
\x07\0\0\0\0\0\0\x18\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xb7\x02\0\0\x20\0\0\0\xb7\
\x03\0\0\0\0\0\0\x85\0\0\0\x83\0\0\0\x15\0\x12\0\0\0\0\0\xbf\x01\0\0\0\0\0\0\
\x0f\x71\0\0\0\0\0\0\xbf\x62\0\0\0\0\0\0\x0f\x72\0\0\0\0\0\0\x71\x22\0\0\0\0\0\
\0\x73\x21\0\0\0\0\0\0\x07\x07\0\0\x01\0\0\0\x15\x07\x01\0\x20\0\0\0\x05\0\xf7\
\xff\0\0\0\0\xbf\x01\0\0\0\0\0\0\xb7\x02\0\0\0\0\0\0\x85\0\0\0\x84\0\0\0\x18\
\x01\0\0\x0f\x01\0\0\0\0\0\0\0\0\0\0\xb7\x02\0\0\x1f\0\0\0\xb7\x03\0\0\x20\0\0\
\0\x85\0\0\0\x06\0\0\0\x05\0\xb3\xff\0\0\0\0\x18\x01\0\0\xed\0\0\0\0\0\0\0\0\0\
\0\0\xb7\x02\0\0\x22\0\0\0\x05\0\xae\xff\0\0\0\0\x44\x75\x61\x6c\x20\x42\x53\
\x44\x2f\x47\x50\x4c\0\0\0\0\x70\x61\x63\x6b\x65\x74\x20\x73\x69\x7a\x65\x20\
\x69\x73\x20\x25\x64\0\x5b\x45\x52\x52\x4f\x52\x5d\x20\x6e\x6f\x74\x20\x54\x43\
\x50\0\x5b\x45\x52\x52\x4f\x52\x5d\x20\x69\x70\x20\x68\x65\x61\x64\x65\x72\x20\
\x6c\x65\x6e\x20\x69\x73\x20\x6c\x65\x73\x73\x20\x74\x68\x61\x6e\x20\x73\x74\
\x72\x75\x63\x74\x75\x72\x65\0\x5b\x45\x52\x52\x4f\x52\x5d\x20\x69\x70\x20\x2b\
\x20\x68\x65\x61\x64\x65\x72\x20\x6c\x65\x6e\x20\x69\x73\x20\x67\x72\x65\x61\
\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x74\x6f\x74\x61\x6c\x20\x64\x61\x74\x61\
\x20\x6c\x65\x6e\0\x5b\x45\x52\x52\x4f\x52\x5d\x20\x54\x68\x65\x72\x65\x20\x69\
\x73\x20\x6e\x6f\x20\x70\x61\x63\x6b\x65\x74\x20\x61\x66\x74\x65\x72\x20\x54\
\x43\x50\0\x5b\x45\x52\x52\x4f\x52\x5d\x20\x74\x63\x70\x20\x2b\x20\x74\x63\x70\
\x5f\x68\x65\x61\x64\x65\x72\x5f\x62\x79\x74\x65\x73\x20\x6c\x65\x6e\x20\x69\
\x73\x20\x67\x72\x65\x61\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x74\x6f\x74\x61\
\x6c\x20\x64\x61\x74\x61\x20\x6c\x65\x6e\0\x5b\x45\x52\x52\x4f\x52\x5d\x20\x42\
\x75\x66\x66\x65\x72\x20\x72\x65\x73\x65\x72\x76\x61\x74\x69\x6f\x6e\x20\x66\
\x61\x69\x6c\x65\x64\0\x43\x61\x70\x74\x75\x72\x65\x64\x20\x54\x43\x50\x20\x68\
\x65\x61\x64\x65\x72\x20\x28\x25\x64\x20\x62\x79\x74\x65\x73\x29\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\x30\0\0\0\0\0\0\0\x01\0\0\0\x1b\0\0\0\x60\0\0\0\0\0\0\
\0\x01\0\0\0\x1b\0\0\0\x20\0\0\0\0\0\0\0\x01\0\0\0\x1c\0\0\0\x88\0\0\0\0\0\0\0\
\x01\0\0\0\x1c\0\0\0\xd8\0\0\0\0\0\0\0\x01\0\0\0\x1c\0\0\0\x08\x01\0\0\0\0\0\0\
\x01\0\0\0\x1c\0\0\0\x40\x01\0\0\0\0\0\0\x01\0\0\0\x1c\0\0\0\x90\x01\0\0\0\0\0\
\0\x01\0\0\0\x1b\0\0\0\xc0\x01\0\0\0\0\0\0\x01\0\0\0\x1b\0\0\0\x28\x02\0\0\0\0\
\0\0\x01\0\0\0\x1c\0\0\0\x50\x02\0\0\0\0\0\0\x01\0\0\0\x1f\0\0\0\xe0\x02\0\0\0\
\0\0\0\x01\0\0\0\x1c\0\0\0\x10\x03\0\0\0\0\0\0\x01\0\0\0\x1c\0\0\0\x9f\xeb\x01\
\0\x18\0\0\0\0\0\0\0\x3c\x06\0\0\x3c\x06\0\0\x22\x09\0\0\0\0\0\0\0\0\0\x02\x03\
\0\0\0\x01\0\0\0\0\0\0\x01\x04\0\0\0\x20\0\0\x01\0\0\0\0\0\0\0\x03\0\0\0\0\x02\
\0\0\0\x04\0\0\0\x1b\0\0\0\x05\0\0\0\0\0\0\x01\x04\0\0\0\x20\0\0\0\0\0\0\0\0\0\
\0\x02\x06\0\0\0\0\0\0\0\0\0\0\x03\0\0\0\0\x02\0\0\0\x04\0\0\0\0\0\0\x01\0\0\0\
\0\x02\0\0\x04\x10\0\0\0\x19\0\0\0\x01\0\0\0\0\0\0\0\x1e\0\0\0\x05\0\0\0\x40\0\
\0\0\x2a\0\0\0\0\0\0\x0e\x07\0\0\0\x01\0\0\0\0\0\0\0\0\0\0\x02\x0a\0\0\0\x2d\0\
\0\0\x11\0\0\x84\x14\0\0\0\x34\0\0\0\x0b\0\0\0\0\0\0\0\x3b\0\0\0\x0b\0\0\0\x10\
\0\0\0\x40\0\0\0\x0e\0\0\0\x20\0\0\0\x44\0\0\0\x0e\0\0\0\x40\0\0\0\x4c\0\0\0\
\x0c\0\0\0\x60\0\0\x04\x51\0\0\0\x0c\0\0\0\x64\0\0\x04\x56\0\0\0\x0c\0\0\0\x68\
\0\0\x01\x5a\0\0\0\x0c\0\0\0\x69\0\0\x01\x5e\0\0\0\x0c\0\0\0\x6a\0\0\x01\x62\0\
\0\0\x0c\0\0\0\x6b\0\0\x01\x66\0\0\0\x0c\0\0\0\x6c\0\0\x01\x6a\0\0\0\x0c\0\0\0\
\x6d\0\0\x01\x6e\0\0\0\x0c\0\0\0\x6e\0\0\x01\x72\0\0\0\x0c\0\0\0\x6f\0\0\x01\
\x76\0\0\0\x0b\0\0\0\x70\0\0\0\x7d\0\0\0\x11\0\0\0\x80\0\0\0\x83\0\0\0\x0b\0\0\
\0\x90\0\0\0\x8b\0\0\0\0\0\0\x08\x0c\0\0\0\x92\0\0\0\0\0\0\x08\x0d\0\0\0\x98\0\
\0\0\0\0\0\x01\x02\0\0\0\x10\0\0\0\xa7\0\0\0\0\0\0\x08\x0f\0\0\0\xae\0\0\0\0\0\
\0\x08\x10\0\0\0\xb4\0\0\0\0\0\0\x01\x04\0\0\0\x20\0\0\0\xc1\0\0\0\0\0\0\x08\
\x0c\0\0\0\0\0\0\0\x01\0\0\x0d\x13\0\0\0\xc9\0\0\0\x09\0\0\0\xcd\0\0\0\0\0\0\
\x08\x14\0\0\0\xd2\0\0\0\0\0\0\x01\x01\0\0\0\x08\0\0\x04\xd8\0\0\0\x01\0\0\x0c\
\x12\0\0\0\0\0\0\0\0\0\0\x02\x17\0\0\0\xe5\0\0\0\x06\0\0\x04\x18\0\0\0\xec\0\0\
\0\x0f\0\0\0\0\0\0\0\xf1\0\0\0\x0f\0\0\0\x20\0\0\0\xfa\0\0\0\x0f\0\0\0\x40\0\0\
\0\x04\x01\0\0\x0f\0\0\0\x60\0\0\0\x14\x01\0\0\x0f\0\0\0\x80\0\0\0\x23\x01\0\0\
\x0f\0\0\0\xa0\0\0\0\0\0\0\0\x01\0\0\x0d\x02\0\0\0\x32\x01\0\0\x16\0\0\0\x36\
\x01\0\0\x01\0\0\x0c\x18\0\0\0\x41\x01\0\0\x03\0\0\x04\x0e\0\0\0\x48\x01\0\0\
\x1c\0\0\0\0\0\0\0\x4f\x01\0\0\x1c\0\0\0\x30\0\0\0\x58\x01\0\0\x0b\0\0\0\x60\0\
\0\0\x60\x01\0\0\0\0\0\x01\x01\0\0\0\x08\0\0\0\0\0\0\0\0\0\0\x03\0\0\0\0\x1b\0\
\0\0\x04\0\0\0\x06\0\0\0\x6e\x01\0\0\x0a\0\0\x84\x14\0\0\0\x74\x01\0\0\x1e\0\0\
\0\0\0\0\x04\x78\x01\0\0\x1e\0\0\0\x04\0\0\x04\x80\x01\0\0\x1e\0\0\0\x08\0\0\0\
\x84\x01\0\0\x0b\0\0\0\x10\0\0\0\x8c\x01\0\0\x0b\0\0\0\x20\0\0\0\x8f\x01\0\0\
\x0b\0\0\0\x30\0\0\0\x98\x01\0\0\x1e\0\0\0\x40\0\0\0\x9c\x01\0\0\x1e\0\0\0\x48\
\0\0\0\x7d\0\0\0\x11\0\0\0\x50\0\0\0\0\0\0\0\x1f\0\0\0\x60\0\0\0\xa5\x01\0\0\0\
\0\0\x08\x1b\0\0\0\0\0\0\0\x02\0\0\x05\x08\0\0\0\0\0\0\0\x20\0\0\0\0\0\0\0\xaa\
\x01\0\0\x20\0\0\0\0\0\0\0\0\0\0\0\x02\0\0\x04\x08\0\0\0\xb0\x01\0\0\x0e\0\0\0\
\0\0\0\0\xb6\x01\0\0\x0e\0\0\0\x20\0\0\0\xbc\x01\0\0\0\0\0\x01\x01\0\0\0\x08\0\
\0\x01\0\0\0\0\0\0\0\x03\0\0\0\0\x21\0\0\0\x04\0\0\0\x0d\0\0\0\xc1\x01\0\0\0\0\
\0\x0e\x22\0\0\0\x01\0\0\0\xc9\x01\0\0\0\0\0\x0e\x10\0\0\0\0\0\0\0\xd3\x01\0\0\
\0\0\0\x01\x08\0\0\0\x40\0\0\0\xe1\x01\0\0\0\0\0\x0e\x25\0\0\0\0\0\0\0\0\0\0\0\
\0\0\0\x0a\x21\0\0\0\0\0\0\0\0\0\0\x03\0\0\0\0\x27\0\0\0\x04\0\0\0\x12\0\0\0\
\xef\x01\0\0\0\0\0\x0e\x28\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\0\0\0\0\x27\0\0\0\
\x04\0\0\0\x10\0\0\0\x02\x02\0\0\0\0\0\x0e\x2a\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\
\0\0\0\0\x27\0\0\0\x04\0\0\0\x2d\0\0\0\x17\x02\0\0\0\0\0\x0e\x2c\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\x03\0\0\0\0\x27\0\0\0\x04\0\0\0\x37\0\0\0\x2c\x02\0\0\0\0\0\x0e\
\x2e\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\0\0\0\0\x27\0\0\0\x04\0\0\0\x25\0\0\0\x41\
\x02\0\0\0\0\0\x0e\x30\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\0\0\0\0\x27\0\0\0\x04\0\
\0\0\x42\0\0\0\x56\x02\0\0\0\0\0\x0e\x32\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x03\0\0\0\
\0\x27\0\0\0\x04\0\0\0\x22\0\0\0\x6b\x02\0\0\0\0\0\x0e\x34\0\0\0\0\0\0\0\0\0\0\
\0\0\0\0\x03\0\0\0\0\x27\0\0\0\x04\0\0\0\x1f\0\0\0\x80\x02\0\0\0\0\0\x0e\x36\0\
\0\0\0\0\0\0\xfd\x08\0\0\x01\0\0\x0f\x0d\0\0\0\x23\0\0\0\0\0\0\0\x0d\0\0\0\x05\
\x09\0\0\x02\0\0\x0f\x10\0\0\0\x24\0\0\0\0\0\0\0\x04\0\0\0\x26\0\0\0\x08\0\0\0\
\x08\0\0\0\x0a\x09\0\0\x08\0\0\x0f\x2e\x01\0\0\x29\0\0\0\0\0\0\0\x12\0\0\0\x2b\
\0\0\0\x12\0\0\0\x10\0\0\0\x2d\0\0\0\x22\0\0\0\x2d\0\0\0\x2f\0\0\0\x4f\0\0\0\
\x37\0\0\0\x31\0\0\0\x86\0\0\0\x25\0\0\0\x33\0\0\0\xab\0\0\0\x42\0\0\0\x35\0\0\
\0\xed\0\0\0\x22\0\0\0\x37\0\0\0\x0f\x01\0\0\x1f\0\0\0\x12\x09\0\0\x01\0\0\x0f\
\x10\0\0\0\x08\0\0\0\0\0\0\0\x10\0\0\0\0\x69\x6e\x74\0\x5f\x5f\x41\x52\x52\x41\
\x59\x5f\x53\x49\x5a\x45\x5f\x54\x59\x50\x45\x5f\x5f\0\x74\x79\x70\x65\0\x6d\
\x61\x78\x5f\x65\x6e\x74\x72\x69\x65\x73\0\x72\x62\0\x74\x63\x70\x68\x64\x72\0\
\x73\x6f\x75\x72\x63\x65\0\x64\x65\x73\x74\0\x73\x65\x71\0\x61\x63\x6b\x5f\x73\
\x65\x71\0\x72\x65\x73\x31\0\x64\x6f\x66\x66\0\x66\x69\x6e\0\x73\x79\x6e\0\x72\
\x73\x74\0\x70\x73\x68\0\x61\x63\x6b\0\x75\x72\x67\0\x65\x63\x65\0\x63\x77\x72\
\0\x77\x69\x6e\x64\x6f\x77\0\x63\x68\x65\x63\x6b\0\x75\x72\x67\x5f\x70\x74\x72\
\0\x5f\x5f\x62\x65\x31\x36\0\x5f\x5f\x75\x31\x36\0\x75\x6e\x73\x69\x67\x6e\x65\
\x64\x20\x73\x68\x6f\x72\x74\0\x5f\x5f\x62\x65\x33\x32\0\x5f\x5f\x75\x33\x32\0\
\x75\x6e\x73\x69\x67\x6e\x65\x64\x20\x69\x6e\x74\0\x5f\x5f\x73\x75\x6d\x31\x36\
\0\x74\x63\x70\0\x62\x6f\x6f\x6c\0\x5f\x42\x6f\x6f\x6c\0\x69\x73\x5f\x73\x79\
\x6e\x5f\x66\x6c\x6f\x6f\x64\0\x78\x64\x70\x5f\x6d\x64\0\x64\x61\x74\x61\0\x64\
\x61\x74\x61\x5f\x65\x6e\x64\0\x64\x61\x74\x61\x5f\x6d\x65\x74\x61\0\x69\x6e\
\x67\x72\x65\x73\x73\x5f\x69\x66\x69\x6e\x64\x65\x78\0\x72\x78\x5f\x71\x75\x65\
\x75\x65\x5f\x69\x6e\x64\x65\x78\0\x65\x67\x72\x65\x73\x73\x5f\x69\x66\x69\x6e\
\x64\x65\x78\0\x63\x74\x78\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\0\x65\x74\
\x68\x68\x64\x72\0\x68\x5f\x64\x65\x73\x74\0\x68\x5f\x73\x6f\x75\x72\x63\x65\0\
\x68\x5f\x70\x72\x6f\x74\x6f\0\x75\x6e\x73\x69\x67\x6e\x65\x64\x20\x63\x68\x61\
\x72\0\x69\x70\x68\x64\x72\0\x69\x68\x6c\0\x76\x65\x72\x73\x69\x6f\x6e\0\x74\
\x6f\x73\0\x74\x6f\x74\x5f\x6c\x65\x6e\0\x69\x64\0\x66\x72\x61\x67\x5f\x6f\x66\
\x66\0\x74\x74\x6c\0\x70\x72\x6f\x74\x6f\x63\x6f\x6c\0\x5f\x5f\x75\x38\0\x61\
\x64\x64\x72\x73\0\x73\x61\x64\x64\x72\0\x64\x61\x64\x64\x72\0\x63\x68\x61\x72\
\0\x4c\x49\x43\x45\x4e\x53\x45\0\x73\x79\x6e\x5f\x63\x6f\x75\x6e\x74\0\x75\x6e\
\x73\x69\x67\x6e\x65\x64\x20\x6c\x6f\x6e\x67\0\x6c\x61\x73\x74\x5f\x73\x79\x6e\
\x5f\x74\x69\x6d\x65\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\
\x5f\x66\x6d\x74\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\x5f\
\x66\x6d\x74\x2e\x31\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\x5f\x5f\x5f\
\x5f\x66\x6d\x74\x2e\x32\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\x5f\x5f\
\x5f\x5f\x66\x6d\x74\x2e\x33\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\x5f\
\x5f\x5f\x5f\x66\x6d\x74\x2e\x34\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\x2e\
\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x35\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\x70\
\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x36\0\x68\x61\x6e\x64\x6c\x65\x5f\x78\x64\
\x70\x2e\x5f\x5f\x5f\x5f\x66\x6d\x74\x2e\x37\0\x2f\x68\x6f\x6d\x65\x2f\x70\x61\
\x72\x74\x68\x2f\x57\x6f\x72\x6b\x2f\x41\x6c\x6c\x5f\x64\x61\x74\x61\x2f\x75\
\x6e\x69\x76\x65\x72\x73\x69\x74\x79\x2f\x41\x64\x76\x61\x6e\x63\x65\x5f\x4f\
\x53\x2f\x41\x73\x73\x69\x67\x6e\x6d\x65\x6e\x74\x2f\x41\x73\x73\x69\x67\x6e\
\x6d\x65\x6e\x74\x5f\x33\x2f\x71\x32\x2f\x6d\x69\x6e\x69\x6d\x61\x6c\x2e\x62\
\x70\x66\x2e\x63\0\x09\x69\x66\x20\x28\x74\x63\x70\x2d\x3e\x73\x79\x6e\x20\x26\
\x26\x20\x21\x74\x63\x70\x2d\x3e\x61\x63\x6b\x29\x20\x7b\0\x20\x20\x20\x20\x20\
\x20\x20\x20\x73\x79\x6e\x5f\x63\x6f\x75\x6e\x74\x2b\x2b\x3b\0\x09\x09\x75\x6e\
\x73\x69\x67\x6e\x65\x64\x20\x6c\x6f\x6e\x67\x20\x63\x75\x72\x72\x65\x6e\x74\
\x5f\x74\x69\x6d\x65\x20\x3d\x20\x62\x70\x66\x5f\x6b\x74\x69\x6d\x65\x5f\x67\
\x65\x74\x5f\x6e\x73\x28\x29\x3b\0\x09\x09\x69\x66\x28\x63\x75\x72\x72\x65\x6e\
\x74\x5f\x74\x69\x6d\x65\x20\x2d\x20\x6c\x61\x73\x74\x5f\x73\x79\x6e\x5f\x74\
\x69\x6d\x65\x20\x3e\x20\x31\x30\x30\x30\x30\x30\x30\x30\x30\x30\x29\x7b\0\x09\
\x09\x09\x6c\x61\x73\x74\x5f\x73\x79\x6e\x5f\x74\x69\x6d\x65\x20\x3d\x20\x63\
\x75\x72\x72\x65\x6e\x74\x5f\x74\x69\x6d\x65\x3b\0\x09\x09\x09\x73\x79\x6e\x5f\
\x63\x6f\x75\x6e\x74\x20\x3d\x20\x30\x3b\0\x7d\0\x09\x76\x6f\x69\x64\x20\x2a\
\x64\x61\x74\x61\x5f\x65\x6e\x64\x20\x3d\x20\x28\x76\x6f\x69\x64\x20\x2a\x29\
\x28\x6c\x6f\x6e\x67\x29\x63\x74\x78\x2d\x3e\x64\x61\x74\x61\x5f\x65\x6e\x64\
\x3b\0\x09\x76\x6f\x69\x64\x20\x2a\x64\x61\x74\x61\x20\x3d\x20\x28\x76\x6f\x69\
\x64\x20\x2a\x29\x28\x6c\x6f\x6e\x67\x29\x63\x74\x78\x2d\x3e\x64\x61\x74\x61\
\x3b\0\x09\x69\x6e\x74\x20\x70\x6b\x74\x5f\x73\x7a\x20\x3d\x20\x64\x61\x74\x61\
\x5f\x65\x6e\x64\x20\x2d\x20\x64\x61\x74\x61\x3b\0\x09\x62\x70\x66\x5f\x70\x72\
\x69\x6e\x74\x6b\x28\x22\x70\x61\x63\x6b\x65\x74\x20\x73\x69\x7a\x65\x20\x69\
\x73\x20\x25\x64\x22\x2c\x20\x70\x6b\x74\x5f\x73\x7a\x29\x3b\0\x09\x69\x66\x20\
\x28\x28\x76\x6f\x69\x64\x20\x2a\x29\x28\x65\x74\x68\x20\x2b\x20\x31\x29\x20\
\x3e\x20\x64\x61\x74\x61\x5f\x65\x6e\x64\x29\0\x09\x69\x66\x20\x28\x62\x70\x66\
\x5f\x6e\x74\x6f\x68\x73\x28\x65\x74\x68\x2d\x3e\x68\x5f\x70\x72\x6f\x74\x6f\
\x29\x20\x21\x3d\x20\x45\x54\x48\x5f\x50\x5f\x49\x50\x29\0\x09\x69\x66\x20\x28\
\x69\x70\x2d\x3e\x70\x72\x6f\x74\x6f\x63\x6f\x6c\x20\x21\x3d\x20\x49\x50\x50\
\x52\x4f\x54\x4f\x5f\x54\x43\x50\x29\0\x09\x69\x66\x20\x28\x21\x69\x73\x5f\x74\
\x63\x70\x28\x65\x74\x68\x2c\x20\x64\x61\x74\x61\x5f\x65\x6e\x64\x29\x29\0\x09\
\x09\x62\x70\x66\x5f\x70\x72\x69\x6e\x74\x6b\x28\x22\x5b\x45\x52\x52\x4f\x52\
\x5d\x20\x6e\x6f\x74\x20\x54\x43\x50\x22\x29\x3b\0\x09\x69\x6e\x74\x20\x69\x70\
\x5f\x68\x64\x72\x5f\x6c\x65\x6e\x20\x3d\x20\x69\x70\x2d\x3e\x69\x68\x6c\x20\
\x2a\x20\x34\x3b\x20\x2f\x2f\x20\x77\x65\x20\x6e\x65\x65\x64\x20\x74\x6f\x20\
\x6d\x75\x6c\x74\x69\x70\x6c\x79\x20\x61\x73\x20\x69\x74\x20\x69\x73\x20\x33\
\x32\x62\x69\x74\x20\x77\x6f\x72\x64\x2c\x20\x73\x6f\x20\x74\x6f\x74\x61\x6c\
\x20\x73\x69\x7a\x65\x20\x77\x6f\x75\x6c\x64\x20\x62\x65\x20\x28\x33\x32\x77\
\x6f\x72\x64\x73\x29\x20\x2a\x20\x34\0\x09\x69\x66\x28\x69\x70\x5f\x68\x64\x72\
\x5f\x6c\x65\x6e\x20\x3c\x20\x73\x69\x7a\x65\x6f\x66\x28\x73\x74\x72\x75\x63\
\x74\x20\x69\x70\x68\x64\x72\x29\x29\x7b\0\x09\x09\x62\x70\x66\x5f\x70\x72\x69\
\x6e\x74\x6b\x28\x22\x5b\x45\x52\x52\x4f\x52\x5d\x20\x69\x70\x20\x68\x65\x61\
\x64\x65\x72\x20\x6c\x65\x6e\x20\x69\x73\x20\x6c\x65\x73\x73\x20\x74\x68\x61\
\x6e\x20\x73\x74\x72\x75\x63\x74\x75\x72\x65\x22\x29\x3b\0\x09\x69\x66\x28\x28\
\x76\x6f\x69\x64\x20\x2a\x29\x69\x70\x20\x2b\x20\x69\x70\x5f\x68\x64\x72\x5f\
\x6c\x65\x6e\x20\x3e\x20\x64\x61\x74\x61\x5f\x65\x6e\x64\x29\x7b\x20\x2f\x2f\
\x20\x63\x6f\x6d\x70\x61\x72\x69\x6e\x67\x20\x74\x68\x65\x20\x61\x64\x64\x72\
\x65\x73\x73\x3d\0\x09\x09\x62\x70\x66\x5f\x70\x72\x69\x6e\x74\x6b\x28\x22\x5b\
\x45\x52\x52\x4f\x52\x5d\x20\x69\x70\x20\x2b\x20\x68\x65\x61\x64\x65\x72\x20\
\x6c\x65\x6e\x20\x69\x73\x20\x67\x72\x65\x61\x74\x65\x72\x20\x74\x68\x61\x6e\
\x20\x74\x6f\x74\x61\x6c\x20\x64\x61\x74\x61\x20\x6c\x65\x6e\x22\x29\x3b\0\x09\
\x69\x66\x20\x28\x28\x76\x6f\x69\x64\x20\x2a\x29\x20\x28\x74\x63\x70\x20\x2b\
\x20\x31\x29\x20\x3e\x20\x64\x61\x74\x61\x5f\x65\x6e\x64\x29\x7b\0\x09\x09\x62\
\x70\x66\x5f\x70\x72\x69\x6e\x74\x6b\x28\x22\x5b\x45\x52\x52\x4f\x52\x5d\x20\
\x54\x68\x65\x72\x65\x20\x69\x73\x20\x6e\x6f\x20\x70\x61\x63\x6b\x65\x74\x20\
\x61\x66\x74\x65\x72\x20\x54\x43\x50\x22\x29\x3b\0\x09\x69\x66\x20\x28\x28\x76\
\x6f\x69\x64\x20\x2a\x29\x74\x63\x70\x20\x2b\x20\x74\x63\x70\x5f\x68\x65\x61\
\x64\x65\x72\x5f\x62\x79\x74\x65\x73\x20\x3e\x20\x64\x61\x74\x61\x5f\x65\x6e\
\x64\x29\x7b\0\x09\x09\x62\x70\x66\x5f\x70\x72\x69\x6e\x74\x6b\x28\x22\x5b\x45\
\x52\x52\x4f\x52\x5d\x20\x74\x63\x70\x20\x2b\x20\x74\x63\x70\x5f\x68\x65\x61\
\x64\x65\x72\x5f\x62\x79\x74\x65\x73\x20\x6c\x65\x6e\x20\x69\x73\x20\x67\x72\
\x65\x61\x74\x65\x72\x20\x74\x68\x61\x6e\x20\x74\x6f\x74\x61\x6c\x20\x64\x61\
\x74\x61\x20\x6c\x65\x6e\x22\x29\x3b\0\x09\x76\x6f\x69\x64\x20\x2a\x72\x69\x6e\
\x67\x62\x75\x66\x5f\x73\x70\x61\x63\x65\x20\x3d\x20\x62\x70\x66\x5f\x72\x69\
\x6e\x67\x62\x75\x66\x5f\x72\x65\x73\x65\x72\x76\x65\x28\x26\x72\x62\x2c\x20\
\x74\x63\x70\x5f\x68\x65\x61\x64\x65\x72\x5f\x62\x79\x74\x65\x73\x2c\x20\x30\
\x29\x3b\x20\x2f\x2f\x20\x72\x65\x73\x65\x72\x76\x69\x6e\x67\x20\x6f\x6e\x65\
\x20\x62\x79\x74\x65\0\x09\x69\x66\x20\x28\x21\x72\x69\x6e\x67\x62\x75\x66\x5f\
\x73\x70\x61\x63\x65\x29\0\x09\x09\x28\x28\x75\x6e\x73\x69\x67\x6e\x65\x64\x20\
\x63\x68\x61\x72\x20\x2a\x29\x72\x69\x6e\x67\x62\x75\x66\x5f\x73\x70\x61\x63\
\x65\x29\x5b\x69\x5d\x20\x3d\x20\x62\x79\x74\x65\x3b\0\x09\x09\x75\x6e\x73\x69\
\x67\x6e\x65\x64\x20\x63\x68\x61\x72\x20\x62\x79\x74\x65\x20\x3d\x20\x2a\x28\
\x20\x28\x28\x75\x6e\x73\x69\x67\x6e\x65\x64\x20\x63\x68\x61\x72\x20\x2a\x29\
\x74\x63\x70\x29\x20\x2b\x20\x69\x20\x29\x3b\0\x09\x66\x6f\x72\x20\x28\x69\x6e\
\x74\x20\x69\x20\x3d\x20\x30\x3b\x20\x69\x20\x3c\x20\x74\x63\x70\x5f\x68\x65\
\x61\x64\x65\x72\x5f\x62\x79\x74\x65\x73\x3b\x20\x69\x2b\x2b\x29\x7b\0\x09\x62\
\x70\x66\x5f\x72\x69\x6e\x67\x62\x75\x66\x5f\x73\x75\x62\x6d\x69\x74\x28\x72\
\x69\x6e\x67\x62\x75\x66\x5f\x73\x70\x61\x63\x65\x2c\x20\x30\x29\x3b\0\x20\x20\
\x20\x20\x62\x70\x66\x5f\x70\x72\x69\x6e\x74\x6b\x28\x22\x43\x61\x70\x74\x75\
\x72\x65\x64\x20\x54\x43\x50\x20\x68\x65\x61\x64\x65\x72\x20\x28\x25\x64\x20\
\x62\x79\x74\x65\x73\x29\x22\x2c\x20\x74\x63\x70\x5f\x68\x65\x61\x64\x65\x72\
\x5f\x62\x79\x74\x65\x73\x29\x3b\0\x09\x09\x62\x70\x66\x5f\x70\x72\x69\x6e\x74\
\x6b\x28\x22\x5b\x45\x52\x52\x4f\x52\x5d\x20\x42\x75\x66\x66\x65\x72\x20\x72\
\x65\x73\x65\x72\x76\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64\x22\x29\
\x3b\0\x30\x3a\x37\0\x30\x3a\x31\x30\0\x30\x3a\x31\0\x30\x3a\x30\0\x30\x3a\x32\
\0\x6c\x69\x63\x65\x6e\x73\x65\0\x2e\x62\x73\x73\0\x2e\x72\x6f\x64\x61\x74\x61\
\0\x2e\x6d\x61\x70\x73\0\x2e\x74\x65\x78\x74\0\x78\x64\x70\0\0\0\x9f\xeb\x01\0\
\x20\0\0\0\0\0\0\0\x24\0\0\0\x24\0\0\0\x14\x04\0\0\x38\x04\0\0\xa4\0\0\0\x08\0\
\0\0\x18\x09\0\0\x01\0\0\0\0\0\0\0\x15\0\0\0\x1e\x09\0\0\x01\0\0\0\0\0\0\0\x19\
\0\0\0\x10\0\0\0\x18\x09\0\0\x0e\0\0\0\0\0\0\0\x95\x02\0\0\xee\x02\0\0\x0b\xc0\
\x02\0\x08\0\0\0\x95\x02\0\0\xee\x02\0\0\x06\xc0\x02\0\x10\0\0\0\x95\x02\0\0\
\xee\x02\0\0\x0f\xc0\x02\0\x18\0\0\0\x95\x02\0\0\xee\x02\0\0\x18\xc0\x02\0\x20\
\0\0\0\x95\x02\0\0\xee\x02\0\0\x13\xc0\x02\0\x28\0\0\0\x95\x02\0\0\xee\x02\0\0\
\x06\xc0\x02\0\x30\0\0\0\x95\x02\0\0\x0c\x03\0\0\x12\xc8\x02\0\x58\0\0\0\x95\
\x02\0\0\x21\x03\0\0\x20\xcc\x02\0\x60\0\0\0\x95\x02\0\0\x54\x03\0\0\x15\xd8\
\x02\0\x78\0\0\0\x95\x02\0\0\x54\x03\0\0\x13\xd8\x02\0\x90\0\0\0\x95\x02\0\0\
\x54\x03\0\0\x06\xd8\x02\0\x98\0\0\0\x95\x02\0\0\x85\x03\0\0\x12\xe0\x02\0\xa8\
\0\0\0\x95\x02\0\0\xa6\x03\0\0\x0e\xdc\x02\0\xb0\0\0\0\x95\x02\0\0\xb8\x03\0\0\
\x01\0\x03\0\x1e\x09\0\0\x32\0\0\0\0\0\0\0\x95\x02\0\0\xba\x03\0\0\x26\x48\x03\
\0\x08\0\0\0\x95\x02\0\0\xe9\x03\0\0\x22\x44\x03\0\x10\0\0\0\x95\x02\0\0\x10\
\x04\0\0\x18\x4c\x03\0\x20\0\0\0\x95\x02\0\0\x2f\x04\0\0\x02\x54\x03\0\x40\0\0\
\0\x95\x02\0\0\x59\x04\0\0\x13\x90\x01\0\x50\0\0\0\x95\x02\0\0\x59\x04\0\0\x06\
\x90\x01\0\x58\0\0\0\x95\x02\0\0\x7c\x04\0\0\x06\xa4\x01\0\x60\0\0\0\x95\x02\0\
\0\x7c\x04\0\0\x06\xa4\x01\0\x78\0\0\0\x95\x02\0\0\xa6\x04\0\0\x0a\x34\x02\0\
\x80\0\0\0\x95\x02\0\0\xc8\x04\0\0\x06\x84\x03\0\x88\0\0\0\x95\x02\0\0\xe5\x04\
\0\0\x03\x8c\x03\0\xa0\0\0\0\x95\x02\0\0\0\0\0\0\0\0\0\0\xa8\0\0\0\x95\x02\0\0\
\xb8\x03\0\0\x01\x34\x05\0\xb8\0\0\0\x95\x02\0\0\x06\x05\0\0\x17\xac\x03\0\xc0\
\0\0\0\x95\x02\0\0\x06\x05\0\0\x1b\xac\x03\0\xd0\0\0\0\x95\x02\0\0\x76\x05\0\0\
\x05\xb4\x03\0\xd8\0\0\0\x95\x02\0\0\x9e\x05\0\0\x03\xb8\x03\0\xf8\0\0\0\x95\
\x02\0\0\xdc\x05\0\0\x10\xc8\x03\0\0\x01\0\0\x95\x02\0\0\xdc\x05\0\0\x05\xc8\
\x03\0\x08\x01\0\0\x95\x02\0\0\x1f\x06\0\0\x03\xcc\x03\0\x28\x01\0\0\x95\x02\0\
\0\x67\x06\0\0\x14\x60\x04\0\x38\x01\0\0\x95\x02\0\0\x67\x06\0\0\x06\x60\x04\0\
\x40\x01\0\0\x95\x02\0\0\x8c\x06\0\0\x03\x64\x04\0\x60\x01\0\0\x95\x02\0\0\xee\
\x02\0\0\x0b\xc0\x02\0\x68\x01\0\0\x95\x02\0\0\xee\x02\0\0\x06\xc0\x02\0\x70\
\x01\0\0\x95\x02\0\0\xee\x02\0\0\x0f\xc0\x02\0\x78\x01\0\0\x95\x02\0\0\xee\x02\
\0\0\x18\xc0\x02\0\x80\x01\0\0\x95\x02\0\0\xee\x02\0\0\x13\xc0\x02\0\x88\x01\0\
\0\x95\x02\0\0\xee\x02\0\0\x06\xc0\x02\0\x90\x01\0\0\x95\x02\0\0\x0c\x03\0\0\
\x12\xc8\x02\0\xb8\x01\0\0\x95\x02\0\0\x21\x03\0\0\x20\xcc\x02\0\xc0\x01\0\0\
\x95\x02\0\0\x54\x03\0\0\x15\xd8\x02\0\xd8\x01\0\0\x95\x02\0\0\x54\x03\0\0\x13\
\xd8\x02\0\xf0\x01\0\0\x95\x02\0\0\x54\x03\0\0\x06\xd8\x02\0\xf8\x01\0\0\x95\
\x02\0\0\x85\x03\0\0\x12\xe0\x02\0\x08\x02\0\0\x95\x02\0\0\xa6\x03\0\0\x0e\xdc\
\x02\0\x10\x02\0\0\x95\x02\0\0\xc2\x06\0\0\x12\xc0\x04\0\x20\x02\0\0\x95\x02\0\
\0\xc2\x06\0\0\x06\xc0\x04\0\x28\x02\0\0\x95\x02\0\0\xf3\x06\0\0\x03\xc4\x04\0\
\x50\x02\0\0\x95\x02\0\0\x46\x07\0\0\x18\xdc\x04\0\x78\x02\0\0\x95\x02\0\0\xa2\
\x07\0\0\x06\xe0\x04\0\x80\x02\0\0\x95\x02\0\0\xb7\x07\0\0\x03\x0c\x05\0\x90\
\x02\0\0\x95\x02\0\0\xe5\x07\0\0\x32\x08\x05\0\xa0\x02\0\0\x95\x02\0\0\xe5\x07\
\0\0\x18\x08\x05\0\xa8\x02\0\0\x95\x02\0\0\xb7\x07\0\0\x27\x0c\x05\0\xb0\x02\0\
\0\x95\x02\0\0\x1d\x08\0\0\x29\x04\x05\0\xb8\x02\0\0\x95\x02\0\0\x1d\x08\0\0\
\x02\x04\x05\0\xc8\x02\0\0\x95\x02\0\0\x4a\x08\0\0\x02\x1c\x05\0\xe0\x02\0\0\
\x95\x02\0\0\x71\x08\0\0\x05\x28\x05\0\x10\x03\0\0\x95\x02\0\0\xb5\x08\0\0\x03\
\xe8\x04\0\x10\0\0\0\x18\x09\0\0\x02\0\0\0\0\0\0\0\x0a\0\0\0\xe8\x08\0\0\0\0\0\
\0\x18\0\0\0\x0a\0\0\0\xec\x08\0\0\0\0\0\0\x1e\x09\0\0\x07\0\0\0\0\0\0\0\x17\0\
\0\0\xf1\x08\0\0\0\0\0\0\x08\0\0\0\x17\0\0\0\xf5\x08\0\0\0\0\0\0\x58\0\0\0\x1a\
\0\0\0\xf9\x08\0\0\0\0\0\0\x78\0\0\0\x1d\0\0\0\xe8\x08\0\0\0\0\0\0\xb8\0\0\0\
\x1d\0\0\0\xf5\x08\0\0\0\0\0\0\x60\x01\0\0\x0a\0\0\0\xe8\x08\0\0\0\0\0\0\x78\
\x01\0\0\x0a\0\0\0\xec\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\x01\0\0\0\x03\0\0\0\x20\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x40\0\
\0\0\0\0\0\0\xa8\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\x09\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xe8\x01\0\0\0\0\0\0\
\x18\x03\0\0\0\0\0\0\x01\0\0\0\0\0\0\0\x08\0\0\0\0\0\0\0\x18\0\0\0\0\0\0\0\x11\
\0\0\0\x01\0\0\0\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05\0\0\0\0\0\0\xc0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x17\0\0\0\x01\0\0\0\
\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\xc0\x05\0\0\0\0\0\0\x30\x03\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x1b\0\0\0\x01\0\0\0\x03\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\xf0\x08\0\0\0\0\0\0\x0d\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x23\0\0\0\x08\0\0\0\x03\0\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\x09\0\0\0\0\0\0\x10\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x08\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\x28\0\0\0\x01\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x09\
\0\0\0\0\0\0\x2e\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\x30\0\0\0\x01\0\0\0\x03\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x30\x0a\0\0\0\0\0\0\
\x10\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x88\x01\0\
\0\x09\0\0\0\x40\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x40\x0a\0\0\0\0\0\0\x20\0\0\0\0\
\0\0\0\x02\0\0\0\x03\0\0\0\x08\0\0\0\0\0\0\0\x10\0\0\0\0\0\0\0\x92\x01\0\0\x09\
\0\0\0\x40\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x60\x0a\0\0\0\0\0\0\xb0\0\0\0\0\0\0\0\
\x02\0\0\0\x04\0\0\0\x08\0\0\0\0\0\0\0\x10\0\0\0\0\0\0\0\x9a\x01\0\0\x01\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x10\x0b\0\0\0\0\0\0\x76\x0f\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x9f\x01\0\0\x01\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\x88\x1a\0\0\0\0\0\0\xfc\x04\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\x08\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

	*sz = sizeof(data) - 1;
	return (const void *)data;
}

#ifdef __cplusplus
struct minimal_bpf *minimal_bpf::open(const struct bpf_object_open_opts *opts) { return minimal_bpf__open_opts(opts); }
struct minimal_bpf *minimal_bpf::open_and_load() { return minimal_bpf__open_and_load(); }
int minimal_bpf::load(struct minimal_bpf *skel) { return minimal_bpf__load(skel); }
int minimal_bpf::attach(struct minimal_bpf *skel) { return minimal_bpf__attach(skel); }
void minimal_bpf::detach(struct minimal_bpf *skel) { minimal_bpf__detach(skel); }
void minimal_bpf::destroy(struct minimal_bpf *skel) { minimal_bpf__destroy(skel); }
const void *minimal_bpf::elf_bytes(size_t *sz) { return minimal_bpf__elf_bytes(sz); }
#endif /* __cplusplus */

__attribute__((unused)) static void
minimal_bpf__assert(struct minimal_bpf *s __attribute__((unused)))
{
#ifdef __cplusplus
#define _Static_assert static_assert
#endif
#ifdef __cplusplus
#undef _Static_assert
#endif
}

#endif /* __MINIMAL_BPF_SKEL_H__ */
