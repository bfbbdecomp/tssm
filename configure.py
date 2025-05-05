#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GGVE78",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20240706"
config.dtk_tag = "v1.4.1"
config.objdiff_tag = "v2.7.1"
config.sjiswrap_tag = "v1.2.0"
config.wibo_tag = "0.6.11"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
    f"--defsym VERSION_{config.version}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-w off",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-common off",
    "-inline auto",
    "-cwd source",
    "-i src/dolphin",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

cflags_tssm = [
    *cflags_base,
    "-common on",
    "-char unsigned",
    "-str reuse,readonly",
    "-use_lmw_stmw on",
    '-pragma "cpp_extensions on"',
    "-inline off",
    "-gccinc",
    "-i include/bink", 
    "-i include/PowerPC_EABI_Support",
    "-i include/PowerPC_EABI_Support",
    "-i include/Dolphin",
    "-i include/inline",
    "-i include/rwsdk", 
    "-i src/SB/Core/gc",
    "-i src/SB/Core/x",
    "-i src/SB/Game",
    "-i src/dolphin",
    "-DGAMECUBE",
]

config.linker_version = "GC/2.6"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }

# Helper function for RenderWare libraries
def RenderWareLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {

        "lib": "SB",
        "mw_version": config.linker_version,
        "cflags": cflags_tssm,
        "progress_category": "game",
        "objects": [
            Object(NonMatching, "SB/Core/x/xWad4.cpp"),
            Object(NonMatching, "SB/Core/x/xWad2.cpp"),
            Object(NonMatching, "SB/Core/x/xWad3.cpp"),
            Object(NonMatching, "SB/Core/x/xWad1.cpp"),
            Object(NonMatching, "SB/Core/x/xWad5.cpp"),
            Object(NonMatching, "SB/Core/x/xFXHighDynamicRange.cpp"),
            Object(NonMatching, "SB/Core/x/xFXCameraTexture.cpp"),
            Object(NonMatching, "SB/Core/x/xModelBlur.cpp"),
            Object(NonMatching, "SB/Core/x/xCamera.cpp"),
            Object(NonMatching, "SB/Game/zWadNME.cpp"),
            Object(NonMatching, "SB/Game/zWad1.cpp"), 
            Object(NonMatching, "SB/Game/zWad2.cpp"),
            Object(NonMatching, "SB/Game/zWad3.cpp"),
            Object(NonMatching, "SB/Game/zWadEnt.cpp"),
            Object(NonMatching, "SB/Game/zWadHud.cpp"),
            Object(NonMatching, "SB/Game/zWadUI.cpp"),
            Object(NonMatching, "SB/Game/zMain.cpp"),
            Object(NonMatching, "SB/Game/zTalkBox.cpp"),
            Object(NonMatching, "SB/Game/zTaskBox.cpp"),
            Object(NonMatching, "SB/Game/zSmoke.cpp"),
            Object(NonMatching, "SB/Game/zSplash.cpp"),
            Object(NonMatching, "SB/Game/zExplosion.cpp"),
            Object(NonMatching, "SB/Game/zFMV.cpp"),
            Object(NonMatching, "SB/Game/zJSPExtraData.cpp"),
            Object(NonMatching, "SB/Game/zTextBox.cpp"),
            Object(NonMatching, "SB/Game/zDust.cpp"),
            Object(NonMatching, "SB/Game/zReactiveAnimation.cpp"),
            Object(NonMatching, "SB/Game/zFire.cpp"),
            Object(NonMatching, "SB/Game/zBossCam.cpp"),
            Object(NonMatching, "SB/Game/zParticleGenerator.cpp"),
            Object(NonMatching, "SB/Game/zParticleLocator.cpp"),
            Object(NonMatching, "SB/Game/zParticleSystems.cpp"),
            Object(NonMatching, "SB/Game/zParticleSystemWaterfall.cpp"),
            Object(NonMatching, "SB/Game/zWadCine.cpp"),
            Object(NonMatching, "SB/Game/zParticleCustom.cpp"),
            Object(NonMatching, "SB/Core/gc/iWad.cpp"),
            Object(NonMatching, "SB/Core/gc/iTRC.cpp"),
            Object(Matching, "SB/Core/gc/iException.cpp"),
            Object(NonMatching, "SB/Core/gc/iScrFX.cpp"),
            Object(NonMatching, "SB/Core/gc/iARAMTmp.cpp"),
        
        ],
    },
 {

        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(NonMatching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
        ],
    },
 {
        "lib": "binkngc",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_runtime,
        "progress_category": "sdk",
        "objects": [
            # not even sure bink is used
            Object(NonMatching, "bink/src/sdk/decode/ngc/binkngc.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcsnd.c"),
            Object(NonMatching, "bink/src/sdk/decode/binkread.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcfile.c"),
            Object(NonMatching, "bink/src/sdk/decode/yuv.cpp"),
            Object(NonMatching, "bink/src/sdk/decode/binkacd.c"),
            Object(NonMatching, "bink/shared/time/radcb.c"),
            Object(NonMatching, "bink/src/sdk/decode/expand.c"),
            Object(NonMatching, "bink/src/sdk/popmal.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcrgb.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcyuy2.c"),
            Object(NonMatching, "bink/src/sdk/varbits.c"),
            Object(NonMatching, "bink/src/sdk/fft.c"),
            Object(NonMatching, "bink/src/sdk/dct.c"),
            Object(NonMatching, "bink/src/sdk/bitplane.c"),
        ],
    },
    {
        "lib": "TRK_MINNOW_DOLPHIN",
        "cflags": [*cflags_runtime, "-inline deferred", "-sdata 0", "-sdata2 0"],
        "mw_version": "GC/2.6",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/mainloop.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/nubevent.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/nubinit.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/msg.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/msgbuf.c"),
            Object(
                Matching,
                "Dolphin/TRK_MINNOW_DOLPHIN/serpoll.c",
                extra_cflags=["-sdata 8"],
            ),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/usr_put.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/dispatch.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/msghndlr.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/support.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/mutex_TRK.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/notify.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/flush_cache.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/mem_TRK.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/targimpl.c"),
            Object(
                Matching,
                "Dolphin/TRK_MINNOW_DOLPHIN/targsupp.c",
                extra_cflags=["-func_align 32"],
            ),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/mpc_7xx_603e.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/__exception.s"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/dolphin_trk.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/main_TRK.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/dolphin_trk_glue.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/targcont.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/target_options.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/mslsupp.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/UDP_Stubs.c"),
            Object(
                Matching,
                "Dolphin/TRK_MINNOW_DOLPHIN/ddh/main.c",
                extra_cflags=["-sdata 8"],
            ),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/CircleBuffer.c"),
            Object(
                Matching,
                "Dolphin/TRK_MINNOW_DOLPHIN/gdev/main.c",
                extra_cflags=["-sdata 8"],
            ),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/MWTrace.c"),
            Object(Matching, "Dolphin/TRK_MINNOW_DOLPHIN/MWCriticalSection_gc.cpp"),
        ],
    },
    {
        "lib": "Runtime",
        "cflags": [*cflags_runtime, "-inline deferred"],
        "mw_version": "GC/2.6",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/Runtime/__va_arg.c"),
            Object(Matching, "Dolphin/Runtime/global_destructor_chain.c"),
            Object(Matching, "Dolphin/Runtime/CPlusLibPPC.cp"),
            Object(
                Matching,
                "Dolphin/Runtime/NMWException.cp",
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Dolphin/Runtime/ptmf.c"),
            Object(Matching, "Dolphin/Runtime/runtime.c"),
            Object(Matching, "Dolphin/Runtime/__init_cpp_exceptions.cpp"),
            Object(
                Matching,
                "Dolphin/Runtime/Gecko_ExceptionPPC.cp",
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Dolphin/Runtime/GCN_mem_alloc.c"),
            Object(Matching, "Dolphin/Runtime/__mem.c"),
        ],
    },
    {
        "lib": "MSL_C",
        "cflags": [*cflags_runtime, "-inline deferred"],
        "mw_version": "GC/2.6",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/MSL_C/PPC_EABI/abort_exit.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/alloc.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/ansi_files.c"),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/ansi_fp.c",
                extra_cflags=["-str pool"],
            ),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/arith.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/buffer_io.c"),
            Object(Matching, "Dolphin/MSL_C/PPC_EABI/critical_regions.gamecube.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/ctype.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/direct_io.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/errno.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/file_io.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/FILE_POS.C"),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common/locale.c",
                extra_cflags=["-str pool"],
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common/mbstring.c",
                extra_cflags=["-inline noauto,deferred"],
            ),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/mem.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/mem_funcs.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/misc_io.c"),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common/printf.c",
                extra_cflags=["-str pool"],
            ),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/rand.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/float.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/scanf.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/string.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/strtold.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/strtoul.c"),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/wchar_io.c"),
            Object(Matching, "Dolphin/MSL_C/PPC_EABI/uart_console_io_gcn.c"),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_asin.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_atan2.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_exp.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log10.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_cos.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_sin.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_tan.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_atan.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ceil.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_copysign.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_cos.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_floor.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_frexp.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_modf.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_sin.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_tan.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_asin.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_atan2.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_exp.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_fmod.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_log10.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_pow.c",
            ),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_sqrt.c",
            ),
            Object(Matching, "Dolphin/MSL_C/PPC_EABI/math_ppc.c"),
            Object(
                Matching,
                "Dolphin/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_sqrt.c",
            ),
            Object(Matching, "Dolphin/MSL_C/MSL_Common/extras.c"),
        ],
    },
    {
        "lib": "OdemuExi2",
        "cflags": [*cflags_runtime, "-inline deferred"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/OdemuExi2/DebuggerDriver.c")],
    },
    {
        "lib": "vi",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/vi/vi.c")],
    },
    {
        "lib": "amcstubs",
        "cflags": cflags_runtime,
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/amcstubs/AmcExi2Stubs.c")],
    },
    {
        "lib": "ar",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/ar/ar.c"),
            Object(Matching, "Dolphin/ar/arq.c"),
        ],
    },
    {
        "lib": "ax",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/ax/AX.c"),
            Object(Matching, "Dolphin/ax/AXAlloc.c"),
            Object(Matching, "Dolphin/ax/AXAux.c"),
            Object(Matching, "Dolphin/ax/AXCL.c"),
            Object(Matching, "Dolphin/ax/AXOut.c"),
            Object(Matching, "Dolphin/ax/AXSPB.c"),
            Object(Matching, "Dolphin/ax/AXVPB.c"),
            Object(Matching, "Dolphin/ax/AXComp.c"),
            Object(Matching, "Dolphin/ax/DSPCode.c"),
            Object(Matching, "Dolphin/ax/AXProf.c"),
        ],
    },
    {
        "lib": "axfx",
        "cflags": cflags_runtime,
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/axfx/reverb_hi.c"),
            Object(Matching, "Dolphin/axfx/reverb_std.c"),
            Object(Matching, "Dolphin/axfx/chorus.c"),
            Object(Matching, "Dolphin/axfx/delay.c"),
            Object(Matching, "Dolphin/axfx/axfx.c"),
            Object(Matching, "Dolphin/axfx/reverb_hi_4ch.c")
        ],
    },
    {
        "lib": "base",
        "cflags": cflags_runtime,
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/base/PPCArch.c")],
    },
    {
        "lib": "mix",
        "cflags": cflags_runtime,
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/mix/mix.c")],
    },
    {
        "lib": "card",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/card/CARDBios.c"),
            Object(Matching, "Dolphin/card/CARDUnlock.c"),
            Object(Matching, "Dolphin/card/CARDRdwr.c"),
            Object(Matching, "Dolphin/card/CARDBlock.c"),
            Object(Matching, "Dolphin/card/CARDDir.c"),
            Object(Matching, "Dolphin/card/CARDCheck.c"),
            Object(Matching, "Dolphin/card/CARDMount.c"),
            Object(Matching, "Dolphin/card/CARDFormat.c"),
            Object(Matching, "Dolphin/card/CARDOpen.c"),
            Object(Matching, "Dolphin/card/CARDCreate.c"),
            Object(Matching, "Dolphin/card/CARDRead.c"),
            Object(Matching, "Dolphin/card/CARDWrite.c"),
            Object(Matching, "Dolphin/card/CARDStat.c"),
            Object(Matching, "Dolphin/card/CARDNet.c"),
            Object(Matching, "Dolphin/card/CARDDelete.c"),
            Object(Matching, "Dolphin/card/CARDStatEx.c"),
        ],
    },
    {
        "lib": "db",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/db/db.c")],
    },
    {
        "lib": "dsp",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/dsp/dsp.c"),
            Object(Matching, "Dolphin/dsp/dsp_debug.c"),
            Object(Matching, "Dolphin/dsp/dsp_task.c"),
        ],
    },
    {
        "lib": "dvd",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/dvd/dvdlow.c"),
            Object(Matching, "Dolphin/dvd/dvdfs.c"),
            Object(Matching, "Dolphin/dvd/dvd.c"),
            Object(Matching, "Dolphin/dvd/dvdqueue.c"),
            Object(Matching, "Dolphin/dvd/dvderror.c"),
            Object(Matching, "Dolphin/dvd/dvdidutils.c"),
            Object(Matching, "Dolphin/dvd/dvdFatal.c"),
            Object(Matching, "Dolphin/dvd/fstload.c"),
        ],
    },
    {
        "lib": "exi",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/exi/EXIBios.c"),
            Object(Matching, "Dolphin/exi/EXIUart.c"),
        ],
    },
    {
        "lib": "gd",
        "cflags": cflags_runtime,
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/gd/GDBase.c"),
            Object(Matching, "Dolphin/gd/GDGeometry.c"),
        ],
    },
    {
        "lib": "gx",
        "cflags": [*cflags_runtime, "-str noreadonly", "-fp_contract off"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/gx/GXInit.c"),
            Object(Matching, "Dolphin/gx/GXFifo.c"),
            Object(Matching, "Dolphin/gx/GXAttr.c"),
            Object(Matching, "Dolphin/gx/GXMisc.c"),
            Object(Matching, "Dolphin/gx/GXGeometry.c"),
            Object(Matching, "Dolphin/gx/GXFrameBuf.c"),
            Object(Matching, "Dolphin/gx/GXLight.c"),
            Object(Matching, "Dolphin/gx/GXTexture.c"),
            Object(Matching, "Dolphin/gx/GXBump.c"),
            Object(Matching, "Dolphin/gx/GXTev.c"),
            Object(Matching, "Dolphin/gx/GXPixel.c"),
            Object(Matching, "Dolphin/gx/GXDisplayList.c"),
            Object(Matching, "Dolphin/gx/GXTransform.c"),
            Object(Matching, "Dolphin/gx/GXPerf.c"),
        ],
    },
    {
        "lib": "mtx",
        "cflags": cflags_runtime,
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/mtx/mtx.c"),
            Object(Matching, "Dolphin/mtx/mtxvec.c"),
            Object(Matching, "Dolphin/mtx/mtx44.c"),
            Object(Matching, "Dolphin/mtx/vec.c"),
        ],
    },
    {
        "lib": "odenotstub",
        "cflags": cflags_runtime,
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/odenotstub/odenotstub.c")],
    },
    {
        "lib": "os",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/os/OS.c"),
            Object(Matching, "Dolphin/os/OSAlarm.c"),
            Object(Matching, "Dolphin/os/OSAlloc.c"),
            Object(Matching, "Dolphin/os/OSArena.c"),
            Object(Matching, "Dolphin/os/OSAudioSystem.c"),
            Object(Matching, "Dolphin/os/OSCache.c"),
            Object(Matching, "Dolphin/os/OSContext.c"),
            Object(Matching, "Dolphin/os/OSError.c"),
            Object(Matching, "Dolphin/os/OSExec.c"),
            Object(Matching, "Dolphin/os/OSFont.c"),
            Object(Matching, "Dolphin/os/OSInterrupt.c"),
            Object(Matching, "Dolphin/os/OSLink.c"),
            Object(Matching, "Dolphin/os/OSMessage.c"),
            Object(Matching, "Dolphin/os/OSMemory.c"),
            Object(Matching, "Dolphin/os/OSMutex.c"),
            Object(Matching, "Dolphin/os/OSReboot.c"),
            Object(Matching, "Dolphin/os/OSReset.c"),
            Object(Matching, "Dolphin/os/OSResetSW.c"),
            Object(Matching, "Dolphin/os/OSRtc.c"),
            Object(Matching, "Dolphin/os/OSSync.c"),
            Object(Matching, "Dolphin/os/OSSemaphore.c"),
            Object(Matching, "Dolphin/os/OSThread.c"),
            Object(Matching, "Dolphin/os/OSTime.c"),
            Object(Matching, "Dolphin/os/__start.c"),
            Object(Matching, "Dolphin/os/__ppc_eabi_init.cpp"),
        ],
    },
    {
        "lib": "pad",
        "cflags": [*cflags_runtime, "-fp_contract off", "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/pad/Padclamp.c"),
            Object(Matching, "Dolphin/pad/Pad.c"),
        ],
    },
    {
        "lib": "si",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [
            Object(Matching, "Dolphin/si/SIBios.c"),
            Object(Matching, "Dolphin/si/SISamplingRate.c"),
        ],
    },
    {
        "lib": "ai",
        "cflags": [*cflags_runtime, "-str noreadonly"],
        "mw_version": "GC/1.2.5n",
        "progress_category" : "sdk",
        "host": False,
        "objects": [Object(Matching, "Dolphin/ai/ai.c")],
    },
    RenderWareLib(
        "rpcollis",
        [
            Object(NonMatching, "rwsdk/plugin/collis/ctgeom.c"),
            Object(NonMatching, "rwsdk/plugin/collis/ctworld.c"),
            Object(NonMatching, "rwsdk/plugin/collis/ctbsp.c"),
            Object(NonMatching, "rwsdk/plugin/collis/rpcollis.c"),
        ],
    ),
    RenderWareLib(
        "rphanim",
        [
            Object(NonMatching, "rwsdk/plugin/hanim/stdkey.c"),
            Object(NonMatching, "rwsdk/plugin/hanim/rphanim.c"),
        ],
    ),
    RenderWareLib(
        "rpmatfx",
        [
            Object(NonMatching, "rwsdk/plugin/matfx/gcn/effectPipesGcn.c"),
            Object(NonMatching, "rwsdk/plugin/matfx/gcn/multiTexGcnData.c"),
            Object(NonMatching, "rwsdk/plugin/matfx/gcn/multiTexGcnPipe.c"),
            Object(NonMatching, "rwsdk/plugin/matfx/gcn/multiTexGcn.c"),
            Object(NonMatching, "rwsdk/plugin/matfx/multiTex.c"),
            Object(NonMatching, "rwsdk/plugin/matfx/multiTexEffect.c"),
            Object(NonMatching, "rwsdk/plugin/matfx/rpmatfx.c"),
        ],
    ),
    RenderWareLib(
        "rpptank",
        [
            Object(NonMatching, "rwsdk/plugin/ptank/rpptank.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcncallbacks.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcnrender.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcntransforms.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_nc_ppm.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_cc_ppm.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_nc_cs_nr.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_cc_cs_nr.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_nc_pps_nr.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_cc_pps_nr.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_nc_cs_ppr.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_cc_cs_ppr.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_nc_pps_ppr.c"),
            Object(NonMatching, "rwsdk/plugin/ptank/gcn/ptankgcn_cc_pps_ppr.c"),
        ],
    ),
    RenderWareLib(
        "rpskinmatfx",
        [
            Object(NonMatching, "rwsdk/plugin/skin2/bsplit.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/rpskin.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skingcn.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skinstream.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/instance/instanceskin.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skinmatrixblend.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skingcnasm.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skingcng.c"),
        ],
    ),
    RenderWareLib(
        "rpusrdat",
        [
            Object(NonMatching, "rwsdk/plugin/userdata/rpusrdat.c"),
        ],
    ),
    RenderWareLib(
        "rpworld",
        [
            Object(NonMatching, "rwsdk/world/babinwor.c"),
            Object(NonMatching, "rwsdk/world/baclump.c"),
            Object(NonMatching, "rwsdk/world/bageomet.c"),
            Object(NonMatching, "rwsdk/world/balight.c"),
            Object(NonMatching, "rwsdk/world/bamateri.c"),
            Object(NonMatching, "rwsdk/world/bamatlst.c"),
            Object(NonMatching, "rwsdk/world/bamesh.c"),
            Object(NonMatching, "rwsdk/world/bameshop.c"),
            Object(NonMatching, "rwsdk/world/basector.c"),
            Object(NonMatching, "rwsdk/world/baworld.c"),
            Object(NonMatching, "rwsdk/world/baworobj.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/bapipew.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/gcpipe.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/vtxfmt.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/wrldpipe.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/nodeGameCubeAtomicAllInOne.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/nodeGameCubeWorldSectorAllInOne.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/gclights.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/gcmorph.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/native.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/setup.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/geomcond.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/geominst.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/ibuffer.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/instancegeom.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/instanceworld.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/itools.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/vbuffer.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/vtools.c"),
            Object(NonMatching, "rwsdk/world/pipe/p2/gcn/instance/vtxdesc.c"),
        ],
    ),
    RenderWareLib(
        "rtanim",
        [
            Object(NonMatching, "rwsdk/tool/anim/rtanim.c"),
        ],
    ),
    RenderWareLib(
        "rtintsec",
        [
            Object(NonMatching, "rwsdk/tool/intsec/rtintsec.c"),
        ],
    ),
    RenderWareLib(
        "rtslerp",
        [
            Object(NonMatching, "rwsdk/tool/slerp/rtslerp.c"),
        ],
    ),
    RenderWareLib(
        "rwcore",
        [
            Object(NonMatching, "rwsdk/src/plcore/babinary.c"),
            Object(NonMatching, "rwsdk/src/plcore/bacolor.c"),
            Object(NonMatching, "rwsdk/src/plcore/baerr.c"),
            Object(NonMatching, "rwsdk/src/plcore/bafsys.c"),
            Object(NonMatching, "rwsdk/src/plcore/baimmedi.c"),
            Object(NonMatching, "rwsdk/src/plcore/bamatrix.c"),
            Object(NonMatching, "rwsdk/src/plcore/bamemory.c"),
            Object(NonMatching, "rwsdk/src/plcore/baresour.c"),
            Object(NonMatching, "rwsdk/src/plcore/bastream.c"),
            Object(NonMatching, "rwsdk/src/plcore/batkbin.c"),
            Object(NonMatching, "rwsdk/src/plcore/batkreg.c"),
            Object(NonMatching, "rwsdk/src/plcore/bavector.c"),
            Object(NonMatching, "rwsdk/src/plcore/resmem.c"),
            Object(NonMatching, "rwsdk/src/plcore/rwstring.c"),
            Object(NonMatching, "rwsdk/os/gcn/osintf.c"),
            Object(NonMatching, "rwsdk/src/babbox.c"),
            Object(NonMatching, "rwsdk/src/babincam.c"),
            Object(NonMatching, "rwsdk/src/babinfrm.c"),
            Object(NonMatching, "rwsdk/src/babintex.c"),
            Object(NonMatching, "rwsdk/src/bacamera.c"),
            Object(NonMatching, "rwsdk/src/badevice.c"),
            Object(NonMatching, "rwsdk/src/baframe.c"),
            Object(NonMatching, "rwsdk/src/baimage.c"),
            Object(NonMatching, "rwsdk/src/baimras.c"),
            Object(NonMatching, "rwsdk/src/baraster.c"),
            Object(NonMatching, "rwsdk/src/baresamp.c"),
            Object(NonMatching, "rwsdk/src/basync.c"),
            Object(NonMatching, "rwsdk/src/batextur.c"),
            Object(NonMatching, "rwsdk/src/batypehf.c"),
            Object(NonMatching, "rwsdk/driver/common/palquant.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dl2drend.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dlconvrt.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dldevice.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dlraster.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dlrendst.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dlsprite.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dltexdic.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dltextur.c"),
            Object(NonMatching, "rwsdk/driver/gcn/dltoken.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/baim3d.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/bapipe.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/p2altmdl.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/p2core.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/p2define.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/p2dep.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/p2heap.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/p2renderstate.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/p2resort.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/gcn/im3dpipe.c"),
            Object(NonMatching, "rwsdk/src/pipe/p2/gcn/nodeDolphinSubmitNoLight.c"),
        ],
    ),

]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects

# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress and write progress.json
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
