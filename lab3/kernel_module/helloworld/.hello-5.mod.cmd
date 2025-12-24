savedcmd_hello-5.mod := printf '%s\n'   hello-5.o | awk '!x[$$0]++ { print("./"$$0) }' > hello-5.mod
