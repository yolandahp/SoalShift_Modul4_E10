#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/titut/Downloads";


static int xmp_getattr(const char *path, struct stat *stbuf){
  int res;
  char fpath[1000];
  
  sprintf(fpath,"%s%s",dirpath,path);

  res = lstat(fpath, stbuf);

  if(res == -1) return -errno;
  return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev){
  int res;
  char fpath[2000];
  sprintf(fpath, "%s%s", dirpath, path);

  res = mknod(fpath, mode, rdev);
  if(res == -1)
    return -errno;

  return 0;
}

//untuk mengganti permission file
static int xmp_chmod(const char *path, mode_t mode){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s", dirpath, path);
    res = chmod(fpath, mode);
    if(res == -1)
      return -errno;

    return 0;
}

//buat mindahin data yang abis diedit itu dipindah ke simpanan, data asli tetep
static int xmp_rename(const char *from, const char *to){
  int res;
  char nfrom[1000], nto[1000], newdir[1000], arg[1000], arg2[1000];
  
  sprintf(newdir, "/home/titut/Downloads/simpanan");
  
  sprintf(arg, "mkdir -p %s", newdir);
  system(arg);
  
  sprintf(nfrom,"%s%s", dirpath, from);
  sprintf(nto,"%s%s", newdir, to);
  
  res = rename(nfrom, nto);

  if(res == -1)
    return -errno;

  return 0;
}

//membaca direktori
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
  DIR *dp;
  struct dirent *de;
  char fpath[1000];

  if(strcmp(path,"/") == 0){
    path=dirpath;
    sprintf(fpath,"%s",path);
  }
  else sprintf(fpath, "%s%s",dirpath,path);
  int res = 0;

  (void) offset;
  (void) fi;

  dp = opendir(fpath);
  if (dp == NULL) return -errno;

  while ((de = readdir(dp)) != NULL) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;
    res = (filler(buf, de->d_name, &st, 0));
    if(res!=0) break;
  }

  closedir(dp);
  return 0;
}

//untk menerima dr pengguna, nanti disimpan di memory komp
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
  char fpath[1000];
  int res = 0;
  int fd = 0;
  if(strcmp(path,"/") == 0)
  {
    path=dirpath;
    sprintf(fpath,"%s",path);
  }
  else sprintf(fpath, "%s%s",dirpath,path);

  (void) fi;

  fd = open(fpath, O_RDONLY);
  if (fd == -1) return -errno;

  res = pread(fd, buf, size, offset);
  if (res == -1) res = -errno;

  close(fd);
  return res;
}

//menuliskan data dr read yg disimpan dr memory ke file yg dibuka
static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
  int res;
  int fd;
  char fpath[1000];

  sprintf(fpath, "%s%s",dirpath,path);

  (void) fi;
  fd = open(fpath, O_WRONLY);
  if (fd == -1) return -errno;

  res = pwrite(fd, buf, size, offset);
  if (res == -1) res = -errno;

  close(fd);
  return res;
}

//mengganti alokasi memori tiap file
static int xmp_truncate(const char *path, off_t size){
  int res;
  char fpath[1000];
  sprintf(fpath,"%s%s", dirpath, path);
  res = truncate(fpath, size);
  if(res == -1)
    return -errno;

  return 0;
}

static struct fuse_operations xmp_oper = {
  .getattr  = xmp_getattr,
  .mknod = xmp_mknod,
  .chmod = xmp_chmod,
  .rename = xmp_rename,
  .readdir = xmp_readdir,
  .read = xmp_read,
  .write = xmp_write,
  .truncate = xmp_truncate,
};

int main(int argc, char *argv[])
{
  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);
}