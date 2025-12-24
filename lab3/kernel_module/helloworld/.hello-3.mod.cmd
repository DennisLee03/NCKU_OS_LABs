savedcmd_hello-3.mod := printf '%s\n'   hello-3.o | awk '!x[$$0]++ { print("./"$$0) }' > hello-3.mod
