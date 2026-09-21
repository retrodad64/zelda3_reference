#!/usr/bin/env python3
"""Drive zelda3 from the command line and collect what it dumps.

The game can already start where you want it (--skip-intro, --load-save,
--screen-id) and it can dump tiles and animations on a key press. This ties those
together: it writes a key script, runs the game until the script is done, and
gathers the PNGs and the log into one directory so a run is a thing you can look
at afterwards rather than something you watched go by.

A run with a script ends on its own, because the frame budget is worked out from
it. Nothing there needs a person at the keyboard, and nothing needs the operating
system's permission to send keystrokes, which is the reason the game takes a
script at all.

A run with no script is you opening the game to play it. That one has no frame
budget and no timeout: it ends when you close the window, and anything it dumped
on the way is collected afterwards.

    zdrive.py --save "saves/ref/Chapter 2 - After Eastern Palace.sav" --goto 785,1733

Examples:

    zdrive.py --skip-intro --do "wait 60; tap logtiles" --out runs/house

    zdrive.py --save "saves/ref/Chapter 2 - After Eastern Palace.sav" \\
              --screen 2C --do "hold right 40; hold loganimation 45" --out runs/bush

    zdrive.py --skip-intro --script my.keys --out runs/scripted

    zdrive.py --save ... --goto 2806,3183 --do "tap logtiles" --out runs/bush

The music is off and Link can't be hurt unless you ask otherwise, because an unattended run
has nobody to turn the volume down or heal him. Pass --music or --mortal to get either back.

Actions, separated by semicolons or newlines:

    wait N          nothing happens for N frames
    tap NAME        press and release
    hold NAME N     press, wait N frames, release
    press NAME      press and take no time
    release NAME    release and take no time

Names are the joypad (up down left right select start a b x y l r) and the
commands (logtiles, loganimation, gotoscreen, cheatlife, cheatequipment,
walkthroughwalls, pause, turbo, load1 to load10, save1 to save10).
"""

import argparse
import re
import os
import os
import shlex
import shutil
import subprocess
import sys
import time
from pathlib import Path

# The game writes these next to zelda3.ini, numbered from 0001 upward.
DUMP_PATTERNS = ("zelda3_tiles_*.png", "zelda3_anim_*.png", "zelda3_frame_*.png")

# Frames to keep running after the script finishes, so a dump triggered by the
# last action still has time to be written.
TAIL_FRAMES = 60


def repo_root() -> Path:
    """The directory holding zelda3.ini, which is where the game writes."""
    here = Path(__file__).resolve().parent.parent

    if not (here / "zelda3.ini").exists():
        sys.exit(f"zdrive: no zelda3.ini under {here}")

    return here


def parse_actions(text: str) -> list[str]:
    """Split an action string into script lines, keeping comments out."""
    lines = []

    for raw in re.split(r"[;\n]", text):
        stripped = raw.split("#", 1)[0].strip()

        if stripped:
            lines.append(stripped)

    return lines


def script_frames(lines: list[str]) -> int:
    """How long the script runs for, counted the way the game counts it."""
    total = 0

    for line in lines:
        parts = line.split()
        verb = parts[0].lower()

        if verb == "wait" and len(parts) >= 2:
            total += int(parts[1])
        elif verb == "tap":
            total += 2
        elif verb == "hold" and len(parts) >= 3:
            total += int(parts[2])
        elif verb in ("press", "release"):
            pass

    return total


def tile_digest(stdout: str) -> list[str]:
    """The tile log with runs of identical states collapsed to one line.

    Standing still logs the same line every frame, so a hundred frames of it says
    nothing a single line doesn't. Collapsing them leaves one line per distinct
    state, which is the list of tiles Link actually stood on.
    """
    lines = []
    last = None

    for raw in stdout.splitlines():
        if "tiles module=" not in raw:
            continue

        # Link's position moves a pixel at a time, so it would defeat the whole point of
        # collapsing. The state is everything else: where he is and what he's standing on.
        fields = [f for f in raw.split()
                  if "=" in f and not f.startswith(("link=", "feet="))]
        state = " ".join(fields)

        if state != last:
            lines.append(raw.strip())
            last = state

    return lines


