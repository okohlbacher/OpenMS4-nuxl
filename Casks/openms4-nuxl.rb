cask "openms4-nuxl" do
  arch arm: "arm64", intel: "x64"

  version "1.0.0-ci.5,77c129b72746"
  sha256 arm:   "70a47371c102cc4c6b172541255249b02a46254383ad172e42c1b9224ef44c9c",
         intel: "6b705633f4906edd03ccb94877bbf3d6c764025a4903051f970877ba0bf89ce5"

  url "https://github.com/okohlbacher/OpenMS4-nuxl/releases/download/" \
      "nuxl-v#{version.csv.first}/OpenMS4-nuxl-macos-#{arch}-Homebrew-#{version.csv.second}.tar.gz"
  name "OpenNuXL"
  desc "Search engine for protein-nucleic acid cross-links, built against the OpenMS Core SDK"
  homepage "https://github.com/okohlbacher/OpenMS4-nuxl"

  depends_on formula: "okohlbacher/openms4-core/openms4-core"
  depends_on macos: :sequoia

  payload = "OpenMS4-nuxl-macos-#{arch}-Homebrew-#{version.csv.second}"
  binary "#{payload}/bin/OpenNuXL"

  # libOpenMS has no versioned name, so a payload only runs with the Core it was built against.
  preflight do
    config = "#{HOMEBREW_PREFIX}/opt/openms4-core/lib/cmake/OpenMS/OpenMSConfig.cmake"
    core = File.exist?(config) ? File.read(config)[/set\(OpenMS_SOURCE_REVISION "([0-9a-f]{40})"\)/, 1] : nil
    next if core == "eb58e981d7e0864634b59230874a56a1512369f7"

    raise Cask::CaskError, "openms4-nuxl #{version.csv.first} was built against openms4-core eb58e981d7e0, " \
                           "but the installed openms4-core is #{core&.slice(0, 12) || "unknown"}. " \
                           "Install the openms4-nuxl release built for the installed Core."
  end

  postflight_steps do
    run "/usr/bin/xattr",
        args:           ["-dr", "com.apple.quarantine", "."],
        chdir:          ".",
        writable_paths: ["."]
  end
end
