/*
works on a node with the following
	lustre-1.8.7-wc1_2.6.18_274.12.1.el5_gbca4755
	lustre-debuginfo-1.8.7-wc1_2.6.18_274.12.1.el5_gbca4755
	lustre-modules-1.8.7-wc1_2.6.18_274.12.1.el5_gbca4755
	lustre-source-1.8.7-wc1_2.6.18_274.12.1.el5_gbca4755
	lustre-tests-1.8.7-wc1_2.6.18_274.12.1.el5_gbca4755

command line (as a reference)
	$ lfs getstripe /MY_LUSTRE_FILESYSTEM/foo
		/MY_LUSTRE_FILESYSTEM/foo
		lmm_stripe_count:   1
		lmm_stripe_size:    1048576
		lmm_stripe_offset:  86
				obdidx           objid          objid            group
					86         1091894       0x10a936                0
this api
	$ ./lfsgetstripe /MY_LUSTRE_FILESYSTEM/foo
		Lov magic: 198249424
		Lov pattern: 1
		Lov object id: 8631841
		Lov object group: 0
		Lov stripe size: 1048576
		Lov stripe count: 1
		Lov stripe offset: 0
		Object index: 86, Objid: 1091894

John Brunelle
Harvard FAS Research Computing
*/


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <lustre/liblustreapi.h>
#include <lustre/lustre_user.h>

//(MAX_OSTS here is really maximum stripe count (the lov_user_md_v1 object needs room for the data for each stripe); this should really use LOV_MAX_STRIPE_COUNT from lustre_idl.h, which is 160 as of 1.8.7)
#define MAX_OSTS 1024
#define LOV_EA_SIZE(lum, num) (sizeof(*lum) + num * sizeof(*lum->lmm_objects))
#define LOV_EA_MAX(lum) LOV_EA_SIZE(lum, MAX_OSTS)

int get_file_info(char *path) {
 
	struct lov_user_md_v1 *lum;
	int rc, i;
	 
	lum = malloc(LOV_EA_MAX(lum));
	if (lum==NULL) { fprintf(stderr, "*** ERROR *** malloc failed\n"); exit(1); }
 	
	rc = llapi_file_get_stripe(path, lum);
	if (rc != 0) {
		//e.g. 36, ENAMETOOLONG
		fprintf(stderr, "*** ERROR *** llapi_file_get_stripe of [%s] failed with return value [%d]\n", path, rc);  //docs say it sets errno, I find that's not the case
		return rc;
	}
 
	printf("Lov magic: %u\n"         , lum->lmm_magic);
	printf("Lov pattern: %u\n"       , lum->lmm_pattern);
	printf("Lov object id: %llu\n"   , lum->lmm_object_id);
	printf("Lov object group: %llu\n", lum->lmm_object_gr);
	printf("Lov stripe size: %u\n"   , lum->lmm_stripe_size);
	printf("Lov stripe count: %hu\n" , lum->lmm_stripe_count);
	printf("Lov stripe offset: %u\n" , lum->lmm_stripe_offset);
	for (i = 0; i < lum->lmm_stripe_count; i++) {
		printf("Object index: %d, Objid: %llu\n", lum->lmm_objects[i].l_ost_idx, lum->lmm_objects[i].l_object_id);
	}
 
	free(lum);
	return 0;
}

int main(int argc, char **argv) {
	int rc;
	rc = get_file_info(argv[1]);
	exit(rc);
}
