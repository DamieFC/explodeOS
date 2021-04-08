/*
 * x86_64-efi/fs.h
 *
 * Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Filesystem drivers for initial ramdisk.
 *
 */

/**
 * FS/Z initrd (OS/Z's native file system)
 */
file_t fsz_initrd(unsigned char *initrd_p, char *kernel)
{
    file_t ret = { NULL, 0 };
    if(initrd_p==NULL || CompareMem(initrd_p + 512,"FS/Z",4) || kernel==NULL){
        return ret;
    }
    unsigned char passphrase[256],chk[32],iv[32];
    unsigned int i,j,k,l,ss=1<<(initrd_p[519]+11);
    unsigned char *ent, *in=(initrd_p+*((uint64_t*)(initrd_p+560))*ss);
    SHA256_CTX ctx;
    DBG(L" * FS/Z %s\n",a2u(kernel));
    //decrypt initrd
    while(*((uint32_t*)(initrd_p+708))!=0) {
        Print(L" * Passphrase? ");
        l=ReadLine(passphrase,sizeof(passphrase));
        if(!l) {
            Print(L"\n");
            return ret;
        }
        if(*((uint32_t*)(initrd_p+708))!=crc32_calc((char*)passphrase,l)) {
            Print(L"\rBOOTBOOT-ERROR: Bad passphrase\n");
            continue;
        }
        Print(L"\r * Decrypting...\r");
        SHA256_Init(&ctx);
        SHA256_Update(&ctx,passphrase,l);
        SHA256_Update(&ctx,initrd_p+512,6);
        SHA256_Final(chk,&ctx);
        for(i=0;i<28;i++) initrd_p[i+680]^=chk[i];
        SHA256_Init(&ctx);
        SHA256_Update(&ctx,initrd_p+680,28);
        SHA256_Final(iv,&ctx);
        if(((initrd_p[518]>>2)&7)==1) {
            aes_init(iv);
            for(k=ss,j=1;j<*((uint32_t*)(initrd_p+528));k+=ss,j++) {
                aes_dec(initrd_p+k,ss);
            }
        } else {
            for(k=ss,j=1;j<*((uint32_t*)(initrd_p+528));j++) {
                CopyMem(chk,iv,32);
                for(i=0;i<ss;i++) {
                    if(i%32==0) {
                        SHA256_Init(&ctx);
                        SHA256_Update(&ctx,&chk,32);
                        SHA256_Update(&ctx,&j,4);
                        SHA256_Final(chk,&ctx);
                    }
                    initrd_p[k++]^=chk[i%32]^iv[i%32];
                }
            }
        }
        ZeroMem(initrd_p+680,28+4);
        *((uint32_t*)(initrd_p+1020))=crc32_calc((char *)initrd_p+512,508);
        Print(L"                \r");
    }
    // Get the inode
    char *s,*e;
    s=e=kernel;
    i=0;
again:
    while(*e!='/'&&*e!=0){e++;}
    if(*e=='/'){e++;}
    if(!CompareMem(in,"FSIN",4)){
        //is it inlined?
        if(!CompareMem(initrd_p[520]&1? in + 2048 : in + 1024,"FSDR",4)){
            ent=(initrd_p[520]&1? in + 2048 : in + 1024);
        } else if(!CompareMem(initrd_p+*((uint64_t*)(in+448))*ss,"FSDR",4)){
            // go, get the sector pointed
            ent=(initrd_p+*((uint64_t*)(in+448))*ss);
        } else {
            return ret;
        }
        //skip header
        unsigned char *hdr=ent; ent+=128;
        //iterate on directory entries
        int j=*((uint64_t*)(hdr+16));
        while(j-->0){
            if(!CompareMem(ent + 16,s,e-s)) {
                if(*e==0) {
                    i=*((uint64_t*)(ent+0));
                    break;
                } else {
                    s=e;
                    in=(initrd_p+*((uint64_t*)(ent+0))*ss);
                    goto again;
                }
            }
            ent+=128;
        }
    } else {
        i=0;
    }
    if(i!=0) {
        // fid -> inode ptr -> data ptr
        unsigned char *in=(initrd_p+i*ss);
        if(!CompareMem(in,"FSIN",4)){
            ret.size=*((uint64_t*)(in+464));
            switch(in[488]) {
                case 0xFF:
                    // inline data
                    ret.ptr=(uint8_t*)(initrd_p+i*ss+(initrd_p[520]&1? 2048 : 1024));
                    break;
                case 0x80:
                case 0x7F:
                    // sector directory or list inlined
                    ret.ptr=(uint8_t*)(initrd_p + *((uint64_t*)(initrd_p[520]&1? in + 2048 : in + 1024))*ss);
                    break;
                case 0:
                    // direct data
                    ret.ptr=(uint8_t*)(initrd_p + *((uint64_t*)(in+448)) * ss);
                    break;
                // sector directory (only one level supported here, and no holes in files)
                case 0x81:
                case 1:
                    ret.ptr=(uint8_t*)(initrd_p + *((uint64_t*)(initrd_p + *((uint64_t*)(in+448))*ss)) * ss);
                    break;
                default:
                    ret.size=0;
                    break;
            }
        }
    }
    return ret;
}

