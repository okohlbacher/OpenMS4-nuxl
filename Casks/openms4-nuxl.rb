cask "openms4-nuxl" do
  arch arm: "arm64", intel: "x64"

  version "1.0.0-ci.7,3331cea8243c"
  sha256 arm:   "000af705e0b8ff4df77e4fa4f649c8bd87c8f20935e8e9d644f4eb12aa8ef83c",
         intel: "6ec2fbe3786d7aa72ae65995384cf7b74cb0182fae84e64acdc0eecccd5e23c7"

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
    next if core == "83ce20da78337b0b329f5c634e52226585e4788d"

    raise Cask::CaskError, "openms4-nuxl #{version.csv.first} was built against openms4-core 83ce20da7833, " \
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
