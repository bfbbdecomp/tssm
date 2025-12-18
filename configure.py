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
config.objdiff_tag = "v3.3.1"
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
    # "-W all",
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
    "-i src/PowerPC_EABI_Support/include",
    "-i src/dolphin/include",
    "-i src/dolphin/src",
    "-i src",
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
    "-gccinc",
    "-common off",
    "-inline auto",
]

# dolphin library flags
cflags_dolphin = [
    *cflags_base,
    "-lang=c",
    "-fp fmadd",
    "-fp_contract off",
    "-char signed",
    "-str reuse",
    "-common off",
    "-O4,p",
    #"-requireprotos"
]

# Renderware library flags
cflags_renderware = [
    *cflags_base,
    "-lang=c",
    "-fp fmadd",
    "-fp_contract off",
    "-char signed",
    "-str reuse",
    "-common off",
    "-O4,p",
    #"-requireprotos"
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

cflags_tssm = [
    *cflags_base,
    "-lang=C++",
    "-common on",
    "-char unsigned",
    "-str reuse,pool,readonly",
    "-use_lmw_stmw on",
    '-pragma "cpp_extensions on"',
    "-inline auto",
    "-gccinc",
    "-i include/bink",
    "-i include/inline",
    "-i include/rwsdk",
    "-i src/SB/Core/gc",
    "-i src/SB/Core/x",
    "-i src/SB/Game",
    "-DGAMECUBE",
]

config.linker_version = "GC/2.6"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "src_dir": "src/",
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_dolphin,
        "progress_category": "sdk",
        "host": True,
        "objects": objects,
    }

    # Helper function for MSL libraries
def mslLib(lib_name: str, extra_cflags: List[str], objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "src_dir": "src/PowerPC_EABI_Support/src",
        "mw_version": "GC/2.6",
        "cflags": cflags_runtime + extra_cflags,
        "progress_category": "msl",
        "host": True,
        "objects": objects,
    }

def trkLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "src_dir": "src/runtime_libs",
        "mw_version": "GC/2.6",
        "cflags": cflags_runtime,
        "progress_category": "msl",
        "host": True,
        "objects": objects,
    }


