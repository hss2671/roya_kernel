#!/bin/bash

echo -e "==========================="
echo -e "= START COMPILING KERNEL  ="
echo -e "==========================="
bold=$(tput bold)
normal=$(tput sgr0)

export KBUILD_BUILD_USER="build-user"
export TZ=Asia/Jakarta
export KBUILD_BUILD_VERSION=1
export KBUILD_BUILD_TIMESTAMP="$(date)"
export KBUILD_BUILD_HOST="build-host"
export KERNELDIR="$(pwd)"
export KERNELNAME=""
export SRCDIR="${KERNELDIR}"
export OUTDIR="${KERNELDIR}/out"
export ANYKERNEL="${KERNELDIR}/AnyKernel3"
export DEFCONFIG="vendor/miholi-qgki_defconfig"
export ZIP_DIR="${KERNELDIR}/files"
export IMAGE="${OUTDIR}/arch/arm64/boot/Image"
export DTB="${OUTDIR}/arch/arm64/boot/dts/vendor/xiaomi/stone.dtb"
export DTBO="${OUTDIR}/arch/arm64/boot/dtbo.img"
export VARI=""
export PATH="$(pwd)/../clang/bin:$PATH"

NEEDVER=false
if [[ "$NEEDVER" == true ]]; then
RELEASE_MODE=false
if [[ "$RELEASE" == "1" ]] || [[ "$RELEASE" == "true" ]]; then
    RELEASE_MODE=true
fi
fi

while (( ${#} )); do
    case ${1} in
        "-r"|"--regen") REGEN=true ;;
        "-c"|"--clean") CLEAN=true ;;
        "--release") RELEASE_MODE=true ;;
    esac
    shift
done

[[ -z ${ZIP} ]] && { echo "${bold}LOADING-_-....${normal}"; }

if [[ "$CLEAN" == true ]]; then
    echo "Cleaning out directory..."
    rm -rf out
fi

if [[ "$REGEN" == true ]]; then
    echo "Regenerating defconfig..."
    mkdir -p out
    make O=out ARCH=arm64 $DEFCONFIG savedefconfig
    cp out/defconfig arch/arm64/configs/$DEFCONFIG
    echo "Defconfig saved to arch/arm64/configs/$DEFCONFIG"
    exit
fi

if [[ "$NEEDVER" == true ]]; then
if [[ "$RELEASE_MODE" == "true" ]]; then
    COUNTER_FILE="${KERNELDIR}/.countr"
    SUFFIX_PREFIX="-r"
else
    COUNTER_FILE="${KERNELDIR}/.countrc"
    SUFFIX_PREFIX="-rc"
fi

if [ -f "$COUNTER_FILE" ]; then
    COUNTER=$(cat "$COUNTER_FILE")
    COUNTER=$((COUNTER + 1))
else
    CURRENT_EXTRA=$(grep -E "^EXTRAVERSION =.*${SUFFIX_PREFIX}" "${KERNELDIR}/Makefile" | sed -E "s/.*${SUFFIX_PREFIX}([0-9]+).*/\1/")
    if [ ! -z "$CURRENT_EXTRA" ]; then
        COUNTER=$((CURRENT_EXTRA + 1))
    else
        COUNTER=1
    fi
fi
echo "$COUNTER" > "$COUNTER_FILE"

sed -i "s/^EXTRAVERSION =.*/EXTRAVERSION = ${SUFFIX_PREFIX}${COUNTER}/" "${KERNELDIR}/Makefile"
fi

rm -f "$IMAGE"
mkdir -p out
echo "Generating .config..."
make O=out ARCH=arm64 $DEFCONFIG
echo -e "${bold}Compiling Kernel...${normal}"
make -j$(nproc --all) \
    O=out \
    ARCH=arm64 \
    CC="ccache clang" \
    LD="ccache ld.lld" \
    LLVM=1 \
    CROSS_COMPILE=aarch64-linux-gnu- \
    CROSS_COMPILE_ARM32=arm-linux-gnueabi- \
    KCFLAGS="-fno-ident \
             -fno-unwind-tables \
             -fno-asynchronous-unwind-tables \
             -fno-unroll-loops \
             -fdiscard-value-names \
             -fmerge-all-constants \
             -fno-standalone-debug \
             -g0 \
             -mtune=cortex-a78 \
             -falign-functions=4 \
             -fno-semantic-interposition \
             -fno-plt \
             -fno-math-errno \
             -mno-outline-atomics" 2>&1 | tee log.txt


echo -e "==========================="
echo -e "   COMPILE KERNEL COMPLETE "
echo -e "==========================="

echo -e "${bold}Proceeding to Packing Kernel...${normal}"
if [ ! -f "$IMAGE" ] || [ ! -d "$ANYKERNEL" ]; then
    echo -e "Abort: Image file ($IMAGE) or AnyKernel3 folder ($ANYKERNEL) is missing!"
    exit 1
fi

if [[ -z "$KERNELNAME" || -z "$VARI" || -z "$COUNTER" ]]; then
    KVER=$(awk '/^VERSION =/{v=$3} /^PATCHLEVEL =/{p=$3} /^SUBLEVEL =/{s=$3} END{print v"."p"."s}' Makefile)
    export ZIPNAME="Kernel_${KVER}_$(echo "$DEFCONFIG" | sed 's|.*/||;s/_defconfig//')_$(date +%Y%m%d_%H%M%S).zip"
else
    if [[ "$RELEASE_MODE" == "true" ]]; then
        export ZIPNAME="Kernel_${KERNELNAME}_$(echo "$DEFCONFIG" | sed 's|.*/||;s/_defconfig//')_${VARI}_r${COUNTER}.zip"
    else
        export ZIPNAME="Kernel_${KERNELNAME}_$(echo "$DEFCONFIG" | sed 's|.*/||;s/_defconfig//')_${VARI}_rc${COUNTER}.zip"
    fi
fi
export FINAL_ZIP="${ZIP_DIR}/${ZIPNAME}"
rm -rf "${ZIP_DIR}"
mkdir -p "${ZIP_DIR}"
echo -e "Copying files to AnyKernel3"
cp -f "${IMAGE}" "${ANYKERNEL}/Image"
echo -e "copy Image"
if [ -f "$DTB" ]; then
    cp -f "$DTB" "${ANYKERNEL}/dtb"
    echo -e "copy dtb"
fi
if [ -f "$DTBO" ]; then
    cp -f "$DTBO" "${ANYKERNEL}/dtbo.img"
    echo -e "copy dtbo.img"
fi

cd "${ANYKERNEL}" || exit
echo -e "Zipping kernel..."
zip -r9 "${FINAL_ZIP}" * -x .git README.md *placeholder
echo -e "Delete Removing Image, dtb, dtbo.img..."
rm -f "Image" "dtb" "dtbo.img"
echo -e "Uploading to GoFile..."
curl -LSs "https://raw.githubusercontent.com/lordgaruda/GoFile-Upload/master/upload.sh" | bash -s "${FINAL_ZIP}"
cd "${KERNELDIR}" || exit

if [[ ":v" ]]; then
    exit
fi
