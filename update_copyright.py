#!/usr/bin/env python3
#

from pathlib import Path

before = "Copyright (C) 2012-2019 The VPaint Developers"

after = "Copyright (C) 2012-2019 The VPaint Developers"

if __name__ == "__main__":

    path = Path(".")
    for file in path.glob("src/**/*"):
        if file.suffix in [".h", ".cpp", ".pro"]:
            s1 = file.read_text()
            s2 = s1.replace(before, after)
            if s1 != s2:
                file.write_text(s2)
                print(file)