def existing_dumps(root: Path) -> set[Path]:
    found = set()

    for pattern in DUMP_PATTERNS:
        found.update(root.glob(pattern))

    return found


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Run zelda3 from a key script and collect its dumps.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )

    ap.add_argument("--do", help="actions, separated by semicolons")
    ap.add_argument("--script", help="a file of actions, one per line")
    ap.add_argument("--out", help="directory to collect the run into")
    ap.add_argument("--skip-intro", action="store_true", help="start a new file in the house")
    ap.add_argument("--save", help="savestate to load")
    ap.add_argument("--screen", help="overworld screen to jump to, hex 00 to 7F")
    ap.add_argument("--goto", help="land his feet on an exact world pixel, x,y or d:x,y for dark")
    ap.add_argument("--enemy-warp", help="warp to an enemy by sprite type, hex")
    ap.add_argument("--npc-warp", help="warp to an npc by sprite type, hex")
    ap.add_argument("--boss-warp", help="warp to a boss by sprite type, hex")
    ap.add_argument("--entrance", help="go through a door by entrance number, hex")
    ap.add_argument("--lift-warp", nargs="?", const="any",
                    help="stand beside something pickable, optionally a map16 value in hex")
    ap.add_argument("--list-liftables", action="store_true",
                    help="print every liftable cell on the loaded map, then carry on")
    ap.add_argument("--music", action="store_true", help="leave the music on")
    ap.add_argument("--mortal", action="store_true", help="leave the damage cheat off")
    ap.add_argument("--frames", type=int, help="frame budget, otherwise taken from the script")
    ap.add_argument("--timeout", type=int, default=120,
                    help="seconds before giving up on a scripted run; a run without a script waits")
    ap.add_argument("--keep", action="store_true", help="leave the dumps where the game wrote them")
    args = ap.parse_args()

    if args.do and args.script:
        sys.exit("zdrive: pass --do or --script, not both")

    root = repo_root()
    game = root / "zelda3"

    if not game.exists():
        sys.exit(f"zdrive: {game} is not built")

    if args.script:
        lines = parse_actions(Path(args.script).read_text())
    elif args.do:
        lines = parse_actions(args.do)
    else:
        lines = []

    # A run with a script ends on its own, because the budget is worked out from the script.
    # A run without one is somebody opening the game to play it, so it has no budget at all and
    # ends when they close it.
    if args.frames:
        frames = args.frames
    elif lines:
        frames = script_frames(lines) + TAIL_FRAMES
    else:
        frames = 0

    cmd = [str(game)]

    if args.skip_intro:
        cmd.append("--skip-intro")

    if args.save:
        cmd += ["--load-save", args.save]

    if args.screen:
        cmd += ["--screen-id", args.screen]

    if args.goto:
        cmd += ["--goto", args.goto]

    for flag in ("enemy_warp", "npc_warp", "boss_warp"):
        value = getattr(args, flag)

        if value:
            cmd += ["--" + flag.replace("_", "-"), value]

    # A scripted run has nobody listening and nobody to heal Link, so it starts quiet and
    # unkillable. Combat work wants the real thing, which is what the two opt outs are for.
    if args.entrance:
        cmd += ["--entrance", args.entrance]

    if args.lift_warp:
        cmd.append("--lift-warp")

        if args.lift_warp != "any":
            cmd.append(args.lift_warp)

    if args.list_liftables:
        cmd.append("--list-liftables")

    if not args.music:
        cmd.append("--no-music")

    if not args.mortal:
        cmd.append("--invincible")

    script_path = None

    if lines:
        # The pid keeps two runs at once from deleting each other's script.
        script_path = root / f".zdrive.{os.getpid()}.keys"
        script_path.write_text("\n".join(lines) + "\n")
        cmd += ["--keys", str(script_path)]

    if frames:
        cmd += ["--quit-after", str(frames)]

    # The game numbers its dumps from 0001 every run, so anything still sitting in the root is
    # from a run that didn't collect. Leaving it there would mask this run's dump of the same name.
    for stale in existing_dumps(root):
        stale.unlink()

    before = existing_dumps(root)
    log_before = 0
    log_path = root / "zelda3_debug.log"

    if log_path.exists():
        log_before = log_path.stat().st_size

    # --no-music only quiets the music, and a scripted run still has nobody listening, so the
    # whole audio device goes away unless the music was asked for.
    env = dict(os.environ)

    if not args.music:
        env["SDL_AUDIODRIVER"] = "dummy"

    print("zdrive:", shlex.join(cmd))
    started = time.time()

    try:
        done = subprocess.run(cmd, cwd=root, capture_output=True, text=True,
                              timeout=(args.timeout if frames else None), env=env)
        stdout, stderr, code = done.stdout, done.stderr, done.returncode
    except subprocess.TimeoutExpired as expired:
        stdout = expired.stdout.decode() if expired.stdout else ""
        stderr = expired.stderr.decode() if expired.stderr else ""
        code = -1
        print(f"zdrive: gave up after {args.timeout}s", file=sys.stderr)

    elapsed = time.time() - started
    fresh = sorted(existing_dumps(root) - before)

    budget = f"{frames} frames" if frames else "no frame limit"

    print(f"zdrive: {budget} in {elapsed:.1f}s, exit {code}, {len(fresh)} dump(s)")

    for path in fresh:
        print("   ", path.name)

    digest = tile_digest(stdout)

    if digest:
        print(f"zdrive: {len(digest)} distinct tile state(s)")

        for line in digest[:12]:
            print("   ", line)

        if len(digest) > 12:
            print(f"    ... {len(digest) - 12} more, see tiles.txt")

    if stderr.strip():
        print("zdrive: stderr:", stderr.strip(), file=sys.stderr)

    if args.out:
        out = Path(args.out)
        out.mkdir(parents=True, exist_ok=True)

        (out / "stdout.txt").write_text(stdout)

        if digest:
            (out / "tiles.txt").write_text("\n".join(digest) + "\n")

        if stderr:
            (out / "stderr.txt").write_text(stderr)

        if script_path and script_path.exists():
            shutil.copy(script_path, out / "run.keys")

        # Only the part of the log this run appended, so the file is about this run.
        if log_path.exists():
            with log_path.open() as handle:
                handle.seek(log_before)
                (out / "debug.log").write_text(handle.read())

        for path in fresh:
            if args.keep:
                shutil.copy(path, out / path.name)
            else:
                shutil.move(str(path), out / path.name)

        print("zdrive: collected into", out)

    if script_path and script_path.exists():
        script_path.unlink()

    return 0 if code == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