/**
 * cpio archive
 */
file_t cpio_initrd(unsigned char *initrd_p, char *kernel)
{
    unsigned char *ptr=initrd_p;
    int k;
    file_t ret = { NULL, 0 };
    if(initrd_p==NULL || kernel==NULL ||
        (CompareMem(initrd_p,"070701",6) && CompareMem(initrd_p,"070702",6) && CompareMem(initrd_p,"070707",6)))
        return ret;
    DBG(L" * cpio %s\n",a2u(kernel));
    k=strlena((unsigned char*)kernel);
    // hpodc archive
    while(!CompareMem(ptr,"070707",6)){
        int ns=oct2bin(ptr+8*6+11,6);
        int fs=oct2bin(ptr+8*6+11+6,11);
        if(!CompareMem(ptr+9*6+2*11,kernel,k+1) ||
            (ptr[9*6+2*11] == '.' && ptr[9*6+2*11+1] == '/' && !CompareMem(ptr+9*6+2*11+2,kernel,k+1))) {
            ret.size=fs;
            ret.ptr=(UINT8*)(ptr+9*6+2*11+ns);
            return ret;
        }
        ptr+=(76+ns+fs);
    }
    // newc and crc archive
    while(!CompareMem(ptr,"07070",5)){
        int fs=hex2bin(ptr+8*6+6,8);
        int ns=hex2bin(ptr+8*11+6,8);
        if(!CompareMem(ptr+110,kernel,k+1) || (ptr[110] == '.' && ptr[111] == '/' && !CompareMem(ptr+112,kernel,k+1))) {
            ret.size=fs;
            ret.ptr=(UINT8*)(ptr+((110+ns+3)/4)*4);
            return ret;
        }
        ptr+=((110+ns+3)/4)*4 + ((fs+3)/4)*4;
    }
    return ret;
}

/**
 * ustar tarball archive
 */
file_t tar_initrd(unsigned char *initrd_p, char *kernel)
{
    unsigned char *ptr=initrd_p;
    int k;
    file_t ret = { NULL, 0 };
    if(initrd_p==NULL || kernel==NULL || CompareMem(initrd_p+257,"ustar",5))
        return ret;
    DBG(L" * tar %s\n",a2u(kernel));
    k=strlena((unsigned char*)kernel);
    while(!CompareMem(ptr+257,"ustar",5)){
        int fs=oct2bin(ptr+0x7c,11);
        if(!CompareMem(ptr,kernel,k+1) || (ptr[0] == '.' && ptr[1] == '/' && !CompareMem(ptr+2,kernel,k+1))) {
            ret.size=fs;
            ret.ptr=(UINT8*)(ptr+512);
            return ret;
        }
        ptr+=(((fs+511)/512)+1)*512;
    }
    return ret;
}

/**
 * Simple File System
 */
