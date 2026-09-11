#!/usr/bin/env python3
"""Write Casks/openms4-nuxl.rb from the assets of a published NuXL release.

Checksums come from the release itself, so the cask can only be generated once
the tested payloads exist. Run it after the release workflow succeeds, then
commit the result.
"""

import argparse
import json
from pathlib import Path
import subprocess

REPOSITORY = "okohlbacher/OpenMS4-nuxl"
ARCHES = {"arm": "arm64", "intel": "x64"}

TEMPLATE = '''cask "openms4-nuxl" do
  arch arm: "arm64", intel: "x64"

  version "{version},{build}"
  sha256 arm:   "{arm}",
         intel: "{intel}"

  url "https://github.com/okohlbacher/OpenMS4-nuxl/releases/download/" \\
      "nuxl-v#{{version.csv.first}}/OpenMS4-nuxl-macos-#{{arch}}-Homebrew-#{{version.csv.second}}.tar.gz"
  name "OpenNuXL"
  desc "Search engine for protein-nucleic acid cross-links, built against the OpenMS Core SDK"
  homepage "https://github.com/okohlbacher/OpenMS4-nuxl"

  depends_on formula: "okohlbacher/openms4-core/openms4-core"
  depends_on macos: :sequoia

  payload = "OpenMS4-nuxl-macos-#{{arch}}-Homebrew-#{{version.csv.second}}"
  binary "#{{payload}}/bin/OpenNuXL"

  postflight_steps do
    run "/usr/bin/xattr",
        args:           ["-dr", "com.apple.quarantine", "."],
        chdir:          ".",
        writable_paths: ["."]
  end
end
'''


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tag", required=True, help="published release tag, e.g. nuxl-v1.0.0-ci.1")
    parser.add_argument("--output", type=Path,
                        default=Path(__file__).resolve().parents[2] / "Casks/openms4-nuxl.rb")
    args = parser.parse_args()
    if not args.tag.startswith("nuxl-v"):
        parser.error("--tag must start with nuxl-v")
    version = args.tag.removeprefix("nuxl-v")
    assets = json.loads(subprocess.check_output(
        ["gh", "release", "view", args.tag, "--repo", REPOSITORY, "--json", "assets"], text=True))["assets"]
    digests, builds = {}, set()
    for key, arch in ARCHES.items():
        prefix = f"OpenMS4-nuxl-macos-{arch}-Homebrew-"
        matches = [a for a in assets if a["name"].startswith(prefix) and a["name"].endswith(".tar.gz")]
        if len(matches) != 1:
            raise SystemExit(f"{args.tag}: expected one {arch} Homebrew payload, found {len(matches)}")
        asset = matches[0]
        digest = asset.get("digest", "")
        if not digest.startswith("sha256:"):
            raise SystemExit(f"{asset['name']}: release does not report a sha256 digest")
        digests[key] = digest.removeprefix("sha256:")
        builds.add(asset["name"].removeprefix(prefix).removesuffix(".tar.gz"))
    if len(builds) != 1:
        raise SystemExit(f"payloads disagree about the source revision: {sorted(builds)}")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        TEMPLATE.format(version=version, build=builds.pop(), **digests), encoding="utf-8")
    print(f"wrote {args.output} for {args.tag}")


if __name__ == "__main__":
    main()
