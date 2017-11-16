
#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
#define _XOPEN_SOURCE 700
#endif

static const char *dirpath = "/home/titut/Downloads"; //direktori yang dimount

//fungsi untuk mengambil atribut. fungsi ini akan dipanggil setiap fungsi xmp readdir membaca isi direktori 
static int xmp_getattr(const char *path, struct stat *stbuf){
  int res;
  char fpath[1000];
  
  sprintf(fpath,"%s%s",dirpath,path);

  res = lstat(fpath, stbuf);

  if(res == -1) return -errno;
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

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
  char fpath[1000], lala[1000];

  sprintf(lala, path);

  if(strcmp(path,"/") == 0){
    path=dirpath;
    sprintf(fpath,"%s", path);
  }
  else{
    sprintf(fpath, "%s%s",dirpath,path);
  }

  int res = 0;
  int fd = 0;

  char ext[4]; //extensionnya
  int i, l = strlen(fpath); //lalala.jpg
  for(i = 4; i >= 1; i--){
    ext[4-i] = fpath[l-i];
  }

  if(strcmp(ext, ".doc") == 0 || strcmp(ext, ".txt") == 0 || strcmp(ext, ".pdf") == 0){
    char newfile[1000];
    
    (void) fi;
    fd = open(fpath, -1);

    system("mkdir /home/titut/Downloads/rahasia");
    //system("notify-send \"Warning!\" \"Terjadi Kesalahan! File berisi konten berbahaya.\n\" ");
 
    char arg[1000], arg1[1000], arg2[1000];
    sprintf(arg, "(sleep .1 && wmctrl -F -a \"Warning!\" -b add,above) & (zenity --warning --title=\"Warning!\" --text=\"");
    sprintf(arg, "(sleep .1 && wmctrl -F -a \"Warning!\" -b add,above) & (zenity --warning --title=\"Warning!\" --text=\"");
    // Kesalahan! File berisi konten berbahaya.\n\");

    system(arg);

    sprintf(newfile, "%s%s", fpath, path);
    sprintf(arg1, "mv %s %s", fpath, newfile);
    system(arg1);

    sprintf(arg2, "mv %s /home/titut/Downloads/rahasia", newfile);
    system(arg2);

    if (fd == -1) return -errno;
    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;

    close(fd);
    return res;
  }
  else{
    (void) fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;

    close(fd);
    return res;
  }
}

static struct fuse_operations xmp_oper = {
  .getattr  = xmp_getattr,
  .readdir  = xmp_readdir,
  .read   = xmp_read,
};

int main(int argc, char *argv[])
{
  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);
}