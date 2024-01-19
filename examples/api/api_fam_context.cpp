/*
 * api_fam_context.cpp
 * Copyright (c) 2022 Hewlett Packard Enterprise Development, LP. All rights
 * reserved. Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * See https://spdx.org/licenses/BSD-3-Clause
 *
 */

#include <fam/fam.h>
#include <fam/fam_exception.h>
#include <iostream>
#include <string.h>
using namespace std;
using namespace openfam;

int main(void) {
    int ret = 0;
    Fam_Descriptor *descriptor = NULL;

    fam *myFam = new fam();
    Fam_Options *fm = (Fam_Options *)malloc(sizeof(Fam_Options));
    memset((void *)fm, 0, sizeof(Fam_Options));
    // assume that no specific options are needed by the implementation
    fm->runtime = strdup("NONE");
    try {
        myFam->fam_initialize("myApplication", fm);
        printf("FAM initialized\n");
    } catch (Fam_Exception &e) {
        printf("FAM Initialization failed: %s\n", e.fam_error_msg());
        myFam->fam_abort(-1); // abort the program
        // note that fam_abort currently returns after signaling
        // so we must terminate with the same value
        return -1;
    }
    fam_context *ctx = myFam->fam_context_open();

    try {
        // create a 100 MB region with 0777 permissions
        Fam_Region_Descriptor *rd = myFam->fam_create_region(
            "myRegion", (uint64_t)10000000, 0777, NULL);
        // use the created region...
        if (rd != NULL)
            printf("fam_create_region successful\n");
        descriptor = myFam->fam_allocate("myItem", (uint64_t)(50 * sizeof(int)),
                                         0600, rd);
        std::cout << "Calling IO operations" << std::endl;
        ctx->fam_or(descriptor, 0, (uint32_t)1);
        ctx->fam_or(descriptor, 0, (uint32_t)2);
        ctx->fam_or(descriptor, 0, (uint32_t)3);
        ctx->fam_or(descriptor, 0, (uint32_t)4);
        myFam->fam_or(descriptor, 0, (uint32_t)4);
        myFam->fam_or(descriptor, 0, (uint32_t)4);
        ctx->fam_quiet();
        myFam->fam_quiet();
        std::cout << "Quiet completed" << std::endl;
        // ... continuation code here
        // we are finished. Destroy the region and everything in it
        myFam->fam_destroy_region(rd);
        printf("fam_destroy_region successful\n");

    } catch (Fam_Exception &e) {
        printf("Create/Destroy region failed: %d: %s\n", e.fam_error(),
               e.fam_error_msg());
        Fam_Region_Descriptor *rd = myFam->fam_lookup_region("myRegion");
        myFam->fam_destroy_region(rd);
        ret = -1;
    }

    myFam->fam_context_close(ctx);
    try {
        myFam->fam_finalize("myApplication");
        printf("FAM finalized\n");
    } catch (Fam_Exception &e) {
        printf("FAM Finalization failed: %s\n", e.fam_error_msg());
        myFam->fam_abort(-1); // abort the program
        // note that fam_abort currently returns after signaling
        // so we must terminate with the same value
        return -1;
    }
    return (ret);
}