# Helper function for RenderWare libraries
def RenderWareLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "src_dir": "src",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_renderware,
        "progress_category": "RW",
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
            Object(NonMatching, "SB/Core/x/xWad4.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Core/x/xWad2.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Core/x/xWad3.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Core/x/xWad1.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Core/x/xWad5.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Core/x/xFXHighDynamicRange.cpp"),
            Object(NonMatching, "SB/Core/x/xFXCameraTexture.cpp"),
            Object(NonMatching, "SB/Core/x/xModelBlur.cpp"),
            Object(NonMatching, "SB/Core/x/xCamera.cpp"),
            Object(NonMatching, "SB/Game/zWadNME.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zWad1.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zWad2.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zWad3.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zWadEnt.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zWadHud.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zWadUI.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zMain.cpp"),
            Object(NonMatching, "SB/Game/zTalkBox.cpp", extra_cflags=["-inline off"]),
            Object(NonMatching, "SB/Game/zTaskBox.cpp", extra_cflags=["-inline off"]),
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
            Object(NonMatching, "SB/Game/zWadCine.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "SB/Game/zParticleCustom.cpp"),
            Object(NonMatching, "SB/Core/gc/iWad.cpp", extra_cflags=["-sym on"]),
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

        "lib": "FMOD",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "FMOD",  # str | List[str]
        "objects": [
            Object(NonMatching, "FMOD/src/fmusic.c"),
            Object(NonMatching, "FMOD/src/format_fsb.c"),
            Object(NonMatching, "FMOD/src/format_gcadpcm.c"),
            Object(NonMatching, "FMOD/src/fsound.c"),
            Object(NonMatching, "FMOD/src/fsound_3d.c"),
            Object(NonMatching, "FMOD/src/fsound_device.c"),
            Object(NonMatching, "FMOD/src/fsound_async.c"),
            Object(NonMatching, "FMOD/src/fsound_channel.c"),
            Object(NonMatching, "FMOD/src/fsound_dsp.c"),
            Object(NonMatching, "FMOD/src/fsound_gc.c"),
            Object(NonMatching, "FMOD/src/fsound_sample.c"),
            Object(NonMatching, "FMOD/src/fsound_stream.c"),
            Object(NonMatching, "FMOD/src/mixer.c"),
            Object(NonMatching, "FMOD/src/fsound_stream_decode.c"),
            Object(NonMatching, "FMOD/src/mixer_clipcopy.c"),
            Object(NonMatching, "FMOD/src/music_formatfsb.c"),
            Object(NonMatching, "FMOD/src/output_gc.c"),
            Object(NonMatching, "FMOD/src/output_nosound.c"),
            Object(NonMatching, "FMOD/src/sound_software.c"),
            Object(NonMatching, "FMOD/src/sound_software_3d.c"),
            Object(NonMatching, "FMOD/src/system_file.c"),
            Object(NonMatching, "FMOD/src/system_memory.c"),
            Object(NonMatching, "FMOD/src/system_misc_gc.c"),
            Object(NonMatching, "FMOD/src/system_thread.c"),
            Object(NonMatching, "FMOD/src/system_time.c"),
            Object(NonMatching, "FMOD/src/system_string.c"),
        ],
    },
 {
        "lib": "binkngc",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_runtime,
        "progress_category": "bink",
        "objects": [
            Object(NonMatching, "bink/src/sdk/decode/ngc/binkngc.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcsnd.c"),
            Object(NonMatching, "bink/src/sdk/decode/binkread.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcfile.c"),
            Object(NonMatching, "bink/src/sdk/decode/yuv.cpp"),
            Object(NonMatching, "bink/src/sdk/decode/binkacd.c"),
            Object(NonMatching, "bink/shared/time/radcb.c"),
            Object(NonMatching, "bink/src/sdk/decode/expand.c"),
            Object(NonMatching, "bink/src/sdk/decode/radmem.c"),
            Object(NonMatching, "bink/src/sdk/decode/yuv.c"),
            Object(NonMatching, "bink/src/sdk/popmal.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcrgb.c"),
            Object(NonMatching, "bink/src/sdk/decode/ngc/ngcyuy2.c"),
            Object(NonMatching, "bink/src/sdk/varbits.c"),
            Object(NonMatching, "bink/src/sdk/fft.c"),
            Object(NonMatching, "bink/src/sdk/dct.c"),
            Object(NonMatching, "bink/src/sdk/bitplane.c"),
        ],
    },
    DolphinLib(
        "ai",
        [
            Object(Matching, "dolphin/src/ai/ai.c"),
        ],
    ),
    DolphinLib(
        "amcstubs",
        [
            Object(Matching, "dolphin/src/amcstubs/AmcExi2Stubs.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(Matching, "dolphin/src/ar/ar.c"),
            Object(Matching, "dolphin/src/ar/arq.c")
        ]
    ),
    DolphinLib(
        "ax",
        [
            Object(Matching, "dolphin/src/ax/AX.c"),
            Object(NonMatching, "dolphin/src/ax/AXAlloc.c"),
            Object(Matching, "dolphin/src/ax/AXAux.c"),
            Object(NonMatching, "dolphin/src/ax/AXCL.c"),
            Object(NonMatching, "dolphin/src/ax/AXOut.c"),
            Object(NonMatching, "dolphin/src/ax/AXSPB.c"),
            Object(NonMatching, "dolphin/src/ax/AXVPB.c"),
            Object(Matching, "dolphin/src/ax/AXComp.c"),
            Object(NonMatching, "dolphin/src/ax/DSPCode.c"),
            Object(Matching, "dolphin/src/ax/AXProf.c"),
        ],
    ),
    DolphinLib(
        "ax",
        [
            Object(NonMatching, "dolphin/src/axfx/reverb_hi.c"),
            Object(NonMatching, "dolphin/src/axfx/reverb_std.c"),
            Object(NonMatching, "dolphin/src/axfx/chorus.c"),
            Object(Matching, "dolphin/src/axfx/delay.c"),
            Object(Matching, "dolphin/src/axfx/axfx.c"),
            Object(NonMatching, "dolphin/src/axfx/reverb_hi_4ch.c"),
        ]
    ),
    DolphinLib(
        "base",
        [
            Object(Matching, "dolphin/src/base/PPCArch.c")
        ]
    ),
    DolphinLib(
        "card",
        [
            Object(Matching, "dolphin/src/card/CARDBios.c"),
            Object(NonMatching, "dolphin/src/card/CARDUnlock.c"),
            Object(Matching, "dolphin/src/card/CARDRdwr.c"),
            Object(Matching, "dolphin/src/card/CARDBlock.c"),
            Object(Matching, "dolphin/src/card/CARDDir.c"),
            Object(Matching, "dolphin/src/card/CARDCheck.c"),
            Object(Matching, "dolphin/src/card/CARDMount.c"),
            Object(Matching, "dolphin/src/card/CARDFormat.c"),
            Object(NonMatching, "dolphin/src/card/CARDOpen.c"),
            Object(Matching, "dolphin/src/card/CARDCreate.c"),
            Object(Matching, "dolphin/src/card/CARDRead.c"),
            Object(Matching, "dolphin/src/card/CARDWrite.c"),
            Object(Matching, "dolphin/src/card/CARDDelete.c"),
            Object(Matching, "dolphin/src/card/CARDStat.c"),
            Object(Matching,"dolphin/src/card/CARDStatEx.c"),
            Object(NonMatching, "dolphin/src/card/CARDNet.c"),
        ]
    ),
    DolphinLib(
        "db",
        [
            Object(Matching, "dolphin/src/db/db.c"),
        ]
    ),
    DolphinLib(
        "dsp",
        [
            Object(NonMatching, "dolphin/src/dsp/dsp.c"),
            Object(Matching, "dolphin/src/dsp/dsp_debug.c"),
            Object(NonMatching, "dolphin/src/dsp/dsp_task.c")
        ]
    ),
    DolphinLib(
        "dvd",
        [
            Object(Matching, "dolphin/src/dvd/dvdlow.c"),
            Object(Matching, "dolphin/src/dvd/dvdfs.c"),
            Object(Matching, "dolphin/src/dvd/dvd.c"),
            Object(Matching, "dolphin/src/dvd/dvdqueue.c"),
            Object(Matching, "dolphin/src/dvd/dvderror.c"),
            Object(Matching, "dolphin/src/dvd/dvdidutils.c"),
            Object(Matching, "dolphin/src/dvd/dvdFatal.c"),
            Object(Matching, "dolphin/src/dvd/emu_level2/fstload.c"),
        ],
    ),
    DolphinLib(
        "exi",
        [
            Object(Matching, "dolphin/src/exi/EXIBios.c"),
            Object(Matching, "dolphin/src/exi/EXIUart.c")
        ]
    ),
    DolphinLib(
        "gx",
        [
            Object(NonMatching, "dolphin/src/gx/GXInit.c"),
            Object(NonMatching, "dolphin/src/gx/GXFifo.c"), # Code Match
            Object(NonMatching, "dolphin/src/gx/GXAttr.c"),
            Object(Matching, "dolphin/src/gx/GXMisc.c"),
            Object(NonMatching, "dolphin/src/gx/GXGeometry.c"),
            Object(Matching, "dolphin/src/gx/GXFrameBuf.c"),
            Object(NonMatching, "dolphin/src/gx/GXLight.c"),
            Object(Matching, "dolphin/src/gx/GXTexture.c"),
            Object(NonMatching, "dolphin/src/gx/GXBump.c"),
            Object(NonMatching, "dolphin/src/gx/GXTev.c"),
            Object(NonMatching, "dolphin/src/gx/GXPixel.c"),
            Object(NonMatching, "dolphin/src/gx/GXDraw.c"),
            Object(Matching, "dolphin/src/gx/GXDisplayList.c"),
            Object(NonMatching, "dolphin/src/gx/GXTransform.c"),
            Object(Matching, "dolphin/src/gx/GXPerf.c")
        ]
    ),
    DolphinLib(
        "mix",
        [
            Object(NonMatching, "dolphin/src/mix/mix.c")
        ]
    ),
    DolphinLib(
        "mtx",
        [
            Object(NonMatching, "dolphin/src/mtx/mtx.c"),
            Object(NonMatching, "dolphin/src/mtx/mtx44.c"),
        ]
    ),
    DolphinLib(
        "OdemuExi2",
        [
            Object(NonMatching, "dolphin/src/OdemuExi2/DebuggerDriver.c", extra_cflags=["-inline on, deferred"])
        ]
    ),
    DolphinLib(
        "odenotstub",
        [
            Object(Matching, "dolphin/src/odenotstub/odenotstub.c")
        ]
    ),
    DolphinLib(
        "os",
        [
            Object(NonMatching, "dolphin/src/os/OS.c"),
            Object(Matching, "dolphin/src/os/OSAlarm.c"),
            Object(Matching, "dolphin/src/os/OSAlloc.c"),
            Object(Matching, "dolphin/src/os/OSArena.c"),
            Object(Matching, "dolphin/src/os/OSAudioSystem.c"),
            Object(Matching, "dolphin/src/os/OSCache.c"),
            Object(Matching, "dolphin/src/os/OSContext.c"),
            Object(NonMatching, "dolphin/src/os/OSError.c"),
            Object(NonMatching, "dolphin/src/os/OSExec.c"),
            Object(NonMatching, "dolphin/src/os/OSFont.c"),
            Object(Matching, "dolphin/src/os/OSInterrupt.c"),
            Object(Matching, "dolphin/src/os/OSLink.c"),
            Object(Matching, "dolphin/src/os/OSMemory.c"),
            Object(Matching, "dolphin/src/os/OSMutex.c"),
            Object(NonMatching, "dolphin/src/os/OSReboot.c"),
            Object(NonMatching, "dolphin/src/os/OSReset.c"),
            Object(Matching, "dolphin/src/os/OSResetSW.c"),
            Object(Matching, "dolphin/src/os/OSRtc.c"),
            Object(NonMatching, "dolphin/src/os/OSThread.c"),
            Object(Matching, "dolphin/src/os/OSTime.c"),
            Object(Matching, "dolphin/src/os/OSSemaphore.c"),
            Object(Matching, "dolphin/src/os/OSSync.c"),
            Object(NonMatching, "dolphin/src/os/init/__start.c"),
            Object(NonMatching, "dolphin/src/os/init/__ppc_eabi_init.cpp")
        ]
    ),
    DolphinLib(
        "pad",
        [
            Object(Matching, "dolphin/src/pad/Padclamp.c"),
            Object(Matching, "dolphin/src/pad/Pad.c")
        ]
    ),
    DolphinLib(
        "si",
        [
            Object(NonMatching, "dolphin/src/si/SIBios.c"),
            Object(Matching, "dolphin/src/si/SISamplingRate.c"),
        ]
    ),
    DolphinLib(
        "vi",
        [
            Object(NonMatching, "dolphin/src/vi/vi.c"), # extra_cflags=["-DMATCHING"]
        ],
    ),
    mslLib(
        "Runtime.PPCEABI.H",
        [],
        [
            Object(NonMatching, "Runtime/__mem.c"),
            Object(Matching, "Runtime/__va_arg.c"),
            Object(NonMatching, "Runtime/global_destructor_chain.c"),
            Object(NonMatching, "Runtime/New.cp"),
            Object(NonMatching, "Runtime/NMWException.cp"),
            Object(Matching, "Runtime/CPlusLibPPC.cp"),
            Object(NonMatching, "Runtime/ptmf.c"),
            Object(NonMatching, "Runtime/runtime.c"),
            Object(NonMatching, "Runtime/__init_cpp_exceptions.cpp"),
            Object(NonMatching, "Runtime/Gecko_ExceptionPPC.cp"),
            Object(NonMatching, "Runtime/GCN_mem_alloc.c"),
        ]
    ),
    mslLib(
        "MSL_C.PPCEABI.H",
        ["-str pool", "-opt level=0, peephole, schedule, nospace", "-inline off", "-sym on"],
        [
            Object(NonMatching, "MSL_C/PPC_EABI/abort_exit.c"),
            Object(NonMatching, "MSL_C/MSL_Common/alloc.c"),
            Object(NonMatching, "MSL_C/MSL_Common/ansi_files.c"),
            Object(NonMatching, "MSL_C/MSL_Common_Embedded/ansi_fp.c"),
            Object(NonMatching, "MSL_C/MSL_Common/arith.c"),
            Object(NonMatching, "MSL_C/MSL_Common/bsearch.c"),
            Object(NonMatching, "MSL_C/MSL_Common/buffer_io.c"),
            Object(Matching, "MSL_C/PPC_EABI/critical_regions.gamecube.c"),
            Object(NonMatching, "MSL_C/MSL_Common/ctype.c"),
            Object(NonMatching, "MSL_C/MSL_Common/direct_io.c"),
            Object(Matching, "MSL_C/MSL_Common/errno.c"),
            Object(NonMatching, "MSL_C/MSL_Common/extras.c"),
            Object(NonMatching, "MSL_C/MSL_Common/file_io.c"),
            Object(NonMatching, "MSL_C/MSL_Common/FILE_POS.C"),
            Object(NonMatching, "MSL_C/MSL_Common/ios.cpp"),
            Object(Matching, "MSL_C/MSL_Common/locale.c"),
            Object(NonMatching, "MSL_C/MSL_Common/mbstring.c"),
            Object(NonMatching, "MSL_C/MSL_Common/mem.c"),
            Object(NonMatching, "MSL_C/MSL_Common/mem_funcs.c"),
            Object(Matching, "MSL_C/MSL_Common/misc_io.c"),
            Object(NonMatching, "MSL_C/MSL_Common/printf.c"),
            Object(NonMatching, "MSL_C/MSL_Common/qsort.c"),
            Object(NonMatching, "MSL_C/MSL_Common/rand.c"),
            Object(NonMatching, "MSL_C/MSL_Common/scanf.c"),
            Object(NonMatching, "MSL_C/MSL_Common/signal.c"),
            Object(NonMatching, "MSL_C/MSL_Common/string.c"),
            Object(NonMatching, "MSL_C/MSL_Common/strtold.c"),
            Object(NonMatching, "MSL_C/MSL_Common/strtoul.c"),
            Object(Matching, "MSL_C/MSL_Common/float.c"),
            Object(NonMatching, "MSL_C/MSL_Common/char_io.c"),
            Object(NonMatching, "MSL_C/MSL_Common/wchar_io.c"),
            Object(NonMatching, "MSL_C/MSL_Common_Embedded/uart_console_io_gcn.c")
        ]
    ),
    mslLib(
        "fdlibm.PPCEABI.H",
        [],
        [
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_acos.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_asin.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_atan2.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_exp.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log.c"),
            Object(NonMatching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c"),
            Object(NonMatching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/e_sqrt.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/k_cos.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/k_sin.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/k_tan.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_atan.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ceil.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_copysign.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_cos.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_floor.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_frexp.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_modf.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_sin.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/s_tan.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_acos.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_asin.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_atan2.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_exp.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_fmod.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_log.c"),
            Object(Matching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_pow.c"),
            Object(NonMatching, "MSL_C/MSL_Common_Embedded/Math/Double_precision/w_sqrt.c"),
            Object(Matching, "MSL_C/PPC_EABI/math_ppc.c"),
        ]
    ),
   trkLib(
        "TRK_MINNOW_DOLPHIN",
        [
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/mainloop.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/nubevent.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/nubassrt.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/nubinit.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/msg.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/msgbuf.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/serpoll.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/dispatch.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/msghndlr.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/support.c"),
            Object(Matching, "debugger/embedded/MetroTRK/Portable/mutex_TRK.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/notify.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/main_TRK.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/mem_TRK.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Portable/string_TRK.c"),
            Object(Matching, "debugger/embedded/MetroTRK/Processor/ppc/Generic/flush_cache.c"),
            # Object(NonMatching, "debugger/embedded/MetroTRK/Processor/ppc/Generic/__exception.s"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c"),
            # Object(NonMatching, "debugger/embedded/MetroTRK/Processor/ppc/Export/targsupp.s"),
            Object(Matching, "debugger/embedded/MetroTRK/Processor/ppc/Generic/mpc_7xx_603e.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Os/dolphin/usr_put.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk_glue.c"),
            Object(Matching, "debugger/embedded/MetroTRK/Os/dolphin/targcont.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Os/dolphin/target_options.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Os/dolphin/UDP_Stubs.c"),
            Object(NonMatching, "debugger/embedded/MetroTRK/Export/mslsupp.c"),

            Object(NonMatching, "gamedev/cust_connection/cc/exi2/GCN/EXI2_DDH_GCN/main.c"),
            Object(NonMatching, "gamedev/cust_connection/utils/common/CircleBuffer.c"),
            Object(NonMatching, "gamedev/cust_connection/cc/exi2/GCN/EXI2_GDEV_GCN/main.c"),
            Object(NonMatching, "gamedev/cust_connection/utils/common/MWTrace.c"),
            Object(NonMatching, "gamedev/cust_connection/utils/gc/MWCriticalSection_gc.cpp"),
        ]
    ),
    {
        "lib": "OdemuExi2",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_runtime,
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "OdemuExi2/DebuggerDriver.c"),
        ],
    },
    RenderWareLib(
        "rpcollis",
        [
            Object(NonMatching, "rwsdk/plugin/rpcollis/ctdata.c"),
            Object(NonMatching, "rwsdk/plugin/rpcollis/colldata.c"),
            Object(NonMatching, "rwsdk/plugin/rpcollis/collquery.c"),
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
            Object(NonMatching, "rwsdk/plugin/matfx/gcn/nbtGen.c"),
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
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skingcnm.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skinstream.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/instance/instanceskin.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skinmatrixblend.c"),
            Object(NonMatching, "rwsdk/plugin/skin2/gcn/skinmatfxgcn.c"),
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
        "fsyst",
        [
            Object(NonMatching, "rwsdk/tool/fsyst/rtfsmgr.c"),
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
        "sknsplit",
        [
            Object(NonMatching, "rwsdk/tool/sknsplit/skinsplit.c"),
            Object(NonMatching, "rwsdk/tool/sknsplit/rtskinsp.c"),
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
            Object(NonMatching, "rwsdk/src/plcore/rwgrp.c"),
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
    ProgressCategory("msl", "MSL"),
    ProgressCategory("RW", "Renderware SDK"),
    ProgressCategory("bink", "Bink SDK"),
    ProgressCategory("FMOD", "FMOD SDK"),
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
