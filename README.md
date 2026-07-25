# smol-linux
A tiny busybox and musl based linux built from scratch that tries to stay as small as possible - webpage and releases
# usage
qemu-system-x86_64 -cdrom smol-bios.iso -m 512 -nographic (bios)
qemu-system-x86_64 -bios /usr/share/edk2/x64/OVMF.4m.fd -cdrom smol-uefi.iso -m 512 -nographic (uefi needs the edk2-ovmf package)
