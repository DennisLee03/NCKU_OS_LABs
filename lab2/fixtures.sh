#!/bin/bash
cat > demo.txt <<'EOF'
OS lab is fun.
grep this line.
Pipeline tests with tail and head.
apple banana carrot
another OS line
ERROR: disk full
warn: low battery
Info: charging
error: overheating
ERROR: kernel panic
EOF

cat > lorem.txt <<'EOF'
Lorem ipsum dolor sit amet,
consectetur adipiscing elit.
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam,
quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.
Excepteur sint occaecat cupidatat non proident,
sunt in culpa qui officia deserunt mollit anim id est laborum.
EOF

cat > nums.txt <<'EOF'
10
9
8
7
7
7
3
3
2
1
EOF

cat > utf8.txt <<'EOF'
中文行一
第二行 with English
emoji: 😀😎🔥
最後一行
EOF

cat > sample.log <<'EOF'
INFO starting service
WARN deprecated API
ERROR failed to bind
INFO retrying
ERROR connection refused
error timeout
WARN low memory
INFO done
ERROR connection refused
error timeout
EOF

: > empty.txt
echo "[fixtures] OK"
