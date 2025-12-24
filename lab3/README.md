#  Lab3 Concepts
## The /proc filesystem
* 用途
    * 最初的功能: 讓 kernel, kernel modules 能去 access process 的資訊
    * 現在的功能: 除了以上的功能以外，還有其他像是 modules, memory 等相關 kernel 資訊也能透過 /proc filesystem 去取得
        ```bash
        $ cat /proc/meminfo
        $ cat /proc/modules
        ```
* 概念
    * 其實 /proc 是個 pseudo/virtual filesystem，他並不會掛載到 disk 上面，而是只存在在 memory 中，所以斷電的話 /proc 就會清掉，開機再重建

* 歷史
    * 在以前的 UNIX 中，透過 `ps` 去 access processes 的資訊，而要獲取 processes 的資訊就必須經由 read from kernel's virtual memory 去取得，所以 `ps` 必須在實作成 privileged program 才能取得資訊
    * 而如今的 Linux 則不需要讓 `ps` 做成 privileged program，用一般的 program 時做就好，不須特權。/proc 的內容是由 kernel 及時產生的資訊，所以如果要 processes 資訊的話就簡單地 parse files 的內容就好。