file_t sfs_initrd(unsigned char *initrd_p, char *kernel)
{
    unsigned char *ptr, *end;
    int k,bs,ver;
    file_t ret = { NULL, 0 };
    if(initrd_p==NULL || kernel==NULL || (CompareMem(initrd_p+0x1AC,"SFS",3) && CompareMem(initrd_p+0x1A6,"SFS",3)))
        return ret;
    // 1.0 Brendan's version, 1.10 BenLunt's version
    ver=!CompareMem(initrd_p+0x1A6,"SFS",3)?10:0;
    bs=1<<(7+(UINT8)initrd_p[ver?0x1B6:0x1BC]);
    end=initrd_p + *((UINT64 *)&initrd_p[ver?0x1AA:0x1B0]) * bs; // base + total_number_of_blocks * blocksize
    // get index area
    ptr=end - *((UINT64 *)&initrd_p[ver?0x19E:0x1A4]); // end - size of index area
    // got a Starting Marker Entry?
    if(ptr[0]!=2)
        return ret;
    DBG(L" * SFS 1.%d %s\n",ver,a2u(kernel));
    k=strlena((unsigned char*)kernel);
    // iterate on index until we reach the end or Volume Identifier
    while(ptr<end && ptr[0]!=0x01){
        ptr+=64;
        // file entry?
        if(ptr[0]!=0x12)
            continue;
        // filename match?
        if(!CompareMem(ptr+(ver?0x23:0x22),kernel,k+1)){
            ret.size=*((UINTN*)&ptr[ver?0x1B:0x1A]);                 // file_length
            ret.ptr=initrd_p + *((UINT64*)&ptr[ver?0x0B:0x0A]) * bs; // base + start_block * blocksize
            break;
        }
    }
    return ret;
}

/**
 * James Molloy's initrd (for some reason it's popular among hobby OS developers)
 * http://www.jamesmolloy.co.uk/tutorial_html
 */
file_t jamesm_initrd(unsigned char *initrd_p, char *kernel)
{
    unsigned char *ptr=initrd_p+4;
    int i,k,nf=*((int*)initrd_p);
    file_t ret = { NULL, 0 };
    // no real magic, so we assume initrd contains at least one file...
    if(initrd_p==NULL || kernel==NULL || initrd_p[2]!=0 || initrd_p[3]!=0 || initrd_p[4]!=0xBF)
        return ret;
    DBG(L" * JamesM %s\n",a2u(kernel));
    k=strlena((unsigned char*)kernel);
    for(i=0;i<nf && ptr[0]==0xBF;i++) {
        if(!CompareMem(ptr+1,kernel,k+1)){
            ret.ptr=*((uint32_t*)(ptr+65)) + initrd_p;
            ret.size=*((uint32_t*)(ptr+69));
        }
        ptr+=73;
    }
    return ret;
}

/**
 * EchFS
 * http://github.com/echfs/echfs
 */
file_t ech_initrd(unsigned char *initrd_p, char *kernel)
{
    UINT64 parent = 0xffffffffffffffffUL, n;
    unsigned char *ptr;
    char *end, *fn;
    int k = 0;
    file_t ret = { NULL, 0 };
    if(initrd_p==NULL || kernel==NULL || CompareMem(initrd_p+4,"_ECH_FS_",8))
        return ret;
    DBG(L" * EchFS %s\n",a2u(kernel));
    CopyMem(&k, initrd_p + 28, 4);
    CopyMem(&n, initrd_p + 12, 8);
    ptr = initrd_p + (((n * 8 + k - 1) / k) + 16) * k;
    for(end = fn = kernel; *end && *end != '/'; end++);
    while(*((UINT64*)ptr)) {
        if(*((UINT64*)ptr) == parent && !CompareMem(ptr + 9, fn, end - fn) && !ptr[9 + end - fn]) {
            parent = *((UINT64*)(ptr + 240));
            if(!*end) {
                ret.size=*((UINTN*)(ptr + 248));
                ret.ptr=(UINT8*)(initrd_p + parent * k);
                break;
            }
            end++;
            for(fn = end; *end && *end != '/'; end++);
        }
        ptr += 256;
    }
    return ret;
}

/**
 * Static file system drivers registry
 */
file_t (*fsdrivers[]) (unsigned char *, char *) = {
    fsz_initrd,
    cpio_initrd,
    tar_initrd,
    sfs_initrd,
    jamesm_initrd,
    ech_initrd,
    NULL
};
