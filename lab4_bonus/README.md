# Notes for Filesystem Lab
## Create File
* `osfs_create()`
* step 1: 把 VFS 傳下來的通用的 parent's directory inode 轉成 osfs 的 inode
    ```C
    struct osfs_inode *parent_inode = dir->i_private;
    ```
* step 2: 檢查 new file's name 的長度是否符合規範
    ```C
    if(dentry->d_name.len > MAX_FILENAME_LEN) return -ENAMETOOLONG;
    ```
* step 3: 在 parent directory: `dir` 下，創建一個新的通用的 struct inode `inode`，之後一樣幫它 parse 成 osfs_inode
    ```C
    inode = osfs_new_inode(dir, mode);
    osfs_inode = inode->i_private;
    ```
* step 4: 把 file: `dentry` 加到 parent directory: `dir` 之下
    ```C
    osfs_add_dir_entry(dir, osfs_inode->i_ino, dentry->d_name.name, dentry->d_name.len);
    ```
* step 5: 更新 timestamp, size
    ```C
    struct timespec64 now = current_time(dir);
    // access, modify, change times
    parent_inode->__i_atime = parent_inode->__i_mtime = now;
    inode_set_atime_to_ts(dir, now);
    inode_set_mtime_to_ts(dir, now);
    dir->i_size = parent_inode->i_size;
    ```
* step 6: 把 entry 與 inode 綁定，使得 VFS 能透透過 entry 找到 inode
    ```C
    d_instantiate(dentry, inode);
    ```
## Write File
* `osfs_write()`
* step 1: 先從要寫入的 file 的 file pointer 去取的 inode, super block 的資訊
    ```C
    struct inode *inode = file_inode(filp);
    struct osfs_inode *osfs_inode = inode->i_private;
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;
    ```
* step 2: 檢查要被寫入的 file 的 data block pointer 有沒有被分配了
    * 這邊的 data block pointer 是 index 而已，感覺就是看看這個 pointer 有沒有分配給他一個 index 讓他去索引去使用 data block
    ```C
    if(osfs_inode->i_blocks == 0) {
        osfs_alloc_data_block(sb_info, &osfs_inode->i_block);
        osfs_inode->i_blocks++;
    }
    ```
* step 3: 檢查目前的 write head + len 是否有超過一個 block 的大小(用來限制他不能寫超過一個 block)
    ```C
    if(*ppos + len > BLOCK_SIZE) len = BLOCK_SIZE - *ppos;
    ```
* step 4: 計算要寫入的位置並且寫進去
    ```C
    data_block = sb_info->data_blocks + osfs_inode->i_block * BLOCK_SIZE + *ppos;
    if (copy_from_user(data_block, buf, len)) return -EFAULT;
    *ppos += len;
    bytes_written = len;
    ```
    * `sb_info->data_blocks`: data blocks 的 base address，好比 &array[0] 這樣
    * `osfs_inode->i_block * BLOCK_SIZE`: offset 到可以寫的 block 去
    * `*ppos`: 再從可以寫得 block， offset 到可以寫的 byte 去
    ```C
    unsigned long __copy_from_user(void * to, const void __user * from, unsigned long n);
    ```
* step 5: 更新 inode attributes: timestamp, size
    ```C
    struct timespec64 now = current_time(inode);
    osfs_inode->__i_atime = osfs_inode->__i_mtime = now;
    inode_set_atime_to_ts(inode, now);
    inode_set_mtime_to_ts(inode, now);
    uint32_t size = (uint32_t)*ppos > osfs_inode->i_size ? *ppos : osfs_inode->i_size;
    osfs_inode->i_size = inode->i_size = size;
    ```
    * `uint_32_t size = (uint_32_t)*ppos > osfs_inode->i_size ? *ppos : osfs_inode->i_size;`
    ![alt text](img/osfs_write.png)

## Test
```bash
# build and test
$ make 
$ sudo insmod osfs.ko
$ mkdir mnt
$ sudo mount -t osfs none mnt/
$ cd mnt
$ sudo touch test.txt
$ sudo bash -c "echo 'I LOVE OSLAB' > test.txt"
$ cat test.txt

# unmount osfs and remove its module
$ cd ..
$ sudo umount mnt
$ sudo rmmod osfs
$ make clean
$ rm -rf mnt
```