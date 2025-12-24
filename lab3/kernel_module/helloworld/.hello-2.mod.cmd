savedcmd_hello-2.mod := printf '%s\n'   hello-2.o | awk '!x[$$0]++ { print("./"$$0) }' > hello-2.mod
