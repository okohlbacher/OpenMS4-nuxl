cask "openms4-nuxl" do
  arch arm: "arm64", intel: "x64"

  version "1.0.0-ci.1,a2a0f7d01719"
  sha256 arm:   "303ea1f6f7f14c43234f8c92f9f323d9f2d2e5725742285027e4e912716c153e",
         intel: "dcca41a28b20afe2968a13c1f6ba6c873f9d13e894b306a397dc3991beb108de"

  url "https://github.com/okohlbacher/OpenMS4-nuxl/releases/download/" \
      "nuxl-v#{version.csv.first}/OpenMS4-nuxl-macos-#{arch}-Homebrew-#{version.csv.second}.tar.gz"
  name "OpenNuXL"
  desc "Search engine for protein-nucleic acid cross-links, built against the OpenMS Core SDK"
  homepage "https://github.com/okohlbacher/OpenMS4-nuxl"

  depends_on formula: "okohlbacher/openms4-core/openms4-core"
  depends_on macos: :sequoia

  payload = "OpenMS4-nuxl-macos-#{arch}-Homebrew-#{version.csv.second}"
  binary "#{payload}/bin/OpenNuXL"

  postflight_steps do
    run "/usr/bin/xattr",
        args:           ["-dr", "com.apple.quarantine", "."],
        chdir:          ".",
        writable_paths: ["."]
  end
end
