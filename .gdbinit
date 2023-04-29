set architecture aarch64
target remote localhost:1234
set substitute-path /chos/ ./
layout split
layout regs

file ./build/kernel.img
