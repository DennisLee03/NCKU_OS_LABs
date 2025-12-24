savedcmd_hello-1.mod := printf '%s\n'   hello-1.o | awk '!x[$$0]++ { print("./"$$0) }' > hello-1.mod
