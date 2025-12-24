savedcmd_hello-4.mod := printf '%s\n'   hello-4.o | awk '!x[$$0]++ { print("./"$$0) }' > hello-4.mod
